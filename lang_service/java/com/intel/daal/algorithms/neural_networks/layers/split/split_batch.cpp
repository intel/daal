/* file: split_batch.cpp */
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

#include <jni.h>
#include "com_intel_daal_algorithms_neural_networks_layers_split_SplitBatch.h"

#include "daal.h"

#include "common_helpers.h"

USING_COMMON_NAMESPACES();
using namespace daal::algorithms::neural_networks::layers;

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_split_SplitBatch
 * Method:    cInit
 * Signature: (II)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_split_SplitBatch_cInit
  (JNIEnv *env, jobject thisObj, jint prec, jint method)
{
    return jniBatchLayer<split::Method, split::Batch, split::defaultDense>::
        newObj(prec, method);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_split_SplitBatch
 * Method:    cInitParameter
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_split_SplitBatch_cInitParameter
  (JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatchLayer<split::Method, split::Batch, split::defaultDense>::
        getParameter(prec, method, algAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_split_SplitBatch
 * Method:    cGetForwardLayer
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_split_SplitBatch_cGetForwardLayer
  (JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatchLayer<split::Method, split::Batch, split::defaultDense>::
        getForwardLayer(prec, method, algAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_split_SplitBatch
 * Method:    cGetBackwardLayer
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_split_SplitBatch_cGetBackwardLayer
  (JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatchLayer<split::Method, split::Batch, split::defaultDense>::
        getBackwardLayer(prec, method, algAddr);
}
