/*******************************************************************************
* Copyright 2020-2021 Intel Corporation
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

#include "oneapi/dal/algo/subgraph_isomorphism/backend/cpu/matching.hpp"

namespace oneapi::dal::preview::subgraph_isomorphism::backend {

namespace dal = oneapi::dal;

template class matching_engine<__CPU_TAG__>;
template class engine_bundle<__CPU_TAG__>;
} // namespace oneapi::dal::preview::subgraph_isomorphism::backend
