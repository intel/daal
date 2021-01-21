/* file: CovDenseBatch.java */
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

/*
 //  Content:
 //     Java example of dense variance-covariance matrix computation in the batch
 //     processing mode
 ////////////////////////////////////////////////////////////////////////////////
 */

/**
 * <a name="DAAL-EXAMPLE-JAVA-COVARIANCEDENSEBATCH">
 * @example CovDenseBatch.java
 */

package com.intel.daal.examples.covariance;

import com.intel.daal.algorithms.covariance.*;
import com.intel.daal.data_management.data.HomogenNumericTable;
import com.intel.daal.data_management.data.NumericTable;
import com.intel.daal.data_management.data_source.DataSource;
import com.intel.daal.data_management.data_source.FileDataSource;
import com.intel.daal.examples.utils.Service;
import com.intel.daal.services.DaalContext;

class CovDenseBatch {
    /* Input data set parameters */
    private static final String datasetFileName = "../data/batch/covcormoments_dense.csv";

    private static DaalContext context = new DaalContext();

    public static void main(String[] args) throws java.io.FileNotFoundException, java.io.IOException {

        /* Retrieve the input data */
        FileDataSource dataSource = new FileDataSource(context, datasetFileName,
                DataSource.DictionaryCreationFlag.DoDictionaryFromContext,
                DataSource.NumericTableAllocationFlag.DoAllocateNumericTable);
        dataSource.loadDataBlock();

        /* Create an algorithm to compute a variance-covariance matrix using the single-pass method */
        Batch alg = new Batch(context, Float.class, Method.defaultDense);
        NumericTable input = dataSource.getNumericTable();
        alg.input.set(InputId.data, input);

        /* Compute the variance-covariance matrix */
        Result res = alg.compute();

        HomogenNumericTable covariance = (HomogenNumericTable) res.get(ResultId.covariance);
        HomogenNumericTable mean = (HomogenNumericTable) res.get(ResultId.mean);

        Service.printNumericTable("Covariance matrix:", covariance);
        Service.printNumericTable("Mean vector:", mean);

        context.dispose();
    }
}
