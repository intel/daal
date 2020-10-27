/* file: mkl_blas.h */
/*******************************************************************************
* Copyright 2014-2020 Intel Corporation
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

/*
//++
//  Wrappers for BLAS functions.
//--
*/

#ifndef __ONEAPI_INTERNAL_MKL_BLAS_H__
#define __ONEAPI_INTERNAL_MKL_BLAS_H__

#include "services/internal/buffer.h"
#include "services/internal/sycl/error_handling_sycl.h"
#include "services/internal/sycl/math/mkl_dal.h"

namespace daal
{
namespace services
{
namespace internal
{
namespace sycl
{
namespace math
{
namespace interface1
{
namespace
{
inline auto to_fpk_transpose(const math::Transpose & trans)
{
    return trans == math::Transpose::Trans ? ::oneapi::fpk::transpose::trans : ::oneapi::fpk::transpose::nontrans;
}

inline auto to_fpk_uplo(const math::UpLo & uplo)
{
    return uplo == math::UpLo::Upper ? ::oneapi::fpk::uplo::upper : ::oneapi::fpk::uplo::lower;
}
} // namespace

/** @ingroup oneapi_internal
 * @{
 */

/**
 *  <a name="DAAL-CLASS-ONEAPI-INTERNAL__MKLGEMM"></a>
 *  \brief Adapter for MKL GEMM routine
 */
template <typename algorithmFPType>
struct MKLGemm
{
    MKLGemm(cl::sycl::queue & queue) : _queue(queue) {}

    Status operator()(const math::Transpose transa, const math::Transpose transb, const size_t m, const size_t n, const size_t k,
                      const algorithmFPType alpha, const Buffer<algorithmFPType> & a_buffer, const size_t lda, const size_t offsetA,
                      const Buffer<algorithmFPType> & b_buffer, const size_t ldb, const size_t offsetB, const algorithmFPType beta,
                      Buffer<algorithmFPType> & c_buffer, const size_t ldc, const size_t offsetC)
    {
        Status status;

#ifdef DAAL_SYCL_INTERFACE_USM
        if (a_buffer.isUSMBacked() && b_buffer.isUSMBacked() && c_buffer.isUSMBacked())
        {
            const auto transamkl = to_fpk_transpose(transa);
            const auto transbmkl = to_fpk_transpose(transb);

            auto a_usm = a_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);
            auto b_usm = b_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);
            auto c_usm = c_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);

            auto a_ptr = a_usm.get() + offsetA;
            auto b_ptr = b_usm.get() + offsetB;
            auto c_ptr = c_usm.get() + offsetC;

            status |= catchSyclExceptions([&]() mutable {
                ::oneapi::fpk::blas::gemm(_queue, transamkl, transbmkl, m, n, k, alpha, a_ptr, lda, b_ptr, ldb, beta, c_ptr, ldc);
                _queue.wait_and_throw();
            });
        }
        else
#endif
        {
            const MKL_TRANSPOSE transamkl = transa == math::Transpose::Trans ? MKL_TRANS : MKL_NOTRANS;
            const MKL_TRANSPOSE transbmkl = transb == math::Transpose::Trans ? MKL_TRANS : MKL_NOTRANS;

            cl::sycl::buffer<algorithmFPType, 1> a_sycl_buff = a_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            cl::sycl::buffer<algorithmFPType, 1> b_sycl_buff = b_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            cl::sycl::buffer<algorithmFPType, 1> c_sycl_buff = c_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            status |= catchSyclExceptions([&]() mutable {
                innerGemm(transamkl, transbmkl, m, n, k, alpha, a_sycl_buff, lda, b_sycl_buff, ldb, beta, c_sycl_buff, ldc, offsetA, offsetB,
                          offsetC);
                _queue.wait_and_throw();
            });
        }

        return status;
    }

private:
    template <typename T>
    void innerGemm(MKL_TRANSPOSE transa, MKL_TRANSPOSE transb, int64_t m, int64_t n, int64_t k, T alpha, cl::sycl::buffer<T, 1> a, int64_t lda,
                   cl::sycl::buffer<T, 1> b, int64_t ldb, T beta, cl::sycl::buffer<T, 1> c, int64_t ldc, int64_t offset_a, int64_t offset_b,
                   int64_t offset_c);

    template <>
    void innerGemm<double>(MKL_TRANSPOSE transa, MKL_TRANSPOSE transb, int64_t m, int64_t n, int64_t k, double alpha, cl::sycl::buffer<double, 1> a,
                           int64_t lda, cl::sycl::buffer<double, 1> b, int64_t ldb, double beta, cl::sycl::buffer<double, 1> c, int64_t ldc,
                           int64_t offset_a, int64_t offset_b, int64_t offset_c)
    {
        ::oneapi::fpk::gpu::dgemm_sycl(&_queue, transa, transb, m, n, k, alpha, &a, lda, &b, ldb, beta, &c, ldc, offset_a, offset_b, offset_c);
    }

    template <>
    void innerGemm<float>(MKL_TRANSPOSE transa, MKL_TRANSPOSE transb, int64_t m, int64_t n, int64_t k, float alpha, cl::sycl::buffer<float, 1> a,
                          int64_t lda, cl::sycl::buffer<float, 1> b, int64_t ldb, float beta, cl::sycl::buffer<float, 1> c, int64_t ldc,
                          int64_t offset_a, int64_t offset_b, int64_t offset_c)
    {
        ::oneapi::fpk::gpu::sgemm_sycl(&_queue, transa, transb, m, n, k, alpha, &a, lda, &b, ldb, beta, &c, ldc, offset_a, offset_b, offset_c);
    }

    cl::sycl::queue & _queue;
};

/**
 *  <a name="DAAL-CLASS-ONEAPI-INTERNAL__MKLSYRK"></a>
 *  \brief Adapter for MKL SYRK routine
 */
template <typename algorithmFPType>
struct MKLSyrk
{
    MKLSyrk(cl::sycl::queue & queue) : _queue(queue) {}

    Status operator()(const math::UpLo upper_lower, const math::Transpose trans, const size_t n, const size_t k, const algorithmFPType alpha,
                      const Buffer<algorithmFPType> & a_buffer, const size_t lda, const size_t offsetA, const algorithmFPType beta,
                      Buffer<algorithmFPType> & c_buffer, const size_t ldc, const size_t offsetC)
    {
        Status status;

#ifdef DAAL_SYCL_INTERFACE_USM
        if (a_buffer.isUSMBacked() && c_buffer.isUSMBacked())
        {
            const auto transmkl = to_fpk_transpose(trans);
            const auto uplomkl  = to_fpk_uplo(upper_lower);

            auto a_usm = a_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);
            auto c_usm = c_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);

            auto a_ptr = a_usm.get() + offsetA;
            auto c_ptr = c_usm.get() + offsetC;

            status |= catchSyclExceptions([&]() mutable {
                ::oneapi::fpk::blas::syrk(_queue, uplomkl, transmkl, n, k, alpha, a_ptr, lda, beta, c_ptr, ldc);
                _queue.wait_and_throw();
            });
        }
        else
#endif
        {
            const MKL_TRANSPOSE transmkl = trans == math::Transpose::Trans ? MKL_TRANS : MKL_NOTRANS;
            const MKL_UPLO uplomkl       = upper_lower == math::UpLo::Upper ? MKL_UPPER : MKL_LOWER;

            cl::sycl::buffer<algorithmFPType, 1> a_sycl_buff = a_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            cl::sycl::buffer<algorithmFPType, 1> c_sycl_buff = c_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            status |= catchSyclExceptions([&]() mutable {
                innerSyrk(uplomkl, transmkl, n, k, alpha, a_sycl_buff, lda, beta, c_sycl_buff, ldc, offsetA, offsetC);
                _queue.wait_and_throw();
            });
        }

        return status;
    }

private:
    template <typename T>
    void innerSyrk(MKL_UPLO uplo, MKL_TRANSPOSE trans, int64_t n, int64_t k, T alpha, cl::sycl::buffer<T, 1> a, int64_t lda, T beta,
                   cl::sycl::buffer<T, 1> c, int64_t ldc, int64_t offset_a, int64_t offset_c);

    template <>
    void innerSyrk(MKL_UPLO uplo, MKL_TRANSPOSE trans, int64_t n, int64_t k, double alpha, cl::sycl::buffer<double, 1> a, int64_t lda, double beta,
                   cl::sycl::buffer<double, 1> c, int64_t ldc, int64_t offset_a, int64_t offset_c)
    {
        ::oneapi::fpk::gpu::dsyrk_sycl(&_queue, uplo, trans, n, k, alpha, &a, lda, beta, &c, ldc, offset_a, offset_c);
    }

    template <>
    void innerSyrk(MKL_UPLO uplo, MKL_TRANSPOSE trans, int64_t n, int64_t k, float alpha, cl::sycl::buffer<float, 1> a, int64_t lda, float beta,
                   cl::sycl::buffer<float, 1> c, int64_t ldc, int64_t offset_a, int64_t offset_c)
    {
        ::oneapi::fpk::gpu::ssyrk_sycl(&_queue, uplo, trans, n, k, alpha, &a, lda, beta, &c, ldc, offset_a, offset_c);
    }

    cl::sycl::queue & _queue;
};

/**
 *  <a name="DAAL-CLASS-ONEAPI-INTERNAL__MKLAXPY"></a>
 *  \brief Adapter for MKL AXPY routine
 */
template <typename algorithmFPType>
struct MKLAxpy
{
    MKLAxpy(cl::sycl::queue & queue) : _queue(queue) {}

    Status operator()(const int n, const algorithmFPType a, const Buffer<algorithmFPType> & x_buffer, const int incx,
                      Buffer<algorithmFPType> & y_buffer, const int incy)
    {
        Status status;

#ifdef DAAL_SYCL_INTERFACE_USM
        if (x_buffer.isUSMBacked() && y_buffer.isUSMBacked())
        {
            auto x_usm = x_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);

            auto y_usm = y_buffer.toUSM(status);
            DAAL_CHECK_STATUS_VAR(status);

            status |= catchSyclExceptions([&]() mutable {
                ::oneapi::fpk::blas::axpy(_queue, n, a, x_usm.get(), incx, y_usm.get(), incy);
                _queue.wait_and_throw();
            });
        }
        else
#endif
        {
            cl::sycl::buffer<algorithmFPType, 1> x_sycl_buff = x_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            cl::sycl::buffer<algorithmFPType, 1> y_sycl_buff = y_buffer.toSycl(status);
            DAAL_CHECK_STATUS_VAR(status);

            status |= catchSyclExceptions([&]() mutable {
                ::oneapi::fpk::blas::axpy(_queue, n, a, x_sycl_buff, incx, y_sycl_buff, incy);
                _queue.wait_and_throw();
            });
        }

        return status;
    }

private:
    cl::sycl::queue & _queue;
};

/** @} */
} // namespace interface1

using interface1::MKLGemm;
using interface1::MKLSyrk;

} // namespace math
} // namespace sycl
} // namespace internal
} // namespace services
} // namespace daal

#endif
