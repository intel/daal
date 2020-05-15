/* file: model_builder.cpp */
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
#include "com_intel_daal_algorithms_multi_class_classifier_ModelBuilder.h"
#include "lang_service/java/com/intel/daal/common_helpers_functions.h"

using namespace daal;
using namespace daal::algorithms::multi_class_classifier;
using namespace daal::data_management;
using namespace daal::services;

/*
* Class:     com_intel_daal_algorithms_multi_class_classifier_ModelBuilder
* Method:    cInit
* Signature: (JIII)J
*/
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_multi_1class_1classifier_ModelBuilder_cInit
(JNIEnv *, jobject, jint method, jlong nFeatures, jlong nClasses)
{
    return (jlong)(new SharedPtr<ModelBuilder<>>(new ModelBuilder<>(nFeatures, nClasses)));
}

/*
 * Class:     com_intel_daal_algorithms_multi_class_classifier_ModelBuilder
 * Method:    cGetModel
 * Signature:(JII)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_multi_1class_1classifier_ModelBuilder_cGetModel
(JNIEnv *env, jobject thisObj, jlong algAddr)
{
    ModelPtr *model = new ModelPtr;
    services::SharedPtr<ModelBuilder<>> *ptr = new services::SharedPtr<ModelBuilder<>>();
    *ptr = staticPointerCast<ModelBuilder<>>(*(SharedPtr<ModelBuilder<>> *)algAddr);
    *model = staticPointerCast<Model>((*ptr)->getModel());
    return (jlong)model;
}

/*
 * Class:     com_intel_daal_algorithms_multi_class_classifier_ModelBuilder
 * Method:    cSetTwoClassClassifierModel
 * Signature:(JII)J
 */
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_multi_1class_1classifier_ModelBuilder_cSetTwoClassClassifierModel
(JNIEnv *env, jobject thisObj, jlong algAddr, jlong negativeClassIdx, jlong positiveClassIdx, jlong modelAddr)
{
    daal::algorithms::classifier::ModelPtr *twoClassModel = (daal::algorithms::classifier::ModelPtr *)modelAddr;
    services::SharedPtr<ModelBuilder<>> *ptr = new services::SharedPtr<ModelBuilder<>>();
    *ptr = staticPointerCast<ModelBuilder<>>(*(SharedPtr<ModelBuilder<>> *)algAddr);
    (*ptr)->setTwoClassClassifierModel(negativeClassIdx, positiveClassIdx, staticPointerCast<daal::algorithms::classifier::Model>(*twoClassModel));
    DAAL_CHECK_THROW((*ptr)->getStatus());
}
