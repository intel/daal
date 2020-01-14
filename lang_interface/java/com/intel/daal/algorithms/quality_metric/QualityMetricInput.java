/* file: QualityMetricInput.java */
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
 * @ingroup quality_metric
 * @{
 */
package com.intel.daal.algorithms.quality_metric;

import com.intel.daal.utils.*;
import com.intel.daal.services.DaalContext;

/**
 * <a name="DAAL-CLASS-ALGORITHMS__QUALITY_METRIC__QUALITYMETRICINPUT"></a>
 * @brief  Base class for input objects of quality metrics
 */
public class QualityMetricInput extends com.intel.daal.algorithms.Input {
    /** @private */
    static {
        LibUtils.loadLibrary();
    }

    /**
     * Constructs the input of the quality metric algorithm
     * @param context   Context to manage the input of the quality metric algorithm
     */
    public QualityMetricInput(DaalContext context) {
        super(context);
    }

    public QualityMetricInput(DaalContext context, long cObject) {
        super(context, cObject);
    }
}
/** @} */
