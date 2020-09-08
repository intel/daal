/*******************************************************************************
* Copyright 2020 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <cassert>
#include <iostream>

#include "example_util/utils.hpp"
#include "oneapi/dal/graph/graph_service_functions.hpp"
#include "oneapi/dal/graph/undirected_adjacency_array_graph.hpp"
#include "oneapi/dal/io/detail/meas.hpp"
#include "oneapi/dal/io/graph_csv_data_source.hpp"
#include "oneapi/dal/io/load_graph.hpp"

using namespace oneapi::dal;
using namespace oneapi::dal::preview;

int main(int argc, char **argv) {
    CR_INIT()
    assert(argc > 1);
    const std::string filename = get_data_path(argv[1]);
    std::cout << "Graph is read from file: " << filename << std::endl;

    graph_csv_data_source ds(filename);
    CR_ST()
    load_graph::descriptor<> d;
    auto my_graph = load_graph::load(d, ds);
    CR_END("total")

    return 0;
}
