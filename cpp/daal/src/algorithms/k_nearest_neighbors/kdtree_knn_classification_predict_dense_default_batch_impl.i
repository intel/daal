/* file: kdtree_knn_classification_predict_dense_default_batch_impl.i */
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
//  Common functions for K-Nearest Neighbors predictions calculation
//--
*/

#ifndef __KDTREE_KNN_CLASSIFICATION_PREDICT_DENSE_DEFAULT_BATCH_IMPL_I__
#define __KDTREE_KNN_CLASSIFICATION_PREDICT_DENSE_DEFAULT_BATCH_IMPL_I__

#include "src/threading/threading.h"
#include "services/daal_defines.h"
#include "src/services/service_utils.h"
#include "algorithms/algorithm.h"
#include "services/daal_atomic_int.h"
#include "src/externals/service_memory.h"
#include "src/services/service_data_utils.h"
#include "src/externals/service_math.h"
#include "src/externals/service_rng.h"
#include "src/algorithms/service_sort.h"
#include "data_management/data/numeric_table.h"
#include "src/algorithms/k_nearest_neighbors/kdtree_knn_classification_predict_dense_default_batch.h"
#include "src/algorithms/k_nearest_neighbors/kdtree_knn_classification_model_impl.h"
#include "src/algorithms/k_nearest_neighbors/kdtree_knn_impl.i"
#include <iostream> // Temporary

namespace daal
{
namespace algorithms
{
namespace kdtree_knn_classification
{
namespace prediction
{
namespace internal
{
using namespace daal::services::internal;
using namespace daal::services;
using namespace daal::internal;
using namespace kdtree_knn_classification::internal;

template <CpuType cpu, typename T>
DAAL_FORCEINLINE T heapLeftChildIndex(T index)
{
    return 2 * index + 1;
}
template <CpuType cpu, typename T>
DAAL_FORCEINLINE T heapRightChildIndex(T index)
{
    return 2 * index + 2;
}
template <CpuType cpu, typename T>
DAAL_FORCEINLINE T heapParentIndex(T index)
{
    return (index - 1) / 2;
}

template <CpuType cpu, typename RandomAccessIterator>
void pushMaxHeap(RandomAccessIterator first, RandomAccessIterator last)
{
    if (first != last)
    {
        --last;
        auto i = last - first;
        if (i > 0)
        {
            const auto newItem = *last; // It can be moved instead.
            auto prev          = i;
            for (i = heapParentIndex<cpu>(i); prev && (*(first + i) < newItem); i = heapParentIndex<cpu>(i))
            {
                *(first + prev) = *(first + i); // It can be moved instead.
                prev            = i;
            }
            *(first + prev) = newItem; // It can be moved instead.
        }
    }
}

template <CpuType cpu, typename RandomAccessIterator, typename Diff>
DAAL_FORCEINLINE void internalAdjustMaxHeap(RandomAccessIterator first, RandomAccessIterator /*last*/, Diff count, Diff i)
{
    for (auto largest = i;; i = largest)
    {
        const auto l = heapLeftChildIndex<cpu>(i);
        if ((l < count) && (*(first + largest) < *(first + l)))
        {
            largest = l;
        }
        const auto r = heapRightChildIndex<cpu>(i);
        if ((r < count) && (*(first + largest) < *(first + r)))
        {
            largest = r;
        }

        if (largest == i)
        {
            break;
        }
        auto temp          = *(first + i);
        *(first + i)       = *(first + largest);
        *(first + largest) = temp; // Moving can be used instead.
    }
}

template <CpuType cpu, typename RandomAccessIterator>
void popMaxHeap(RandomAccessIterator first, RandomAccessIterator last)
{
    if (1 < last - first)
    {
        --last;
        auto temp = *first;
        *first    = *last;
        *last     = temp; // Moving can be used instead.
        internalAdjustMaxHeap<cpu>(first, last, last - first, first - first);
    }
}

template <CpuType cpu, typename RandomAccessIterator>
void makeMaxHeap(RandomAccessIterator first, RandomAccessIterator last)
{
    const auto count = last - first;
    auto i           = count / 2;
    while (0 < i)
    {
        internalAdjustMaxHeap<cpu>(first, last, count, --i);
    }
}

template <typename T, CpuType cpu>
class Heap
{
public:
    Heap() : _elements(nullptr), _count(0) {}

    ~Heap()
    {
        services::daal_free(_elements);
        _elements = nullptr;
    }

    bool init(size_t size)
    {
        _count    = 0;
        _elements = static_cast<T *>(daal::services::internal::service_malloc<T, cpu>(size));
        return _elements;
    }

    void clear()
    {
        if (_elements)
        {
            services::daal_free(_elements);
            _elements = nullptr;
        }
    }

    void reset() { _count = 0; }

    void push(const T & e, size_t k)
    {
        std::cout << "[heap.push] e = " << e << "; k = " << k << "; _count = " << _count << std::endl; // Temporary
        _elements[_count++] = e;

        { // Temporary
            std::cout << "[heap.push] _elements = {";
            for (size_t i = 0; i < _count; ++i)
            {
                std::cout << " " << _elements[i];
            }
            std::cout << " }" << std::endl;
        }

        if (_count == k)
        {
            std::cout << "[heap.push] Calling makeMaxHeap" << std::endl; // Temporary
            makeMaxHeap<cpu>(_elements, _elements + _count);

            { // Temporary
                std::cout << "[heap.push] _elements = {";
                for (size_t i = 0; i < _count; ++i)
                {
                    std::cout << " " << _elements[i];
                }
                std::cout << " }" << std::endl;
            }
        }
    }

    void replaceMax(const T & e)
    {
        std::cout << "[heap.replaceMax] e = " << e << "; _count = " << _count << std::endl; // Temporary
        std::cout << "[heap.replaceMax] Calling popMaxHeap" << std::endl;                   // Temporary
        popMaxHeap<cpu>(_elements, _elements + _count);
        _elements[_count - 1] = e;

        { // Temporary
            std::cout << "[heap.replaceMax] _elements = {";
            for (size_t i = 0; i < _count; ++i)
            {
                std::cout << " " << _elements[i];
            }
            std::cout << " }" << std::endl;
        }
        std::cout << "[heap.replaceMax] Calling pushMaxHeap" << std::endl; // Temporary

        pushMaxHeap<cpu>(_elements, _elements + _count);

        { // Temporary
            std::cout << "[heap.replaceMax] _elements = {";
            for (size_t i = 0; i < _count; ++i)
            {
                std::cout << " " << _elements[i];
            }
            std::cout << " }" << std::endl;
        }
    }

    size_t size() const { return _count; }

    T * getMax() { return _elements; }

    const T & operator[](size_t index) const { return *(_elements + index); }

private:
    T * _elements;
    size_t _count;
};

template <typename algorithmFpType, CpuType cpu>
struct GlobalNeighbors
{
    algorithmFpType distance;
    size_t index;

    inline bool operator<(const GlobalNeighbors & rhs) const { return (distance < rhs.distance); }
};

// Temporary
template <typename algorithmFpType, CpuType cpu>
inline std::ostream & operator<<(std::ostream & stream, const GlobalNeighbors<algorithmFpType, cpu> & rhs)
{
    return stream << "{ distance = " << rhs.distance << "; index = " << rhs.index << " }";
}

template <typename algorithmFpType>
struct SearchNode
{
    size_t nodeIndex;
    algorithmFpType minDistance;
};

template <typename algorithmFpType, CpuType cpu>
DAAL_FORCEINLINE bool checkHomogenSOA(const NumericTable & data, services::internal::TArrayScalable<algorithmFpType *, cpu> & soa_arrays)
{
    if (data.getDataLayout() & NumericTableIface::soa)
    {
        if (static_cast<const SOANumericTable &>(data).isHomogeneousFloatOrDouble())
        {
            auto f = (*const_cast<NumericTable &>(data).getDictionary())[0];
            if (daal::data_management::features::getIndexNumType<algorithmFpType>() == f.indexType)
            {
                const size_t xColumnCount = data.getNumberOfColumns();
                soa_arrays.reset(xColumnCount);

                for (size_t i = 0; i < xColumnCount; ++i)
                {
                    soa_arrays[i] = static_cast<algorithmFpType *>(static_cast<SOANumericTable &>(const_cast<NumericTable &>(data)).getArray(i));
                    if (!soa_arrays[i])
                    {
                        return false;
                    }
                }

                return true;
            }
        }
    }
    return false;
}

template <typename algorithmFpType, CpuType cpu>
DAAL_FORCEINLINE const algorithmFpType * getNtData(const bool isHomogenSOA, size_t feat_idx, size_t irow, size_t nrows, const NumericTable & data,
                                                   data_management::BlockDescriptor<algorithmFpType> & xBD,
                                                   services::internal::TArrayScalable<algorithmFpType *, cpu> & soa_arrays)
{
    if (isHomogenSOA)
    {
        return soa_arrays[feat_idx] + irow;
    }
    else
    {
        const_cast<NumericTable &>(data).getBlockOfColumnValues(feat_idx, irow, nrows, readOnly, xBD);
        return xBD.getBlockPtr();
    }
}

template <typename algorithmFpType, CpuType cpu>
DAAL_FORCEINLINE void releaseNtData(const bool isHomogenSOA, const NumericTable & data, data_management::BlockDescriptor<algorithmFpType> & xBD)
{
    if (!isHomogenSOA)
    {
        const_cast<NumericTable &>(data).releaseBlockOfColumnValues(xBD);
    }
}

template <typename algorithmFpType, CpuType cpu>
Status KNNClassificationPredictKernel<algorithmFpType, defaultDense, cpu>::compute(const NumericTable * x, const classifier::Model * m,
                                                                                   NumericTable * y, NumericTable * indices, NumericTable * distances,
                                                                                   const daal::algorithms::Parameter * par)
{
    Status status;

    typedef GlobalNeighbors<algorithmFpType, cpu> Neighbors;
    typedef Heap<Neighbors, cpu> MaxHeap;
    typedef kdtree_knn_classification::internal::Stack<SearchNode<algorithmFpType>, cpu> SearchStack;
    typedef daal::services::internal::MaxVal<algorithmFpType> MaxVal;
    typedef daal::internal::Math<algorithmFpType, cpu> Math;

    size_t k;
    VoteWeights voteWeights       = voteUniform;
    DAAL_UINT64 resultsToEvaluate = classifier::computeClassLabels;

    {
        auto par1 = dynamic_cast<const kdtree_knn_classification::interface1::Parameter *>(par);
        if (par1) k = par1->k;

        auto par2 = dynamic_cast<const kdtree_knn_classification::interface2::Parameter *>(par);
        if (par2)
        {
            k                 = par2->k;
            resultsToEvaluate = par2->resultsToEvaluate;
        }

        const auto par3 = dynamic_cast<const kdtree_knn_classification::interface3::Parameter *>(par);
        if (par3)
        {
            k                 = par3->k;
            voteWeights       = par3->voteWeights;
            resultsToEvaluate = par3->resultsToEvaluate;
        }

        if (par1 == NULL && par2 == NULL && par3 == NULL) return Status(ErrorNullParameterNotSupported);
    }

    const Model * const model    = static_cast<const Model *>(m);
    const auto & kdTreeTable     = *(model->impl()->getKDTreeTable());
    const auto rootTreeNodeIndex = model->impl()->getRootNodeIndex();
    const NumericTable & data    = *(model->impl()->getData());
    const NumericTable * labels  = nullptr;
    if (resultsToEvaluate != 0)
    {
        labels = model->impl()->getLabels().get();
    }

    const NumericTable * const modelIndices = model->impl()->getIndices().get();

    size_t iSize = 1;
    while (iSize < k)
    {
        iSize *= 2;
    }
    const size_t heapSize = (iSize / 16 + 1) * 16;

    const size_t xRowCount        = x->getNumberOfRows();
    const algorithmFpType base    = 2.0;
    const size_t expectedMaxDepth = (Math::sLog(xRowCount) / Math::sLog(base) + 1) * __KDTREE_DEPTH_MULTIPLICATION_FACTOR;
    const size_t stackSize        = Math::sPowx(base, Math::sCeil(Math::sLog(expectedMaxDepth) / Math::sLog(base)));
    struct Local
    {
        MaxHeap heap;
        SearchStack stack;
    };
    daal::tls<Local *> localTLS([&]() -> Local * {
        Local * const ptr = service_scalable_calloc<Local, cpu>(1);
        if (ptr)
        {
            if (!ptr->heap.init(heapSize))
            {
                status.add(services::ErrorMemoryAllocationFailed);
                service_scalable_free<Local, cpu>(ptr);
                return nullptr;
            }
            if (!ptr->stack.init(stackSize))
            {
                status.add(services::ErrorMemoryAllocationFailed);
                ptr->heap.clear();
                service_scalable_free<Local, cpu>(ptr);
                return nullptr;
            }
        }
        else
        {
            status.add(services::ErrorMemoryAllocationFailed);
        }
        return ptr;
    });

    DAAL_CHECK_STATUS_OK((status.ok()), status);

    const auto maxThreads     = threader_get_threads_number();
    const size_t xColumnCount = x->getNumberOfColumns();
    const auto rowsPerBlock   = (xRowCount + maxThreads - 1) / maxThreads;
    const auto blockCount     = (xRowCount + rowsPerBlock - 1) / rowsPerBlock;
    SafeStatus safeStat;

    services::internal::TArrayScalable<algorithmFpType *, cpu> soa_arrays;
    bool isHomogenSOA = checkHomogenSOA<algorithmFpType, cpu>(data, soa_arrays);

    daal::threader_for(blockCount, blockCount, [&](int iBlock) {
        Local * const local = localTLS.local();
        if (local)
        {
            services::Status s;

            const size_t first = iBlock * rowsPerBlock;
            const size_t last  = min<cpu>(static_cast<decltype(xRowCount)>(first + rowsPerBlock), xRowCount);

            const algorithmFpType radius = MaxVal::get();
            data_management::BlockDescriptor<algorithmFpType> xBD;
            const_cast<NumericTable &>(*x).getBlockOfRows(first, last - first, readOnly, xBD);
            const algorithmFpType * const dx = xBD.getBlockPtr();

            data_management::BlockDescriptor<algorithmFpType> indicesBD, distancesBD;
            if (indices)
            {
                s = indices->getBlockOfRows(first, last - first, writeOnly, indicesBD);
                DAAL_CHECK_STATUS_THR(s);
            }
            if (distances)
            {
                s = distances->getBlockOfRows(first, last - first, writeOnly, distancesBD);
                DAAL_CHECK_STATUS_THR(s);
            }

            if (labels)
            {
                const size_t yColumnCount = y->getNumberOfColumns();
                data_management::BlockDescriptor<algorithmFpType> yBD;
                y->getBlockOfRows(first, last - first, writeOnly, yBD);
                auto * const dy = yBD.getBlockPtr();

                for (size_t i = 0; i < last - first; ++i)
                {
                    findNearestNeighbors(&dx[i * xColumnCount], local->heap, local->stack, k, radius, kdTreeTable, rootTreeNodeIndex, data,
                                         isHomogenSOA, soa_arrays);
                    s = predict(&(dy[i * yColumnCount]), local->heap, labels, k, voteWeights, modelIndices, indicesBD, distancesBD, i);
                    DAAL_CHECK_STATUS_THR(s)
                }

                s |= y->releaseBlockOfRows(yBD);
                DAAL_CHECK_STATUS_THR(s);
            }
            else
            {
                for (size_t i = 0; i < last - first; ++i)
                {
                    findNearestNeighbors(&dx[i * xColumnCount], local->heap, local->stack, k, radius, kdTreeTable, rootTreeNodeIndex, data,
                                         isHomogenSOA, soa_arrays);
                    s = predict(nullptr, local->heap, labels, k, voteWeights, modelIndices, indicesBD, distancesBD, i);
                    DAAL_CHECK_STATUS_THR(s)
                }
            }

            if (indices)
            {
                s |= indices->releaseBlockOfRows(indicesBD);
            }
            DAAL_CHECK_STATUS_THR(s);
            if (distances)
            {
                s |= distances->releaseBlockOfRows(distancesBD);
            }
            DAAL_CHECK_STATUS_THR(s);

            const_cast<NumericTable &>(*x).releaseBlockOfRows(xBD);
        }
    });

    DAAL_CHECK_SAFE_STATUS()

    localTLS.reduce([&](Local * ptr) -> void {
        if (ptr)
        {
            ptr->stack.clear();
            ptr->heap.clear();
            service_scalable_free<Local, cpu>(ptr);
        }
    });
    return status;
}

template <typename algorithmFpType, CpuType cpu>
DAAL_FORCEINLINE void computeDistance(size_t start, size_t end, algorithmFpType * distance, const algorithmFpType * query, const bool isHomogenSOA,
                                      const NumericTable & data, data_management::BlockDescriptor<algorithmFpType> xBD[2],
                                      services::internal::TArrayScalable<algorithmFpType *, cpu> & soa_arrays)
{
    for (size_t i = start; i < end; ++i)
    {
        distance[i - start] = 0;
    }

    size_t curBDIdx  = 0;
    size_t nextBDIdx = 1;

    const size_t xColumnCount = data.getNumberOfColumns();

    const algorithmFpType * nx = nullptr;
    const algorithmFpType * dx = getNtData(isHomogenSOA, 0, start, end - start, data, xBD[curBDIdx], soa_arrays);

    size_t j;
    for (j = 1; j < xColumnCount; ++j)
    {
        nx = getNtData(isHomogenSOA, j, start, end - start, data, xBD[nextBDIdx], soa_arrays);

        DAAL_PREFETCH_READ_T0(nx);
        DAAL_PREFETCH_READ_T0(nx + 16);

        for (size_t i = 0; i < end - start; ++i)
        {
            distance[i] += (query[j - 1] - dx[i]) * (query[j - 1] - dx[i]);
        }

        releaseNtData<algorithmFpType, cpu>(isHomogenSOA, data, xBD[curBDIdx]);

        services::internal::swap<cpu, size_t>(curBDIdx, nextBDIdx);
        services::internal::swap<cpu, const algorithmFpType *>(dx, nx);
    }
    {
        for (size_t i = 0; i < end - start; ++i)
        {
            distance[i] += (query[j - 1] - dx[i]) * (query[j - 1] - dx[i]);
        }

        releaseNtData<algorithmFpType, cpu>(isHomogenSOA, data, xBD[curBDIdx]);
    }
}

template <typename algorithmFpType, CpuType cpu>
void KNNClassificationPredictKernel<algorithmFpType, defaultDense, cpu>::findNearestNeighbors(
    const algorithmFpType * query, Heap<GlobalNeighbors<algorithmFpType, cpu>, cpu> & heap,
    kdtree_knn_classification::internal::Stack<SearchNode<algorithmFpType>, cpu> & stack, size_t k, algorithmFpType radius,
    const KDTreeTable & kdTreeTable, size_t rootTreeNodeIndex, const NumericTable & data, const bool isHomogenSOA,
    services::internal::TArrayScalable<algorithmFpType *, cpu> & soa_arrays)
{
    heap.reset();
    stack.reset();
    GlobalNeighbors<algorithmFpType, cpu> curNeighbor;
    size_t i, j;
    SearchNode<algorithmFpType> cur, toPush;
    const KDTreeNode * node;
    cur.nodeIndex   = rootTreeNodeIndex;
    cur.minDistance = 0;

    const size_t xColumnCount = data.getNumberOfColumns();

    DAAL_ALIGNAS(256) algorithmFpType distance[__KDTREE_LEAF_BUCKET_SIZE + 1];
    size_t start, end;

    data_management::BlockDescriptor<algorithmFpType> xBD[2];
    size_t curBDIdx, nextBDIdx;
    for (;;)
    {
        node = static_cast<const KDTreeNode *>(kdTreeTable.getArray()) + cur.nodeIndex;
        if (node->dimension == __KDTREE_NULLDIMENSION)
        {
            start = node->leftIndex;
            end   = node->rightIndex;

            computeDistance<algorithmFpType, cpu>(start, end, distance, query, isHomogenSOA, data, xBD, soa_arrays);

            for (i = start; i < end; ++i)
            {
                if (distance[i - start] <= radius)
                {
                    curNeighbor.distance = distance[i - start];
                    curNeighbor.index    = i;
                    if (heap.size() < k)
                    {
                        heap.push(curNeighbor, k);

                        if (heap.size() == k)
                        {
                            radius = heap.getMax()->distance;
                        }
                    }
                    else
                    {
                        if (heap.getMax()->distance > curNeighbor.distance)
                        {
                            heap.replaceMax(curNeighbor);
                            radius = heap.getMax()->distance;
                        }
                    }
                }
            }

            if (!stack.empty())
            {
                cur = stack.pop();
                DAAL_PREFETCH_READ_T0(static_cast<const KDTreeNode *>(kdTreeTable.getArray()) + cur.nodeIndex);
            }
            else
            {
                break;
            }
        }
        else
        {
            algorithmFpType val        = query[node->dimension];
            const algorithmFpType diff = val - node->cutPoint;

            if (cur.minDistance <= radius)
            {
                cur.nodeIndex    = (diff < 0) ? node->leftIndex : node->rightIndex;
                toPush.nodeIndex = (diff < 0) ? node->rightIndex : node->leftIndex;
                val -= node->cutPoint;
                toPush.minDistance = cur.minDistance + val * val;
                stack.push(toPush);
            }
            else if (!stack.empty())
            {
                cur = stack.pop();
                DAAL_PREFETCH_READ_T0(static_cast<const KDTreeNode *>(kdTreeTable.getArray()) + cur.nodeIndex);
            }
            else
            {
                break;
            }
        }
    }

    { // Temporary
        std::cout << "[findNearestNeighbors] heap.size() = " << heap.size() << "; heap._elements = {" << std::endl;
        for (size_t i = 0; i < heap.size(); ++i)
        {
            std::cout << " " << heap[i];
        }
        std::cout << " }" << std::endl;
    }
}

template <typename algorithmFpType, CpuType cpu>
services::Status KNNClassificationPredictKernel<algorithmFpType, defaultDense, cpu>::predict(
    algorithmFpType * predictedClass, const Heap<GlobalNeighbors<algorithmFpType, cpu>, cpu> & heap, const NumericTable * labels, size_t k,
    VoteWeights voteWeights, const NumericTable * modelIndices, data_management::BlockDescriptor<algorithmFpType> & indices,
    data_management::BlockDescriptor<algorithmFpType> & distances, size_t index)
{
    typedef daal::internal::Math<algorithmFpType, cpu> Math;

    const size_t heapSize = heap.size();
    if (heapSize < 1) return services::Status();

    if (indices.getNumberOfRows() != 0)
    {
        DAAL_ASSERT(modelIndices);

        services::Status s;
        data_management::BlockDescriptor<algorithmFpType> modelIndicesBD;

        const auto nIndices = indices.getNumberOfColumns();
        DAAL_ASSERT(heapSize <= nIndices);

        algorithmFpType * const indicesPtr = indices.getBlockPtr() + index * nIndices;

        for (size_t i = 0; i < heapSize; ++i)
        {
            s |= const_cast<NumericTable *>(modelIndices)->getBlockOfRows(heap[i].index, 1, readOnly, modelIndicesBD);
            DAAL_ASSERT(s.ok());

            indicesPtr[i] = *(modelIndicesBD.getBlockPtr());

            s |= const_cast<NumericTable *>(modelIndices)->releaseBlockOfRows(modelIndicesBD);
            DAAL_ASSERT(s.ok());
        }

        for (size_t i = heapSize; i < nIndices; ++i)
        {
            indicesPtr[i] = static_cast<size_t>(-1);
        }
    }

    if (distances.getNumberOfRows() != 0)
    {
        services::Status s;

        const auto nDistances = distances.getNumberOfColumns();
        DAAL_ASSERT(heapSize <= nDistances);

        algorithmFpType * const distancesPtr = distances.getBlockPtr() + index * nDistances;
        for (size_t i = 0; i < heapSize; ++i)
        {
            distancesPtr[i] = heap[i].distance;
        }

        Math::vSqrt(heapSize, distancesPtr, distancesPtr);

        for (size_t i = heapSize; i < nDistances; ++i)
        {
            distancesPtr[i] = -1;
        }
    }

    if (labels)
    {
        DAAL_ASSERT(predictedClass);

        data_management::BlockDescriptor<algorithmFpType> labelBD;
        algorithmFpType * classes = static_cast<algorithmFpType *>(daal::services::internal::service_malloc<algorithmFpType, cpu>(heapSize));
        DAAL_CHECK_MALLOC(classes);
        for (size_t i = 0; i < heapSize; ++i)
        {
            const_cast<NumericTable *>(labels)->getBlockOfColumnValues(0, heap[i].index, 1, readOnly, labelBD);
            classes[i] = *(labelBD.getBlockPtr());
            const_cast<NumericTable *>(labels)->releaseBlockOfColumnValues(labelBD);
        }
        daal::algorithms::internal::qSort<algorithmFpType, cpu>(heapSize, classes);
        algorithmFpType currentClass = classes[0];
        algorithmFpType winnerClass  = currentClass;

        if (voteWeights == voteUniform)
        {
            size_t currentWeight = 1;
            size_t winnerWeight  = currentWeight;
            for (size_t i = 1; i < heapSize; ++i)
            {
                if (classes[i] == currentClass)
                {
                    if ((++currentWeight) > winnerWeight)
                    {
                        winnerWeight = currentWeight;
                        winnerClass  = currentClass;
                    }
                }
                else
                {
                    currentWeight = 1;
                    currentClass  = classes[i];
                }
            }
            *predictedClass = winnerClass;
        }
        else
        {
            DAAL_ASSERT(voteWeights == voteDistance);

            const algorithmFpType epsilon = daal::services::internal::EpsilonVal<algorithmFpType>::get();

            bool isContainZero = false;

            for (size_t i = 0; i < heapSize; ++i)
            {
                if (heap[i].distance <= epsilon)
                {
                    isContainZero = true;
                    break;
                }
            }

            if (isContainZero)
            {
                size_t currentWeight = (heap[0].distance <= epsilon);
                size_t winnerWeight  = currentWeight;
                for (size_t i = 1; i < heapSize; ++i)
                {
                    if (classes[i] == currentClass)
                    {
                        currentWeight += (heap[i].distance <= epsilon);
                    }
                    else
                    {
                        currentWeight = (heap[i].distance <= epsilon);
                        currentClass  = classes[i];
                    }

                    if (currentWeight > winnerWeight)
                    {
                        winnerWeight = currentWeight;
                        winnerClass  = currentClass;
                    }
                }
                *predictedClass = winnerClass;
            }
            else
            {
                algorithmFpType currentWeight = Math::sSqrt(1.0 / heap[0].distance);
                algorithmFpType winnerWeight  = currentWeight;
                for (size_t i = 1; i < heapSize; ++i)
                {
                    if (classes[i] == currentClass)
                    {
                        currentWeight += Math::sSqrt(1.0 / heap[i].distance);
                    }
                    else
                    {
                        currentWeight = Math::sSqrt(1.0 / heap[i].distance);
                        currentClass  = classes[i];
                    }

                    if (currentWeight > winnerWeight)
                    {
                        winnerWeight = currentWeight;
                        winnerClass  = currentClass;
                    }
                }
                *predictedClass = winnerClass;
            }
        }

        service_free<algorithmFpType, cpu>(classes);
        classes = nullptr;
    }

    return services::Status();
}

} // namespace internal
} // namespace prediction
} // namespace kdtree_knn_classification
} // namespace algorithms
} // namespace daal

#endif
