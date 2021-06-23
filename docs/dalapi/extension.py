# file: extension.py
#===============================================================================
# Copyright 2019-2021 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

import os
import time
from typing import (Dict, Tuple, Text)
import sphinx.util.logging
from . import doxypy
from . import utils
from . import roles
from . import directives
from . import transformers

class PathResolver(object):
    def __init__(self, app,
                 relative_doxyfile_dir,
                 relative_sources_dir):
        self.base_dir = app.confdir
        self.doxyfile_dir = self.absjoin(self.base_dir, relative_doxyfile_dir)
        self.sources_dir = self.absjoin(self.base_dir, relative_sources_dir)
        self.doxygen_xml = self.absjoin(self.doxyfile_dir, 'doxygen', 'xml')

    def __call__(self, relative_name):
        return self.absjoin(self.base_dir, relative_name)

    def absjoin(self, *args):
        return os.path.abspath(os.path.join(*args))


class ProjectWatcher(object):
    def __init__(self, ctx, path_resolver):
        self.ctx = ctx
        self._path_resolver = path_resolver
        self._xml_timer = utils.FileModificationTimer(
            path_resolver.doxygen_xml, '*.xml')
        self._hpp_timer = utils.FileModificationTimer(
            path_resolver.sources_dir, '*.hpp')
        self._doxygen = utils.ProcessHandle(
            'doxygen', path_resolver.doxyfile_dir)

    def link_docname(self, docname):
        full_path = self._path_resolver(f'{docname}.rst')
        self._linked_docnames[docname] = (full_path, time.time())

    def get_outdated_docnames(self, modified_docnames):
        # We do not need to check the modified documents,
        # they should be updated by Sphinx in any way
        for docname in modified_docnames:
            if docname in self._linked_docnames:
                del self._linked_docnames[docname]

        xml_mtime = self._xml_timer()
        hpp_mtime = self._hpp_timer()
        if xml_mtime < hpp_mtime:
            self.ctx.log('Run Doxygen')
            self._doxygen.run()

        outdated_docnames = []
        for docname, info in self._linked_docnames.items():
            _, link_time = info
            if (self.ctx.always_rebuild or
                link_time < xml_mtime or link_time < hpp_mtime):
                outdated_docnames.append(docname)

        if self.ctx.debug:
            for docname in outdated_docnames:
                self.ctx.log('OUTDATED', docname)

        return outdated_docnames


    def _update_linked_docnames(self):
        relevant_linked_docnames = {}
        for docname, info in self._linked_docnames.items():
            docfilename = info[0]
            if os.path.exists(docfilename):
                relevant_linked_docnames[docname] = info
        self._linked_docnames = relevant_linked_docnames

    @property
    def _linked_docnames(self) -> Dict[Text, Tuple[Text, float]]:
        if not hasattr(self.ctx.app.env, 'dalapi_linked_docnames'):
            self.ctx.app.env.dalapi_linked_docnames = {}
        return self.ctx.app.env.dalapi_linked_docnames

    @_linked_docnames.setter
    def _linked_docnames(self, value):
        self.ctx.app.env.dalapi_linked_docnames = value


class Context(object):
    def __init__(self, app):
        self.app = app
        self._index = None
        self._watcher = None
        self._doxygen = None
        self._listing = None
        self._path_resolver = None
        self._is_listing_enabled = False
        self._is_warning_undocumented_enabled = False
        self._info_ignored_undocumented = False
        self._undocumented_warn_filter = doxypy.UndocumentedWarnFilter([])
        self._read_env()

    def configure(self, relative_doxyfile_dir, relative_sources_dir, is_listing_enabled,
                  is_warning_undocumented_enabled, info_ignored_undocumented, ignored_undocumented_warnings):
        self._path_resolver = PathResolver(
            self.app,
            relative_doxyfile_dir,
            relative_sources_dir
        )
        self._is_listing_enabled = is_listing_enabled
        self._is_warning_undocumented_enabled = is_warning_undocumented_enabled
        self._info_ignored_undocumented = info_ignored_undocumented
        self._undocumented_warn_filter = doxypy.UndocumentedWarnFilter(ignored_undocumented_warnings)

    @property
    def current_docname(self):
        return self.app.env.docname

    @property
    def index(self) -> doxypy.Index:
        if self._index is None:
            self._index = doxypy.index(self.path_resolver.doxygen_xml,
                name_transformer=transformers.NameTransformer(),
                transformer_passes= [
                    transformers.PropertyTransformer(),
                    transformers.RstDescriptionTransformer(),
                ]
            )
            if self._is_warning_undocumented_enabled:
                self._log_undocumented_warns()

        return self._index

    @property
    def watcher(self) -> ProjectWatcher:
        if self._watcher is None:
            self._watcher = ProjectWatcher(self, self.path_resolver)
        return self._watcher

    @property
    def listing(self) -> doxypy.ListingReader:
        if self._listing is None:
            self._listing = doxypy.ListingReader(self.path_resolver.sources_dir)
        return self._listing

    @property
    def listing_enabled(self) -> bool:
        return self._is_listing_enabled

    @property
    def path_resolver(self):
        if not self._path_resolver:
            raise Exception('Context is not configured')
        return self._path_resolver

    def log(self, *args):
        if self.debug:
            print('[dalapi]:', *args)

    def _read_env(self):
        def get_env_flag(env_var):
            value = os.environ.get(env_var, '0')
            return value.lower() in ['1', 'yes', 'y']
        self.debug = get_env_flag('DALAPI_DEBUG')
        self.always_rebuild = get_env_flag('DALAPI_ALWAYS_REBUILD')

    def _log_undocumented_warns(self):
        logger = sphinx.util.logging.getLogger('DoxygenWarns')
        for warn in self._index.warns.get('Undocumented', []):
            if self._undocumented_warn_filter.ignored(warn):
                if self._info_ignored_undocumented:
                    logger.info('[Undocumented]%s', warn)
                continue
            logger.warning(warn)


class EventHandler(object):
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def env_get_outdated(self, app, env, added, changed, removed):
        return self.ctx.watcher.get_outdated_docnames(added | changed | removed)

    def get_config_values(self, app):
        self.ctx.configure(
            relative_doxyfile_dir=app.config.onedal_relative_doxyfile_dir,
            relative_sources_dir=app.config.onedal_relative_sources_dir,
            is_listing_enabled=app.config.onedal_enable_listing,
            is_warning_undocumented_enabled=app.config.onedal_enable_warning_undocumented,
            ignored_undocumented_warnings=app.config.onedal_ignored_undocumented_warnings,
            info_ignored_undocumented=app.config.onedal_info_ignored_undocumented
        )

def setup(app):
    ctx = Context(app)

    app.add_role('capterm', roles.capterm_role)
    app.add_role('txtref', roles.txtref_role)

    app.add_directive('onedal_class', directives.ClassDirective(ctx))
    app.add_directive('onedal_func', directives.FunctionDirective(ctx))
    app.add_directive('onedal_code', directives.ListingDirective(ctx))
    app.add_directive('onedal_tags_namespace', directives.TagsNamespaceDirective(ctx))
    app.add_directive('onedal_enumclass', directives.EnumClassDirective(ctx))

    app.add_config_value('onedal_relative_doxyfile_dir', '.', 'env')
    app.add_config_value('onedal_relative_sources_dir', '.', 'env')
    app.add_config_value('onedal_enable_listing', True, 'env')
    app.add_config_value('onedal_enable_warning_undocumented', False, 'env')
    app.add_config_value('onedal_info_ignored_undocumented', False, 'env')
    app.add_config_value('onedal_ignored_undocumented_warnings', [], 'env')

    handler = EventHandler(ctx)
    app.connect("builder-inited", handler.get_config_values)
    app.connect('env-get-outdated', handler.env_get_outdated)
