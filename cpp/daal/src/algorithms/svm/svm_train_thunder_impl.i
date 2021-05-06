/* file: svm_train_thunder_impl.i */
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

/*
//++
//  SVM training algorithm implementation using Thunder method
//--
*/
/*
//  DESCRIPTION
//
//  Definition of the functions for training with SVM 2-class classifier.
//
//  REFERENCES
//
//  1. Zeyi Wen, Jiashuai Shi, Bingsheng He
//     ThunderSVM: A Fast SVM Library on GPUs and CPUs,
//     Journal of Machine Learning Research, 19, 1-5 (2018)
//  2. Rong-En Fan, Pai-Hsuen Chen, Chih-Jen Lin,
//     Working Set Selection Using Second Order Information
//     for Training Support Vector Machines,
//     Journal of Machine Learning Research 6 (2005), pp. 1889___1918
//  3. Bernard E. boser, Isabelle M. Guyon, Vladimir N. Vapnik,
//     A Training Algorithm for Optimal Margin Classifiers.
//  4. Thorsten Joachims, Making Large-Scale SVM Learning Practical,
//     Advances in Kernel Methods - Support Vector Learning
*/

#ifndef __SVM_TRAIN_THUNDER_I__
#define __SVM_TRAIN_THUNDER_I__

#include "src/externals/service_memory.h"
#include "src/data_management/service_micro_table.h"
#include "src/data_management/service_numeric_table.h"
#include "src/services/service_utils.h"
#include "src/services/service_data_utils.h"
#include "src/externals/service_ittnotify.h"
#include "src/externals/service_blas.h"
#include "src/externals/service_math.h"

#include "src/algorithms/svm/svm_train_common.h"
#include "src/algorithms/svm/svm_train_thunder_workset.h"
#include "src/algorithms/svm/svm_train_thunder_cache.h"
#include "src/algorithms/svm/svm_train_result.h"

#include "src/algorithms/svm/svm_train_common_impl.i"

namespace daal
{
namespace algorithms
{
namespace svm
{
namespace training
{
namespace internal
{
template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::compute(const NumericTablePtr & xTable, const NumericTablePtr & wTable,
                                                                      NumericTable & yTable, daal::algorithms::Model * r,
                                                                      const KernelParameter & svmPar)
{
    DAAL_ITTNOTIFY_SCOPED_TASK(COMPUTE);

    services::Status status;

    const algorithmFPType C                 = svmPar.C;
    const algorithmFPType accuracyThreshold = svmPar.accuracyThreshold;
    const algorithmFPType tau               = svmPar.tau;
    const algorithmFPType epsilon           = svmPar.epsilon;
    const algorithmFPType nu                = svmPar.nu;
    const size_t maxIterations              = svmPar.maxIterations;
    const size_t cacheSize                  = svmPar.cacheSize;
    const auto kernel                       = svmPar.kernel->clone();
    const auto svmType                      = svmPar.svmType;

    const size_t nVectors = xTable->getNumberOfRows();
    DAAL_OVERFLOW_CHECK_BY_MULTIPLICATION(size_t, nVectors, 2);
    const size_t nTrainVectors = (svmType == SvmType::regression || svmType == SvmType::nu_regression) ? nVectors * 2 : nVectors;
    TArray<algorithmFPType, cpu> yTArray(nTrainVectors);
    DAAL_CHECK_MALLOC(yTArray.get());
    algorithmFPType * const y = yTArray.get();

    TArray<algorithmFPType, cpu> gradTArray(nTrainVectors);
    DAAL_CHECK_MALLOC(gradTArray.get());
    algorithmFPType * const grad = gradTArray.get();

    TArray<algorithmFPType, cpu> alphaTArray(nTrainVectors);
    DAAL_CHECK_MALLOC(alphaTArray.get());
    algorithmFPType * const alpha = alphaTArray.get();

    TArray<algorithmFPType, cpu> cwTArray(nTrainVectors);
    DAAL_CHECK_MALLOC(cwTArray.get());
    algorithmFPType * const cw = cwTArray.get();

    size_t nNonZeroWeights = nTrainVectors;
    if (svmType == SvmType::classification || svmType == SvmType::nu_classification)
    {
        DAAL_CHECK_STATUS(status, classificationInit(yTable, wTable, C, y, grad, alpha, cw, nNonZeroWeights, nu, svmType));
    }
    else
    {
        DAAL_CHECK_STATUS(status, regressionInit(yTable, wTable, C, epsilon, y, grad, alpha, cw, nNonZeroWeights, nu, svmType));
    }

    TaskWorkingSet<algorithmFPType, cpu> workSet(nNonZeroWeights, nTrainVectors, maxBlockSize, svmType);
    DAAL_CHECK_STATUS(status, workSet.init());
    const size_t nWS = workSet.getSize();

    algorithmFPType diff     = algorithmFPType(0);
    algorithmFPType diffPrev = algorithmFPType(0);
    size_t sameLocalDiff     = 0;

    TArray<algorithmFPType, cpu> buffer(nWS * MemSmoId::latest + nWS * nWS);
    DAAL_CHECK_MALLOC(buffer.get());

    TArray<algorithmFPType, cpu> deltaAlpha(nWS);
    DAAL_CHECK_MALLOC(deltaAlpha.get());

    TArray<char, cpu> I(nWS);
    DAAL_CHECK_MALLOC(I.get());

    size_t defaultCacheSize = services::internal::min<cpu, size_t>(nVectors, cacheSize / nVectors / sizeof(algorithmFPType));
    defaultCacheSize        = services::internal::max<cpu, size_t>(nWS, defaultCacheSize);
    auto cachePtr           = SVMCache<thunder, lruCache, algorithmFPType, cpu>::create(defaultCacheSize, nWS, nVectors, xTable, kernel, status);
    DAAL_CHECK_STATUS_VAR(status);

    if (svmType == SvmType::nu_classification || svmType == SvmType::nu_regression)
    {
        DAAL_CHECK_STATUS(status, initGrad(xTable, kernel, nVectors, nTrainVectors, cacheSize, y, alpha, grad));
    }

    size_t iter = 0;
    for (; iter < maxIterations; ++iter)
    {
        if (iter != 0)
        {
            DAAL_CHECK_STATUS(status, workSet.copyLastToFirst());
        }

        DAAL_CHECK_STATUS(status, workSet.select(y, alpha, grad, cw));
        const uint32_t * const wsIndices = workSet.getIndices();
        algorithmFPType ** kernelSOARes  = nullptr;
        {
            DAAL_ITTNOTIFY_SCOPED_TASK(getRowsBlock);

            DAAL_CHECK_STATUS(status, cachePtr->getRowsBlock(wsIndices, nWS, kernelSOARes));
        }

        DAAL_CHECK_STATUS(status, SMOBlockSolver(y, grad, wsIndices, kernelSOARes, nVectors, nWS, cw, accuracyThreshold, tau, buffer.get(), I.get(),
                                                 alpha, deltaAlpha.get(), diff, svmType));

        DAAL_CHECK_STATUS(status, updateGrad(kernelSOARes, deltaAlpha.get(), grad, nVectors, nTrainVectors, nWS));
        if (checkStopCondition(diff, diffPrev, accuracyThreshold, sameLocalDiff) && iter >= nNoChanges) break;
        diffPrev = diff;
    }

    cachePtr->clear();
    SaveResultTask<algorithmFPType, cpu> saveResult(nVectors, y, alpha, grad, svmType, cachePtr.get());
    DAAL_CHECK_STATUS(status, saveResult.compute(xTable, *static_cast<Model *>(r), cw));

    return status;
}

template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::classificationInit(NumericTable & yTable, const NumericTablePtr & wTable,
                                                                                 const algorithmFPType c, algorithmFPType * const y,
                                                                                 algorithmFPType * const grad, algorithmFPType * const alpha,
                                                                                 algorithmFPType * const cw, size_t & nNonZeroWeights,
                                                                                 const algorithmFPType nu, const SvmType svmType)
{
    services::Status status;
    const size_t nVectors = yTable.getNumberOfRows();
    /* The operation copy is lightweight, therefore a large size is chosen
          so that the number of blocks is a reasonable number. */
    const size_t blockSize = 16384;
    const size_t nBlocks   = nVectors / blockSize + !!(nVectors % blockSize);

    DAAL_ITTNOTIFY_SCOPED_TASK(init.set);
    TlsSum<size_t, cpu> weightsCounter(1);
    SafeStatus safeStat;
    daal::threader_for(nBlocks, nBlocks, [&](const size_t iBlock) {
        const size_t startRow     = iBlock * blockSize;
        const size_t nRowsInBlock = (iBlock != nBlocks - 1) ? blockSize : nVectors - iBlock * blockSize;

        ReadColumns<algorithmFPType, cpu> mtY(yTable, 0, startRow, nRowsInBlock);
        DAAL_CHECK_BLOCK_STATUS_THR(mtY);
        const algorithmFPType * const yIn = mtY.get();

        ReadColumns<algorithmFPType, cpu> mtW(wTable.get(), 0, startRow, nRowsInBlock);
        DAAL_CHECK_BLOCK_STATUS_THR(mtW);
        const algorithmFPType * const weights = mtW.get();

        size_t * const wc = weights ? weightsCounter.local() : nullptr;
        if (svmType == SvmType::classification)
        {
            for (size_t i = 0; i < nRowsInBlock; ++i)
            {
                y[i + startRow]     = yIn[i] == algorithmFPType(0) ? algorithmFPType(-1) : yIn[i];
                grad[i + startRow]  = -y[i + startRow];
                alpha[i + startRow] = algorithmFPType(0);
                cw[i + startRow]    = weights ? weights[i] * c : c;
                if (weights)
                {
                    *wc += static_cast<size_t>(weights[i] != algorithmFPType(0));
                }
            }
        }
        else if (svmType == SvmType::nu_classification)
        {
            for (size_t i = 0; i < nRowsInBlock; ++i)
            {
                y[i + startRow]    = yIn[i] == algorithmFPType(0) ? algorithmFPType(-1) : yIn[i];
                grad[i + startRow] = algorithmFPType(0);
                cw[i + startRow]   = weights ? weights[i] * c : c;
                if (weights)
                {
                    *wc += static_cast<size_t>(weights[i] != algorithmFPType(0));
                }
            }
        }
    });

    if (wTable.get())
    {
        weightsCounter.reduceTo(&nNonZeroWeights, 1);
    }

    if (svmType == SvmType::nu_classification)
    {
        algorithmFPType sum_p = nu * nVectors / algorithmFPType(2);
        algorithmFPType sum_n = nu * nVectors / algorithmFPType(2);

        for (size_t i = 0; i < nVectors; ++i)
        {
            if (y[i] > 0)
            {
                alpha[i] = services::internal::min<cpu, algorithmFPType>(sum_p, 1);
                sum_p -= alpha[i];
            }
            else
            {
                alpha[i] = services::internal::min<cpu, algorithmFPType>(sum_n, 1);
                sum_n -= alpha[i];
            }
        }
    }

    return status;
}

template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::regressionInit(NumericTable & yTable, const NumericTablePtr & wTable,
                                                                             const algorithmFPType c, const algorithmFPType epsilon,
                                                                             algorithmFPType * const y, algorithmFPType * const grad,
                                                                             algorithmFPType * const alpha, algorithmFPType * const cw,
                                                                             size_t & nNonZeroWeights, const algorithmFPType nu,
                                                                             const SvmType svmType)
{
    services::Status status;
    const size_t nVectors = yTable.getNumberOfRows();
    /* The operation copy is lightweight, therefore a large size is chosen
          so that the number of blocks is a reasonable number. */
    const size_t blockSize = 16384;
    const size_t nBlocks   = nVectors / blockSize + !!(nVectors % blockSize);

    TlsSum<size_t, cpu> weightsCounter(1);
    SafeStatus safeStat;
    daal::threader_for(nBlocks, nBlocks, [&](const size_t iBlock) {
        const size_t startRow     = iBlock * blockSize;
        const size_t nRowsInBlock = (iBlock != nBlocks - 1) ? blockSize : nVectors - iBlock * blockSize;

        ReadColumns<algorithmFPType, cpu> mtY(yTable, 0, startRow, nRowsInBlock);
        DAAL_CHECK_BLOCK_STATUS_THR(mtY);
        const algorithmFPType * const yIn = mtY.get();

        ReadColumns<algorithmFPType, cpu> mtW(wTable.get(), 0, startRow, nRowsInBlock);
        DAAL_CHECK_BLOCK_STATUS_THR(mtW);
        const algorithmFPType * const weights = mtW.get();

        size_t * const wc = weights ? weightsCounter.local() : nullptr;
        if (svmType == SvmType::regression)
        {
            for (size_t i = 0; i < nRowsInBlock; ++i)
            {
                y[i + startRow]            = algorithmFPType(1.0);
                y[i + startRow + nVectors] = algorithmFPType(-1.0);

                grad[i + startRow]            = epsilon - yIn[i];
                grad[i + startRow + nVectors] = -epsilon - yIn[i];

                alpha[i + startRow]            = algorithmFPType(0);
                alpha[i + startRow + nVectors] = algorithmFPType(0);

                cw[i + startRow]            = weights ? weights[i] * c : c;
                cw[i + startRow + nVectors] = weights ? weights[i] * c : c;
                if (weights)
                {
                    *wc += static_cast<size_t>(weights[i] != algorithmFPType(0));
                }
            }
        }
        else if (svmType == SvmType::nu_regression)
        {
            for (size_t i = 0; i < nRowsInBlock; ++i)
            {
                y[i + startRow]            = algorithmFPType(1.0);
                y[i + startRow + nVectors] = algorithmFPType(-1.0);

                grad[i + startRow]            = -yIn[i];
                grad[i + startRow + nVectors] = -yIn[i];

                cw[i + startRow]            = weights ? weights[i] * c : c;
                cw[i + startRow + nVectors] = weights ? weights[i] * c : c;
                if (weights)
                {
                    *wc += static_cast<size_t>(weights[i] != algorithmFPType(0));
                }
            }
        }
    });

    if (wTable.get())
    {
        weightsCounter.reduceTo(&nNonZeroWeights, 1);
    }

    if (svmType == SvmType::nu_regression)
    {
        algorithmFPType sum = c * nu * nVectors / algorithmFPType(2);
        for (size_t i = 0; i < nVectors; ++i)
        {
            alpha[i] = alpha[i + nVectors] = services::internal::min<cpu, algorithmFPType>(sum, c);
            sum -= alpha[i];
        }
    }

    return status;
}

template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::SMOBlockSolver(
    const algorithmFPType * y, const algorithmFPType * grad, const uint32_t * wsIndices, algorithmFPType ** kernelWS, const size_t nVectors,
    const size_t nWS, const algorithmFPType * cw, const double accuracyThreshold, const double tau, algorithmFPType * buffer, char * I,
    algorithmFPType * alpha, algorithmFPType * deltaAlpha, algorithmFPType & localDiff, SvmType svmType) const
{
    DAAL_ITTNOTIFY_SCOPED_TASK(SMOBlockSolver);
    services::Status status;

    const size_t innerMaxIterations(nWS * cInnerIterations);

    algorithmFPType * const alphaLocal    = buffer + nWS * MemSmoId::alphaBuffID;
    algorithmFPType * const yLocal        = buffer + nWS * MemSmoId::yBuffID;
    algorithmFPType * const gradLocal     = buffer + nWS * MemSmoId::gradBuffID;
    algorithmFPType * const kdLocal       = buffer + nWS * MemSmoId::kdBuffID;
    algorithmFPType * const oldAlphaLocal = buffer + nWS * MemSmoId::oldAlphaBuffID;
    algorithmFPType * const cwLocal       = buffer + nWS * MemSmoId::cwBuffID;
    algorithmFPType * const kernelLocal   = buffer + nWS * MemSmoId::latest;

    {
        DAAL_ITTNOTIFY_SCOPED_TASK(SMOBlockSolver.init);
        SafeStatus safeStat;

        /* Gather data to local buffers */
        const size_t blockSizeWS = services::internal::min<cpu, algorithmFPType>(nWS, 16);
        const size_t nBlocks     = nWS / blockSizeWS;
        daal::threader_for(nBlocks, nBlocks, [&](const size_t iBlock) {
            const size_t startRow = iBlock * blockSizeWS;

            PRAGMA_IVDEP
            PRAGMA_VECTOR_ALWAYS
            for (size_t i = startRow; i < startRow + blockSizeWS; ++i)
            {
                const size_t wsIndex                       = wsIndices[i];
                const algorithmFPType * const kernelWSData = kernelWS[i];
                yLocal[i]                                  = y[wsIndex];
                gradLocal[i]                               = grad[wsIndex];
                oldAlphaLocal[i]                           = alpha[wsIndex];
                alphaLocal[i]                              = alpha[wsIndex];
                cwLocal[i]                                 = cw[wsIndex];
                kdLocal[i]                                 = kernelWSData[wsIndex % nVectors];
                char Ii                                    = free;
                Ii |= HelperTrainSVM<algorithmFPType, cpu>::isUpper(yLocal[i], alphaLocal[i], cwLocal[i]) ? up : free;
                Ii |= HelperTrainSVM<algorithmFPType, cpu>::isLower(yLocal[i], alphaLocal[i], cwLocal[i]) ? low : free;
                Ii |= (yLocal[i] > 0) ? positive : negative;
                I[i] = Ii;

                PRAGMA_IVDEP
                PRAGMA_VECTOR_ALWAYS
                for (size_t j = 0; j < nWS; ++j)
                {
                    kernelLocal[i * nWS + j] = kernelWSData[wsIndices[j] % nVectors];
                }
            }
        });
    }

    algorithmFPType delta    = algorithmFPType(0);
    algorithmFPType localEps = algorithmFPType(0);
    localDiff                = algorithmFPType(0);
    int Bi                   = -1;
    int Bj                   = -1;

    size_t iter = 0;
    for (; iter < innerMaxIterations; ++iter)
    {
        algorithmFPType GMin  = MaxVal<algorithmFPType>::get();
        algorithmFPType GMax  = -MaxVal<algorithmFPType>::get();
        algorithmFPType GMax2 = -MaxVal<algorithmFPType>::get();

        const algorithmFPType zero(0.0);

        const algorithmFPType * KBiBlock = nullptr;

        if (svmType == SvmType::nu_classification || svmType == SvmType::nu_regression)
        {
            int Bi_p = -1;
            int Bi_n = -1;
            int Bj_p = -1;
            int Bj_n = -1;

            algorithmFPType GMax_p  = -MaxVal<algorithmFPType>::get();
            algorithmFPType GMax_n  = -MaxVal<algorithmFPType>::get();
            algorithmFPType GMax2_p = -MaxVal<algorithmFPType>::get();
            algorithmFPType GMax2_n = -MaxVal<algorithmFPType>::get();

            algorithmFPType delta_p = algorithmFPType(0);
            algorithmFPType delta_n = algorithmFPType(0);

            algorithmFPType GMin_p = HelperTrainSVM<algorithmFPType, cpu>::WSSi(nWS, gradLocal, I, Bi_p, CheckClassLabels::positive);
            algorithmFPType GMin_n = HelperTrainSVM<algorithmFPType, cpu>::WSSi(nWS, gradLocal, I, Bi_n, CheckClassLabels::negative);

            const algorithmFPType KBiBi_p = kdLocal[Bi_p];
            const algorithmFPType KBiBi_n = kdLocal[Bi_n];

            const algorithmFPType * const KBiBlock_p = &kernelLocal[Bi_p * nWS];
            const algorithmFPType * const KBiBlock_n = &kernelLocal[Bi_n * nWS];

            HelperTrainSVM<algorithmFPType, cpu>::WSSjLocal(0, nWS, KBiBlock_p, kdLocal, gradLocal, I, GMin_p, KBiBi_p, tau, Bj_p, GMax_p, GMax2_p,
                                                            delta_p, CheckClassLabels::positive);
            HelperTrainSVM<algorithmFPType, cpu>::WSSjLocal(0, nWS, KBiBlock_n, kdLocal, gradLocal, I, GMin_n, KBiBi_n, tau, Bj_n, GMax_n, GMax2_n,
                                                            delta_n, CheckClassLabels::negative);

            if (GMax_p > GMax_n)
            {
                Bi       = Bi_p;
                Bj       = Bj_p;
                delta    = delta_p;
                GMin     = GMin_p;
                GMax     = GMax_p;
                GMax2    = GMax2_p;
                KBiBlock = KBiBlock_p;
            }
            else
            {
                Bi       = Bi_n;
                Bj       = Bj_n;
                delta    = delta_n;
                GMin     = GMin_n;
                GMax     = GMax_n;
                GMax2    = GMax2_n;
                KBiBlock = KBiBlock_n;
            }
        }
        else
        {
            GMin = HelperTrainSVM<algorithmFPType, cpu>::WSSi(nWS, gradLocal, I, Bi);

            const algorithmFPType KBiBi = kdLocal[Bi];
            KBiBlock                    = &kernelLocal[Bi * nWS];

            HelperTrainSVM<algorithmFPType, cpu>::WSSjLocal(0, nWS, KBiBlock, kdLocal, gradLocal, I, GMin, KBiBi, tau, Bj, GMax, GMax2, delta);
        }

        localDiff = GMax2 - GMin;

        if (iter == 0)
        {
            localEps  = services::internal::max<cpu, algorithmFPType>(accuracyThreshold, localDiff * algorithmFPType(1e-1));
            localDiff = services::internal::max<cpu, algorithmFPType>(localDiff, localEps);
        }
        if (localDiff < localEps)
        {
            break;
        }

        const algorithmFPType yBi  = yLocal[Bi];
        const algorithmFPType yBj  = yLocal[Bj];
        const algorithmFPType cwBi = cwLocal[Bi];
        const algorithmFPType cwBj = cwLocal[Bj];

        /* Update coefficients */
        const algorithmFPType alphaBiDelta = (yBi > zero) ? cwBi - alphaLocal[Bi] : alphaLocal[Bi];
        const algorithmFPType alphaBjDelta =
            services::internal::min<cpu, algorithmFPType>((yBj > zero) ? alphaLocal[Bj] : cwBj - alphaLocal[Bj], delta);
        delta = services::internal::min<cpu, algorithmFPType>(alphaBiDelta, alphaBjDelta);

        /* Update alpha */
        alphaLocal[Bi] += delta * yBi;
        alphaLocal[Bj] -= delta * yBj;

        /* Update up/low sets */
        char IBi = free;
        IBi |= HelperTrainSVM<algorithmFPType, cpu>::isUpper(yBi, alphaLocal[Bi], cwBi) ? up : free;
        IBi |= HelperTrainSVM<algorithmFPType, cpu>::isLower(yBi, alphaLocal[Bi], cwBi) ? low : free;
        IBi |= (yBi > 0) ? positive : negative;
        I[Bi] = IBi;

        char IBj = free;
        IBj |= HelperTrainSVM<algorithmFPType, cpu>::isUpper(yBj, alphaLocal[Bj], cwBj) ? up : free;
        IBj |= HelperTrainSVM<algorithmFPType, cpu>::isLower(yBj, alphaLocal[Bj], cwBj) ? low : free;
        IBj |= (yBj > 0) ? positive : negative;
        I[Bj] = IBj;

        const algorithmFPType * const KBjBlock = &kernelLocal[Bj * nWS];

        /* Update gradient */
        PRAGMA_IVDEP
        PRAGMA_VECTOR_ALWAYS
        for (size_t i = 0; i < nWS; i++)
        {
            const algorithmFPType KiBi = KBiBlock[i];
            const algorithmFPType KiBj = KBjBlock[i];
            gradLocal[i] += delta * (KiBi - KiBj);
        }
    }

    /* Compute diff and scatter to alpha vector */
    PRAGMA_IVDEP
    PRAGMA_VECTOR_ALWAYS
    for (size_t i = 0; i < nWS; ++i)
    {
        deltaAlpha[i]       = (alphaLocal[i] - oldAlphaLocal[i]) * yLocal[i];
        alpha[wsIndices[i]] = alphaLocal[i];
    }
    return status;
}

template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::updateGrad(algorithmFPType ** kernelWS, const algorithmFPType * deltaalpha,
                                                                         algorithmFPType * grad, const size_t nVectors, const size_t nTrainVectors,
                                                                         const size_t nWS)
{
    DAAL_ITTNOTIFY_SCOPED_TASK(updateGrad);

    SafeStatus safeStat;
    const size_t blockSizeGrad = 128;
    const size_t nBlocksGrad   = (nTrainVectors / blockSizeGrad) + !!(nTrainVectors % blockSizeGrad);

    DAAL_INT incX(1);
    DAAL_INT incY(1);

    daal::threader_for(nBlocksGrad, nBlocksGrad, [&](const size_t iBlockGrad) {
        const size_t startRowGrad     = iBlockGrad * blockSizeGrad;
        const size_t nRowsInBlockGrad = (iBlockGrad != nBlocksGrad - 1) ? blockSizeGrad : nTrainVectors - iBlockGrad * blockSizeGrad;
        algorithmFPType * const gradi = &grad[startRowGrad];

        for (size_t i = 0; i < nWS; ++i)
        {
            const algorithmFPType * kernelBlockI = kernelWS[i];
            algorithmFPType deltaalphai          = deltaalpha[i];

            if (startRowGrad + nRowsInBlockGrad > nVectors)
            {
                PRAGMA_IVDEP
                PRAGMA_VECTOR_ALWAYS
                for (size_t j = 0; j < nRowsInBlockGrad; ++j)
                {
                    gradi[j] += deltaalphai * kernelBlockI[(startRowGrad + j) % nVectors];
                }
            }
            else
            {
                Blas<algorithmFPType, cpu>::xxaxpy((DAAL_INT *)&nRowsInBlockGrad, &deltaalphai, kernelBlockI + startRowGrad, &incX, gradi, &incY);
            }
        }
    });

    return services::Status();
}

template <typename algorithmFPType, CpuType cpu>
bool SVMTrainImpl<thunder, algorithmFPType, cpu>::checkStopCondition(const algorithmFPType diff, const algorithmFPType diffPrev,
                                                                     const algorithmFPType accuracyThreshold, size_t & sameLocalDiff)
{
    sameLocalDiff = internal::Math<algorithmFPType, cpu>::sFabs(diff - diffPrev) < accuracyThreshold * accuracyThresholdInner ? sameLocalDiff + 1 : 0;
    if (sameLocalDiff > nNoChanges || diff < accuracyThreshold)
    {
        return true;
    }
    return false;
}

template <typename algorithmFPType, CpuType cpu>
services::Status SVMTrainImpl<thunder, algorithmFPType, cpu>::initGrad(const NumericTablePtr & xTable, const kernel_function::KernelIfacePtr & kernel,
                                                                       const size_t nVectors, const size_t nTrainVectors, const size_t cacheSize,
                                                                       algorithmFPType * const y, algorithmFPType * const alpha,
                                                                       algorithmFPType * grad)
{
    services::Status status;

    TArray<algorithmFPType, cpu> deltaAlphaTArray(nVectors);
    DAAL_CHECK_MALLOC(deltaAlphaTArray.get());
    algorithmFPType * const deltaAlpha = deltaAlphaTArray.get();

    const size_t nBlocks = nVectors / maxBlockSize + !!(nVectors % maxBlockSize);

    SafeStatus safeStat;
    daal::threader_for(nBlocks, nBlocks, [&](const size_t iBlock) {
        const size_t startRow     = iBlock * maxBlockSize;
        const size_t nRowsInBlock = (iBlock != nBlocks - 1) ? maxBlockSize : nVectors - iBlock * maxBlockSize;

        PRAGMA_IVDEP
        PRAGMA_VECTOR_ALWAYS
        for (size_t i = 0; i < nRowsInBlock; ++i)
        {
            deltaAlpha[i + startRow] = alpha[i + startRow] * y[i + startRow];
        }
    });

    size_t defaultCacheSize = services::internal::min<cpu, size_t>(nVectors, cacheSize / nVectors / sizeof(algorithmFPType));

    StaticTlsMem<uint32_t, cpu> tlsIndices(maxBlockSize);
    StaticTlsSum<algorithmFPType, cpu> tlsGrad(nVectors);
    daal::static_threader_for(nBlocks, [&](const size_t iBlock, const size_t tid) {
        const size_t startRow     = iBlock * maxBlockSize;
        const size_t nRowsInBlock = (iBlock != nBlocks - 1) ? maxBlockSize : nVectors - iBlock * maxBlockSize;

        uint32_t * const localIndices     = tlsIndices.local(tid);
        algorithmFPType * const localGrad = tlsGrad.local(tid);

        PRAGMA_IVDEP
        PRAGMA_VECTOR_ALWAYS
        for (size_t i = 0; i < nRowsInBlock; ++i)
        {
            localIndices[i] = i + startRow;
        }

        defaultCacheSize = services::internal::max<cpu, size_t>(nRowsInBlock, defaultCacheSize);
        auto cachePtr = SVMCache<thunder, lruCache, algorithmFPType, cpu>::create(defaultCacheSize, nRowsInBlock, nVectors, xTable, kernel, status);
        DAAL_CHECK_STATUS_THR(status);

        algorithmFPType ** kernelSOARes = nullptr;
        {
            DAAL_ITTNOTIFY_SCOPED_TASK(getRowsBlock);

            DAAL_CHECK_STATUS_THR(cachePtr->getRowsBlock(localIndices, nRowsInBlock, kernelSOARes));
        }

        DAAL_CHECK_STATUS_THR(updateGrad(kernelSOARes, deltaAlpha, localGrad, nVectors, nTrainVectors, nRowsInBlock));
    });
    tlsGrad.reduceTo(grad, nVectors);

    return services::Status();
}

} // namespace internal
} // namespace training
} // namespace svm
} // namespace algorithms
} // namespace daal

#endif
