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

#pragma once

#include "oneapi/dal/algo/knn/infer_types.hpp"

namespace oneapi::dal::knn::detail {

template <typename Context, typename... Options>
struct ONEAPI_DAL_EXPORT infer_ops_dispatcher {
    infer_result operator()(const Context&, const descriptor_base&, const infer_input&) const;
};

template <typename Descriptor>
struct infer_ops {
    using float_t           = typename Descriptor::float_t;
    using method_t          = typename Descriptor::method_t;
    using input_t           = infer_input;
    using result_t          = infer_result;
    using descriptor_base_t = descriptor_base;

    void check_preconditions(const Descriptor& params, const infer_input& input) const {
        if (!(input.get_data().has_data())) {
            throw domain_error("Input data should not be empty");
        }
    }

    void check_postconditions(const Descriptor& params,
                              const infer_input& input,
                              const infer_result& result) const {
        if (result.get_labels().get_column_count() != 1) {
            throw internal_error("Result labels column_count should contain a single column");
        }
        if (result.get_labels().get_row_count() != input.get_data().get_row_count()) {
            throw internal_error("Number of labels in result should match number of rows in input");
        }
        row_accessor<const float_t> acc{ result.get_labels(); }
        for(std::int64_t index; index < result.get_labels().get_row_count(); index++) {
            auto label = acc.pull(index);
            if(label[0] < 0 || label[0] >= params.get_class_count())
                throw internal_error("Result label value is invalid");
        }
    }

    template <typename Context>
    auto operator()(const Context& ctx, const Descriptor& desc, const infer_input& input) const {
        check_preconditions(desc, input);
        const auto result = infer_ops_dispatcher<Context, float_t, method_t>()(ctx, desc, input);
        check_postconditions(desc, input, result);
        return result;  
    }
};

} // namespace oneapi::dal::knn::detail
