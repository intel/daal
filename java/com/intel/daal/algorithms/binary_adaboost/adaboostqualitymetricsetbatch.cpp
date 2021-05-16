/* file: adaboostqualitymetricsetbatch.cpp */
/*******************************************************************************
* Copyright 2014-2021 Intel Corporation
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
#include "com_intel_daal_algorithms_binary_adaboost_quality_metric_set_QualityMetricSetBatch.h"

using namespace daal::algorithms::adaboost::quality_metric_set;

/*
 * Class:     com_intel_daal_algorithms_binary_1adaboost_quality_metric_set_QualityMetricSetBatch
 * Method:    cInit
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_binary_1adaboost_quality_1metric_1set_QualityMetricSetBatch_cInit(JNIEnv *, jobject)
{
    jlong addr = 0;
    addr       = (jlong)(new Batch());
    return addr;
}
