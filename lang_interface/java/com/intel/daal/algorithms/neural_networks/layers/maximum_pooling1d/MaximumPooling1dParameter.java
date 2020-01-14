/* file: MaximumPooling1dParameter.java */
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

/**
 * @ingroup maximum_pooling1d
 * @{
 */
package com.intel.daal.algorithms.neural_networks.layers.maximum_pooling1d;

import com.intel.daal.services.DaalContext;

/**
 * <a name="DAAL-CLASS-ALGORITHMS__NEURAL_NETWORKS__LAYERS__MAXIMUM_POOLING1D__MAXIMUMPOOLING1DPARAMETER"></a>
 * \brief Class that specifies parameters of the one-dimensional maximum pooling layer
 */
public class MaximumPooling1dParameter extends com.intel.daal.algorithms.neural_networks.layers.pooling1d.Pooling1dParameter {
    /** @private */
    public MaximumPooling1dParameter(DaalContext context, long cObject) {
        super(context, cObject);
    }
}
/** @} */
