/* file: adaboost_quality_metric_set_batch.h */
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
//  Interface for the AdaBoost quality metric set.
//--
*/

#ifndef __ADABOOST_QUALITY_METRIC_SET_BATCH_H__
#define __ADABOOST_QUALITY_METRIC_SET_BATCH_H__

#include "algorithms/algorithm_quality_metric_set_batch.h"
#include "algorithms/classifier/binary_confusion_matrix_batch.h"
#include "algorithms/boosting/adaboost_quality_metric_set_types.h"

namespace daal
{
namespace algorithms
{
namespace adaboost
{
/**
 * \brief Contains classes for checking the quality of the model trained with the AdaBoost algorithm
 */
namespace quality_metric_set
{

namespace interface1
{
/**
 * @defgroup adaboost_quality_metric_set_batch Batch
 * @ingroup adaboost_quality_metric_set
 * @{
 */
/**
 * <a name="DAAL-CLASS-ALGORITHMS__ADABOOST__QUALITY_METRIC_SET__BATCH"></a>
 * \brief Class that represents a set of quality metrics to check the model trained with the AdaBoost algorithm  \DAAL_DEPRECATED
 *
 * \par Enumerations
 *      - \ref QualityMetricId  Quality metrics provided by the library
 *
 * \par References
 *      - \ref algorithms::quality_metric_set::interface1::InputAlgorithmsCollection "algorithms::quality_metric_set::InputAlgorithmsCollection" class
 *      - InputDataCollection class
 *      - ResultCollection class
 */
class Batch : public algorithms::quality_metric_set::Batch
{
public:
    /**
     * Constructs a quality metric set for the model trained with the AdaBoost algorithm
     * \param[in] useDefaultMetrics     Flag. If true, a quality metric set is initialized with the quality metrics provided by the library
     */
    DAAL_DEPRECATED Batch(bool useDefaultMetrics = true) : algorithms::quality_metric_set::Batch(useDefaultMetrics)
    {
        _inputData = interface1::InputDataCollectionPtr(new interface1::InputDataCollection());
        if (_useDefaultMetrics)
        {
            initializeQualityMetrics();
        }
        _resultCollection = interface1::ResultCollectionPtr(new interface1::ResultCollection());
    }

    DAAL_DEPRECATED_VIRTUAL virtual ~Batch() {}

    /**
     * Returns the structure that contains a computed quality metric set
     * \return Structure that contains a computed quality metric set
     */
    DAAL_DEPRECATED interface1::ResultCollectionPtr getResultCollection()
    {
        return services::staticPointerCast<interface1::ResultCollection,
                                           algorithms::quality_metric_set::ResultCollection>(_resultCollection);
    }

    /**
     * Returns the collection of input objects for the quality metrics algorithm
     * \return Collection of input objects for the quality metrics algorithm
     */
    DAAL_DEPRECATED interface1::InputDataCollectionPtr getInputDataCollection()
    {
        return services::staticPointerCast<interface1::InputDataCollection,
                                           algorithms::quality_metric_set::InputDataCollection>(_inputData);
    }

protected:
    virtual void initializeQualityMetrics()
    {
        inputAlgorithms[confusionMatrix] = services::SharedPtr<classifier::quality_metric::binary_confusion_matrix::Batch<> >(
                new classifier::quality_metric::binary_confusion_matrix::Batch<>());
        _inputData->add(confusionMatrix, algorithms::InputPtr(
                new classifier::quality_metric::binary_confusion_matrix::Input));
    }
};
/** @} */
} // namespace interface1

namespace interface2
{
/**
 * @defgroup adaboost_quality_metric_set_batch Batch
 * @ingroup adaboost_quality_metric_set
 * @{
 */
/**
 * <a name="DAAL-CLASS-ALGORITHMS__ADABOOST__QUALITY_METRIC_SET__BATCH"></a>
 * \brief Class that represents a set of quality metrics to check the model trained with the AdaBoost algorithm
 *
 * \par Enumerations
 *      - \ref QualityMetricId  Quality metrics provided by the library
 *
 * \par References
 *      - \ref algorithms::quality_metric_set::interface1::InputAlgorithmsCollection "algorithms::quality_metric_set::InputAlgorithmsCollection" class
 *      - InputDataCollection class
 *      - ResultCollection class
 */
class Batch : public algorithms::quality_metric_set::Batch
{
public:
    Parameter parameter;    /*!< Parameters of the algorithm */
    /**
     * Constructs a quality metric set for the model trained with the AdaBoost algorithm
     * \param[in] nClasses Number of classes
     * \param[in] useDefaultMetrics Flag. If true, a quality metric set is initialized with the quality metrics provided by the library
     */
    Batch(size_t nClasses = 2, bool useDefaultMetrics = true) :
        algorithms::quality_metric_set::Batch(useDefaultMetrics),
        parameter(nClasses)
    {
        _inputData = InputDataCollectionPtr(new InputDataCollection());
        if (_useDefaultMetrics)
        {
            initializeQualityMetrics();
        }
        _resultCollection = ResultCollectionPtr(new ResultCollection());
    }

    virtual ~Batch() {}

    /**
     * Returns the structure that contains a computed quality metric set
     * \return Structure that contains a computed quality metric set
     */
    ResultCollectionPtr getResultCollection()
    {
        return services::staticPointerCast<ResultCollection,
                                           algorithms::quality_metric_set::ResultCollection>(_resultCollection);
    }

    /**
     * Returns the collection of input objects for the quality metrics algorithm
     * \return Collection of input objects for the quality metrics algorithm
     */
    InputDataCollectionPtr getInputDataCollection()
    {
        return services::staticPointerCast<InputDataCollection,
                                           algorithms::quality_metric_set::InputDataCollection>(_inputData);
    }

protected:
    virtual void initializeQualityMetrics()
    {
        inputAlgorithms[confusionMatrix] = services::SharedPtr<classifier::quality_metric::multiclass_confusion_matrix::Batch<> >(
                                               new classifier::quality_metric::multiclass_confusion_matrix::Batch<>(parameter.nClasses));
        _inputData->add(confusionMatrix, algorithms::InputPtr(
                          new classifier::quality_metric::multiclass_confusion_matrix::Input));
    }
};
/** @} */
} // namespace interface2
using interface2::Batch;

}
}
}
}
#endif
