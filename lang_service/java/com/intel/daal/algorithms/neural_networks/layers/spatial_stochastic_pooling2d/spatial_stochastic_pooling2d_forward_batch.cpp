/* file: spatial_stochastic_pooling2d_forward_batch.cpp */
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
#include "com_intel_daal_algorithms_neural_networks_layers_spatial_stochastic_pooling2d_SpatialStochasticPooling2dForwardBatch.h"

#include "daal.h"

#include "lang_service/java/com/intel/daal/include/common_helpers.h"

USING_COMMON_NAMESPACES();
using namespace daal::algorithms::neural_networks::layers;

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cInit
 * Signature: (II)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cInit
(JNIEnv *env, jobject thisObj, jint prec, jint method, jlong pyramidHeigh, jlong nDim)
{
    return jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
           newObj(prec, method, pyramidHeigh, nDim);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cInitParameter
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cInitParameter
(JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
           getParameter(prec, method, algAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cGetInput
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cGetInput
(JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
           getInput(prec, method, algAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cGetResult
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cGetResult
(JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
           getResult(prec, method, algAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cSetResult
 * Signature: (JIIJ)V
 */
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cSetResult
(JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method, jlong resAddr)
{
    jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
    setResult<spatial_stochastic_pooling2d::forward::Result>(prec, method, algAddr, resAddr);
}

/*
 * Class:     com_intel_daal_algorithms_neural_networks_layers_stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch
 * Method:    cClone
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_neural_1networks_layers_spatial_1stochastic_1pooling2d_SpatialStochasticPooling2dForwardBatch_cClone
(JNIEnv *env, jobject thisObj, jlong algAddr, jint prec, jint method)
{
    return jniBatch<spatial_stochastic_pooling2d::Method, spatial_stochastic_pooling2d::forward::Batch, spatial_stochastic_pooling2d::defaultDense>::
           getClone(prec, method, algAddr);
}
