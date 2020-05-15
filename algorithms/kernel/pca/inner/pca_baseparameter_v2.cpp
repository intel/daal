/* file: pca_baseparameter_v2.cpp */
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
//  Implementation of PCA algorithm interface.
//--
*/

#include "algorithms/kernel/pca/inner/pca_types_v2.h"

namespace daal
{
namespace algorithms
{
namespace pca
{
namespace interface2
{
BaseBatchParameter::BaseBatchParameter() : resultsToCompute(none), nComponents(0), isDeterministic(false) {}
} // namespace interface2
} // namespace pca
} // namespace algorithms
} // namespace daal
