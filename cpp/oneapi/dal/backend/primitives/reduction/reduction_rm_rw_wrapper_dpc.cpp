/*******************************************************************************
* Copyright 2021 Intel Corporation
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

#include "oneapi/dal/backend/primitives/reduction/reduction_rm_rw_dpc.hpp"

#include "oneapi/dal/backend/primitives/common.hpp"

namespace oneapi::dal::backend::primitives {

#ifdef ONEDAL_DATA_PARALLEL

template <typename Float, typename BinaryOp, typename UnaryOp>
reduction_rm_rw<Float, BinaryOp, UnaryOp>::reduction_rm_rw(sycl::queue& q) : q_{ q } {}

template <typename Float, typename BinaryOp, typename UnaryOp>
typename reduction_rm_rw<Float, BinaryOp, UnaryOp>::reduction_method
reduction_rm_rw<Float, BinaryOp, UnaryOp>::propose_method(std::int64_t width) const {
    const auto device_max_wg = device_max_wg_size(q_);
    if (width < device_max_wg) {
        return reduction_method::narrow;
    }
    else {
        return reduction_method::wide;
    }
    return reduction_method::wide;
}

template <typename Float, typename BinaryOp, typename UnaryOp>
sycl::event reduction_rm_rw<Float, BinaryOp, UnaryOp>::operator()(
    const typename reduction_rm_rw<Float, BinaryOp, UnaryOp>::reduction_method method,
    const Float* input,
    Float* output,
    const std::int64_t width,
    const std::int64_t height,
    const std::int64_t stride,
    const BinaryOp& binary,
    const UnaryOp& unary,
    const event_vector& deps) const {
    // TODO: think about `switch` operator
    if (method == reduction_method::narrow) {
        const narrow_t kernel{ q_ };
        return kernel(input, output, width, height, stride, binary, unary, deps);
    }
    if (method == reduction_method::wide) {
        const auto device_max_wg = device_max_wg_size(q_);
        const wide_t kernel{ q_, std::min(width, device_max_wg) };
        return kernel(input, output, width, height, stride, binary, unary, deps);
    }
    ONEDAL_ASSERT(false);
    return sycl::event{};
}

template <typename Float, typename BinaryOp, typename UnaryOp>
sycl::event reduction_rm_rw<Float, BinaryOp, UnaryOp>::operator()(const Float* input,
                                                                  Float* output,
                                                                  const std::int64_t width,
                                                                  const std::int64_t height,
                                                                  const std::int64_t stride,
                                                                  const BinaryOp& binary,
                                                                  const UnaryOp& unary,
                                                                  const event_vector& deps) const {
    const auto method = propose_method(width);
    return this->operator()(method, input, output, width, height, stride, binary, unary, deps);
}

template <typename Float, typename BinaryOp, typename UnaryOp>
sycl::event reduction_rm_rw<Float, BinaryOp, UnaryOp>::operator()(
    const typename reduction_rm_rw<Float, BinaryOp, UnaryOp>::reduction_method method,
    const Float* input,
    Float* output,
    const std::int64_t width,
    const std::int64_t height,
    const BinaryOp& binary,
    const UnaryOp& unary,
    const event_vector& deps) const {
    return this->operator()(method, input, output, width, height, width, binary, unary, deps);
}

template <typename Float, typename BinaryOp, typename UnaryOp>
sycl::event reduction_rm_rw<Float, BinaryOp, UnaryOp>::operator()(const Float* input,
                                                                  Float* output,
                                                                  const std::int64_t width,
                                                                  const std::int64_t height,
                                                                  const BinaryOp& binary,
                                                                  const UnaryOp& unary,
                                                                  const event_vector& deps) const {
    const auto method = propose_method(width);
    return this->operator()(method, input, output, width, height, width, binary, unary, deps);
}

#define INSTANTIATE(F, B, U) template class reduction_rm_rw<F, B, U>;

#define INSTANTIATE_FLOAT(B, U)                \
    INSTANTIATE(double, B<double>, U<double>); \
    INSTANTIATE(float, B<float>, U<float>);

INSTANTIATE_FLOAT(min, identity)
INSTANTIATE_FLOAT(min, abs)
INSTANTIATE_FLOAT(min, square)

INSTANTIATE_FLOAT(max, identity)
INSTANTIATE_FLOAT(max, abs)
INSTANTIATE_FLOAT(max, square)

INSTANTIATE_FLOAT(sum, identity)
INSTANTIATE_FLOAT(sum, abs)
INSTANTIATE_FLOAT(sum, square)

#undef INSTANTIATE_FLOAT

#undef INSTANTIATE

#endif

} // namespace oneapi::dal::backend::primitives