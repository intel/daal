/* file: log_reg_binary_dense_batch.cpp */
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
!    C++ example of logistic regression 2 classes in the batch processing mode
!    with DPC++ interfaces.
!
!    The program trains the logistic regression model on a training
!    datasetFileName and computes classification for the test data.
!******************************************************************************/

/**
 * <a name="DAAL-EXAMPLE-CPP-LOG_REG_BINARY_DENSE_BATCH"></a>
 * \example log_reg_binary_dense_batch.cpp
 */

#include "daal_sycl.h"
#include "service.h"
#include "service_sycl.h"

using namespace std;
using namespace daal;
using namespace daal::algorithms;
using namespace daal::algorithms::logistic_regression;

/* Input data set parameters */
const string trainDatasetFileName = "../data/batch/binary_cls_train.csv";
const string testDatasetFileName  = "../data/batch/binary_cls_test.csv";
const size_t nFeatures            = 20; /* Number of features in training and testing data sets */

/* Logistic regression training parameters */
const size_t nClasses = 2; /* Number of classes */

training::ResultPtr trainModel(const NumericTablePtr & trainData, const NumericTablePtr & trainDependentVariable);
void testModel(const training::ResultPtr & res, const NumericTablePtr & testData, const NumericTablePtr & testGroundTruth);
void loadData(const std::string & fileName, NumericTablePtr & pData, NumericTablePtr & pDependentVar);

int main(int argc, char * argv[])
{
    checkArguments(argc, argv, 2, &trainDatasetFileName, &testDatasetFileName);

    for (const auto & deviceSelector : getListOfDevices())
    {
        const auto & nameDevice = deviceSelector.first;
        const auto & device     = deviceSelector.second;
        cl::sycl::queue queue(device);
        std::cout << "Running on " << nameDevice << "\n\n";

        services::SyclExecutionContext ctx(queue);
        services::Environment::getInstance()->setDefaultExecutionContext(ctx);

        /* Create Numeric Tables for training data and dependent variables */
        NumericTablePtr trainData, trainDependentVariable;
        loadData(trainDatasetFileName, trainData, trainDependentVariable);

        training::ResultPtr trainingResult = trainModel(trainData, trainDependentVariable);

        /* Create Numeric Tables for testing data and ground truth values */
        NumericTablePtr testData, testGroundTruth;
        loadData(testDatasetFileName, testData, testGroundTruth);

        testModel(trainingResult, testData, testGroundTruth);
    }
    return 0;
}

training::ResultPtr trainModel(const NumericTablePtr & trainData, const NumericTablePtr & trainDependentVariable)
{
    /* Create an algorithm object to train the logistic regression model */
    training::Batch<> algorithm(nClasses);

    /* Pass a training data set and dependent values to the algorithm */
    algorithm.input.set(classifier::training::data, trainData);
    algorithm.input.set(classifier::training::labels, trainDependentVariable);
    algorithm.parameter().penaltyL2     = 0.1f;
    algorithm.parameter().interceptFlag = true;

    /* Set optimization solver SGD mini-batch */
    using SolverType = optimization_solver::sgd::Batch<float, optimization_solver::sgd::miniBatch>;
    services::SharedPtr<SolverType> solver(new SolverType());

    solver->parameter.nIterations       = 200;
    solver->parameter.batchSize         = 1;
    solver->parameter.accuracyThreshold = 1e-6;

    algorithm.parameter().optimizationSolver = solver;

    /* Build the logistic regression model */
    algorithm.compute();

    /* Retrieve the algorithm results */
    training::ResultPtr trainingResult     = algorithm.getResult();
    logistic_regression::ModelPtr modelptr = trainingResult->get(classifier::training::model);
    if (modelptr.get())
    {
        printNumericTable(modelptr->getBeta(), "Logistic Regression coefficients:");
    }
    else
    {
        std::cout << "Null model pointer" << std::endl;
    }
    return trainingResult;
}

void testModel(const training::ResultPtr & trainingResult, const NumericTablePtr & testData, const NumericTablePtr & testGroundTruth)
{
    /* Create an algorithm object to predict values of logistic regression */
    prediction::Batch<> algorithm(nClasses);

    /* Pass a testing data set and the trained model to the algorithm */
    algorithm.input.set(classifier::prediction::data, testData);
    algorithm.input.set(classifier::prediction::model, trainingResult->get(classifier::training::model));

    /* Predict values of logistic regression */
    algorithm.compute();

    /* Retrieve the algorithm results */
    classifier::prediction::ResultPtr predictionResult = algorithm.getResult();
    printNumericTable(predictionResult->get(classifier::prediction::prediction), "Logistic regression prediction results (first 10 rows):", 10);
    printNumericTable(testGroundTruth, "Ground truth (first 10 rows):", 10);
}

void loadData(const std::string & fileName, NumericTablePtr & pData, NumericTablePtr & pDependentVar)
{
    /* Initialize FileDataSource<CSVFeatureManager> to retrieve the input data from a .csv file */
    FileDataSource<CSVFeatureManager> trainDataSource(fileName, DataSource::notAllocateNumericTable, DataSource::doDictionaryFromContext);

    /* Create Numeric Tables for training data and dependent variables */
    pData         = SyclHomogenNumericTable<>::create(nFeatures, 0, NumericTable::notAllocate);
    pDependentVar = SyclHomogenNumericTable<>::create(1, 0, NumericTable::notAllocate);
    NumericTablePtr mergedData(new MergedNumericTable(pData, pDependentVar));

    /* Retrieve the data from input file */
    trainDataSource.loadDataBlock(mergedData.get());
}
