/* file: relu_csr_batch.cpp */
/*******************************************************************************
* Copyright 2014-2019 Intel Corporation
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
!  Content:
!    C++ example of ReLU algorithm.
!
!******************************************************************************/

/**
 * <a name="DAAL-EXAMPLE-CPP-RELU_CSR_BATCH"></a>
 * \example relu_csr_batch.cpp
 */

#define DAAL_HIDE_DEPRECATED

#include "daal.h"
#include "service.h"

using namespace std;
using namespace daal;
using namespace daal::algorithms;
using namespace daal::algorithms::math;

/* Input data set parameters */
string datasetName = "../data/batch/covcormoments_csr.csv";

int main(int argc, char *argv[])
{
    checkArguments(argc, argv, 1, &datasetName);

    /* Read datasetFileName from a file and create a numeric table to store input data */
    CSRNumericTablePtr dataTable(createSparseTable<float>(datasetName));

    /* Create an algorithm */
    relu::Batch<float, relu::fastCSR> relu;

    /* Set an input object for the algorithm */
    relu.input.set(relu::data, dataTable);

    /* Compute ReLU function */
    relu.compute();

    /* Print the results of the algorithm */
    relu::ResultPtr res = relu.getResult();
    printNumericTable(res->get(relu::value), "ReLU result (first 5 rows):", 5);

    return 0;
}
