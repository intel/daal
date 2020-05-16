/* file: parameter.cpp */
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

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>

#include "daal.h"
#include "com_intel_daal_algorithms_normalization_zscore_Parameter.h"

#include "lang_service/java/com/intel/daal/include/common_defines.i"

#define DefaultDense com_intel_daal_algorithms_normalization_zscore_Method_DefaultDense
#define SumDense     com_intel_daal_algorithms_normalization_zscore_Method_SumDense

using namespace daal;
using namespace daal::algorithms;
using namespace daal::services;

/*
 * Class:     com_intel_daal_algorithms_normalization_zscore_Parameter
 * Method:    cSetMoments
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_normalization_zscore_Parameter_cSetMoments
(JNIEnv *env, jobject thisObj, jlong parAddr, jlong momentsAdr, jint prec, jint method, jint cmode)
{
    using namespace daal::algorithms::normalization::zscore;
    if(method == defaultDense)
    {
        if(cmode == jBatch)
        {
            if(prec == 0) //double
            {
                normalization::zscore::Parameter<double, defaultDense> *parameterAddr = (normalization::zscore::Parameter<double,
                                                                                         defaultDense> *)parAddr;
                parameterAddr->moments = staticPointerCast<daal::algorithms::low_order_moments::BatchImpl>(*(SharedPtr<AlgorithmIface> *)momentsAdr);
            }
            else
            {
                normalization::zscore::Parameter<float, defaultDense> *parameterAddr = (normalization::zscore::Parameter<float,
                                                                                        defaultDense> *)parAddr;
                parameterAddr->moments = staticPointerCast<daal::algorithms::low_order_moments::BatchImpl>(*(SharedPtr<AlgorithmIface> *)momentsAdr);
            }
        }
    }
}

/*
 * Class:     com_intel_daal_algorithms_normalization_zscore_Parameter
 * Method:    cSetResultsToCompute
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_normalization_zscore_Parameter_cSetResultsToCompute
(JNIEnv *, jobject, jlong parAddr, jlong resultsToCompute)
{
    ((normalization::zscore::interface3::BaseParameter *)parAddr)->resultsToCompute = resultsToCompute;
}

/*
 * Class:     com_intel_daal_algorithms_normalization_zscore_Parameter
 * Method:    cGetResultsToCompute
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_normalization_zscore_Parameter_cGetResultsToCompute
(JNIEnv *, jobject, jlong parAddr)
{
    return ((normalization::zscore::interface3::BaseParameter *)parAddr)->resultsToCompute;
}

/*
 * Class:     com_intel_daal_algorithms_normalization_zscore_Parameter
 * Method:    cSetDoScaleFlag
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_normalization_zscore_Parameter_cSetDoScaleFlag
(JNIEnv *, jobject, jlong parAddr, jlong doScale)
{
    ((normalization::zscore::interface3::BaseParameter *)parAddr)->doScale = doScale;
}

/*
 * Class:     com_intel_daal_algorithms_normalization_zscore_Parameter
 * Method:    cGetDoScaleFlag
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_normalization_zscore_Parameter_cGetDoScaleFlag
(JNIEnv *, jobject, jlong parAddr)
{
    return ((normalization::zscore::interface3::BaseParameter *)parAddr)->doScale;
}
