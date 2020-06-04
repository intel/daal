/*******************************************************************************
* Copyright 2014-2019 Intel Corporation
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

#pragma once

#include "oneapi/dal/train.hpp"
#include "oneapi/dal/algo/pca/train_types.hpp"
#include "oneapi/dal/algo/pca/detail/train_ops.hpp"

namespace dal {
namespace detail {

template <typename Descriptor>
struct train_ops<Descriptor, decomposition::pca::detail::tag>
  : decomposition::pca::detail::train_ops<Descriptor> {};

} // namespace detail
} // namespace dal
