/* file: transposed_conv2d_layer_backward.h */
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
//++
//  Implementation of the interface for the backward two-dimensional (2D) transposed convolution layer in the
//  batch processing mode
//--
*/

#ifndef __TRANSPOSED_CONV2D_LAYER_BACKWARD_H__
#define __TRANSPOSED_CONV2D_LAYER_BACKWARD_H__

#include "algorithms/algorithm.h"
#include "data_management/data/tensor.h"
#include "services/daal_defines.h"
#include "algorithms/neural_networks/layers/layer.h"
#include "algorithms/neural_networks/layers/transposed_conv2d/transposed_conv2d_layer_types.h"
#include "algorithms/neural_networks/layers/transposed_conv2d/transposed_conv2d_layer_backward_types.h"

namespace daal
{
namespace algorithms
{
namespace neural_networks
{
namespace layers
{
namespace transposed_conv2d
{
namespace backward
{
namespace interface1
{
/**
 * @defgroup transposed_conv2d_backward_batch Batch
 * @ingroup transposed_conv2d_backward
 * @{
 */
/**
 * <a name="DAAL-CLASS-ALGORITHMS__NEURAL_NETWORKS__LAYERS__TRANSPOSED_CONV2D__BACKWARD__BATCHCONTAINER"></a>
 * \brief Provides methods to run implementations of the of the backward 2D transposed convolution layer
 *        This class is associated with the daal::algorithms::neural_networks::layers::transposed_conv2d::backward::Batch class
 *        and supports the method of backward 2D transposed convolution layer computation in the batch processing mode
 *
 * \tparam algorithmFPType  Data type to use in intermediate computations of backward 2D transposed convolution layer, double or float
 * \tparam method           Computation method of the layer, \ref daal::algorithms::neural_networks::layers::transposed_conv2d::Method
 * \tparam cpu              Version of the cpu-specific implementation of the layer, \ref daal::CpuType
 *
 * \DAAL_DEPRECATED
 */
template<typename algorithmFPType, Method method, CpuType cpu>
class BatchContainer : public AnalysisContainerIface<batch>
{
public:
    /**
     * Constructs a container for the backward 2D transposed convolution layer with a specified environment
     * in the batch processing mode
     * \param[in] daalEnv   Environment object
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED BatchContainer(daal::services::Environment::env *daalEnv);
    /**
     * Default destructor
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED ~BatchContainer();
    /**
     * Computes the result of the backward 2D transposed convolution layer in the batch processing mode
     *
     * \return Status of computations
     */
    services::Status compute() DAAL_C11_OVERRIDE;
    services::Status setupCompute() DAAL_C11_OVERRIDE;
    services::Status resetCompute() DAAL_C11_OVERRIDE;
};

/**
 * <a name="DAAL-CLASS-ALGORITHMS__NEURAL_NETWORKS__LAYERS__TRANSPOSED_CONV2D__BACKWARD__BATCH"></a>
 * \brief Provides methods for backward 2D transposed convolution layer computations in the batch processing mode
 * <!-- \n<a href="DAAL-REF-TRANSPOSED_CONV2DBACKWARD-ALGORITHM">Backward 2D transposed convolution layer description and usage models</a> -->
 *
 * \tparam algorithmFPType  Data type to use in intermediate computations of backward 2D transposed convolution layer, double or float
 * \tparam method           Computation method of the layer, \ref Method
 *
 * \par Enumerations
 *      - \ref Method          Computation methods for the backward 2D transposed convolution layer
 *      - \ref LayerDataId     Identifiers of input objects for the backward 2D transposed convolution layer
 *
 * \par References
 *      - Input class
 *      - Result class
 *      - forward::interface1::Batch class
 *
 * \DAAL_DEPRECATED
 */
template<typename algorithmFPType = DAAL_ALGORITHM_FP_TYPE, Method method = defaultDense>
class Batch : public layers::backward::LayerIfaceImpl
{
public:
    typedef layers::backward::LayerIfaceImpl super;

    typedef algorithms::neural_networks::layers::transposed_conv2d::backward::Input     InputType;
    typedef algorithms::neural_networks::layers::transposed_conv2d::Parameter          ParameterType;
    typedef algorithms::neural_networks::layers::transposed_conv2d::backward::Result    ResultType;

    ParameterType &parameter; /*!< \ref interface1::Parameter "Parameters" of the layer */
    InputType input;          /*!< %Input objects of the layer */

    /**
     * Default constructor
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED Batch() : parameter(_defaultParameter)
    {
        initialize();
    };

    /**
     * Constructs a backward 2D transposed convolution layer in the batch processing mode
     * and initializes its parameter with the provided parameter
     * \param[in] parameter Parameter to initialize the parameter of the layer
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED Batch(ParameterType& parameter) : parameter(parameter), _defaultParameter(parameter)
    {
        initialize();
    }

    /**
     * Constructs a backward 2D transposed convolution layer by copying input objects and parameters of another 2D transposed convolution layer
     * \param[in] other A layer to be used as the source to initialize the input objects
     *                  and parameters of this layer
     * \DAAL_DEPRECATED_USE{ cloneImpl() }
     */
    Batch(const Batch<algorithmFPType, method> &other) : super(other),
        _defaultParameter(other.parameter), parameter(_defaultParameter), input(other.input)
    {
        initialize();
    }

    /**
     * Returns computation method of the layer
     * \return Computation method of the layer
     */
    virtual int getMethod() const DAAL_C11_OVERRIDE { return(int) method; }

    /**
     * Returns the structure that contains input objects of 2D transposed convolution layer
     * \return Structure that contains input objects of 2D transposed convolution layer
     */
    virtual InputType *getLayerInput() DAAL_C11_OVERRIDE { return &input; }

    /**
     * Returns the structure that contains parameters of the 2D transposed convolution layer
     * \return Structure that contains parameters of the 2D transposed convolution layer
     */
    virtual ParameterType *getLayerParameter() DAAL_C11_OVERRIDE { return &parameter; };

    /**
     * Returns the structure that contains results of 2D transposed convolution layer
     * \return Structure that contains results of 2D transposed convolution layer
     */
    layers::backward::ResultPtr getLayerResult() DAAL_C11_OVERRIDE
    {
        return getResult();
    }

    /**
     * Returns the structure that contains results of 2D transposed convolution layer
     * \return Structure that contains results of 2D transposed convolution layer
     */
    ResultPtr getResult()
    {
        return _result;
    }

    /**
     * Registers user-allocated memory to store results of 2D transposed convolution layer
     * \param[in] result  Structure to store  results of 2D transposed convolution layer
     *
     * \return Status of computations
     */
    services::Status setResult(const ResultPtr& result)
    {
        DAAL_CHECK(result, services::ErrorNullResult)
        _result = result;
        _res = _result.get();
        return services::Status();
    }

    /**
     * Returns a pointer to the newly allocated 2D transposed convolution layer
     * with a copy of input objects and parameters of this 2D transposed convolution layer
     * \return Pointer to the newly allocated layer
     */
    services::SharedPtr<Batch<algorithmFPType, method> > clone() const
    {
        return services::SharedPtr<Batch<algorithmFPType, method> >(cloneImpl());
    }

    virtual services::Status allocateResult() DAAL_C11_OVERRIDE
    {
        services::Status s = this->_result->template allocate<algorithmFPType>(&(this->input), &parameter, (int) method);
        this->_res = this->_result.get();
        return s;
    }

protected:
    virtual Batch<algorithmFPType, method> *cloneImpl() const DAAL_C11_OVERRIDE
    {
        return new Batch<algorithmFPType, method>(*this);
    }

    void initialize()
    {
        Analysis<batch>::_ac = new __DAAL_ALGORITHM_CONTAINER(batch, BatchContainer, algorithmFPType, method)(&_env);
        _in = &input;
        _par = &parameter;
        _result.reset(new ResultType());
    }

private:
    ResultPtr _result;
    ParameterType _defaultParameter;
};
/** @} */
} // namespace interface1
using interface1::BatchContainer;
using interface1::Batch;
} // namespace backward
} // namespace transposed_conv2d
} // namespace layers
} // namespace neural_networks
} // namespace algorithms
} // namespace daal
#endif
