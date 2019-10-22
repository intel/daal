/* file: buffer.h */
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

/*
//++
// Declaration and implementation of the shared pointer class.
//--
*/

#ifndef __DAAL_SERVICES_BUFFER_H__
#define __DAAL_SERVICES_BUFFER_H__

#include "services/internal/buffer_impl.h"

#ifdef DAAL_SYCL_INTERFACE
#include "services/internal/buffer_impl_sycl.h"
#endif

namespace daal
{
namespace services
{
namespace interface1
{
/**
 * @ingroup sycl
 * @{
 */

/**
 *  <a name="DAAL-CLASS-SERVICES__BUFFER"></a>
 *  \brief Wrapper for a SYCL* buffer.
 *  Can hold data on the host side using shared pointer,
 *  or on host/device sides using SYCL* buffer.
 */
template<typename T>
class Buffer : public Base
{
public:
    /**
     *   Creates empty Buffer object
     */
    Buffer() { }

#ifdef DAAL_SYCL_INTERFACE
    /**
     *  Creates a Buffer object referencing a SYCL* buffer.
     *  Does not copy the data from the SYCL* buffer.
     */
    Buffer(const cl::sycl::buffer<T, 1> &buffer) :
        _impl(new internal::SyclBuffer<T>(buffer)) { }
#endif

#ifdef DAAL_SYCL_INTERFACE_USM
    /**
     *  Creates a Buffer object referencing a USM pointer.
     *  Does not copy the data from the USM pointer.
     *  \param[in] usmData    USM pointer
     *  \param[in] size       Number of elements of type T stored in USM memory block
     *  \param[in] allocType  USM allocation type
     */
    Buffer(const SharedPtr<T> &usmData, size_t size, cl::sycl::usm::alloc allocType) :
        _impl(new internal::UsmBuffer<T>(usmData, size, allocType)) { }
#endif

    /**
     *   Creates a Buffer object from host-allocated raw pointer.
     *   Buffer does not own this pointer.
     */
    Buffer(T *data, size_t size) :
        _impl(new internal::HostBuffer<T>(data, size)) { }

    /**
     *   Creates a Buffer object referencing the shared pointer to the host-allocated data.
     */
    Buffer(const SharedPtr<T> &data, size_t size) :
        _impl(new internal::HostBuffer<T>(data, size)) { }

    /**
     *  Returns true if Buffer points to any data
     */
    operator bool() const
    {
        return _impl;
    }

    /**
     *  Returns true if Buffer is equal to \p other
     */
    bool operator==(const Buffer &other) const
    {
        return _impl.get() == other._impl.get();
    }

    /**
     *  Returns true if Buffer is not equal to \p other
     */
    bool operator!=(const Buffer &other) const
    {
        return _impl.get() != other._impl.get();
    }

    /**
     *  Converts data inside the buffer to the host side.
     *  \param[in] rwFlag  Access flag to the data
     *  \return host-allocated shared pointer to the data.
     */
    inline SharedPtr<T> toHost(const data_management::ReadWriteMode& rwFlag) const
    {
        return internal::HostBufferConverter<T>().toHost(*_impl, rwFlag);
    }

#ifdef DAAL_SYCL_INTERFACE
    /**
     *  Converts data to the SYCL* buffer.
     *  \return one-dimensional SYCL* buffer.
     */
    inline cl::sycl::buffer<T, 1> toSycl() const
    {
        // TODO: Handle the case if _impl is empty
        return internal::SyclBufferConverter<T>().toSycl(*_impl);
    }
#endif


#ifdef DAAL_SYCL_INTERFACE_USM
    inline SharedPtr<T> toUSM() const
    {
        return internal::SyclBufferConverter<T>().toUSM(*_impl);
    }
#endif

    /**
     *   Returns the total number of elements in the buffer.
     */
    inline size_t size() const
    {
        // TODO: Handle the case if _impl is empty
        return _impl->size();
    }

    /**
     *   Drops underlying reference to the data from the buffer and makes it empty
     */
    inline void reset()
    {
        _impl.reset();
    }

    /**
     *   Creates Buffer object that points to the same memory as a parent but with offset
     *   \param[in] offset Offset in elements from start of the parent buffer
     *   \param[in] size   Number of elements in the sub-buffer
     */
    inline Buffer<T> getSubBuffer(size_t offset, size_t size) const
    {
        return Buffer<T>(_impl->getSubBuffer(offset, size));
    }

private:
    explicit Buffer(internal::BufferIface<T> *impl) : _impl(impl) { }
    explicit Buffer(const SharedPtr< internal::BufferIface<T> >& impl) : _impl(impl) { }

    SharedPtr<internal::BufferIface<T> > _impl;
};

/** @} */
} // namespace interface1

using interface1::Buffer;

} // namespace services
} // namespace daal

#endif
