/* file: relu_types.h */
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
//  Implementation of the rectified linear function interface.
//--
*/

#ifndef __RELU_TYPES_H__
#define __RELU_TYPES_H__

#include "algorithms/algorithm.h"
#include "data_management/data/numeric_table.h"
#include "data_management/data/csr_numeric_table.h"
#include "data_management/data/homogen_numeric_table.h"
#include "services/daal_defines.h"

namespace daal
{
namespace algorithms
{
/**
 * \brief Contains classes for computing math functions
 */
namespace math
{
/**
 * @defgroup relu Rectifier Linear Unit (ReLU)
 * \copydoc daal::algorithms::math::relu
 * @ingroup math
 * @{
 */
/**
 * \brief Contains classes for computing the rectified linear function
 */
namespace relu
{
/**
 * <a name="DAAL-ENUM-ALGORITHMS__MATH__RELU__METHOD"></a>
 * Computation methods for computing the rectified linear function
 */
enum Method
{
    defaultDense = 0,       /*!< Default: performance-oriented method. */
    fastCSR = 1             /*!< Fast: performance-oriented method. Works with Compressed Sparse Rows(CSR) numeric tables */
};

/**
 * <a name="DAAL-ENUM-ALGORITHMS__MATH__RELU__INPUTID"></a>
 * Available identifiers of input objects for the rectified linear function
 */
enum InputId
{
    data,                /*!< %Input data table */
    lastInputId = data
};

/**
 * <a name="DAAL-ENUM-ALGORITHMS__MATH__RELU__RESULTID"></a>
 * Available identifiers of the result of the rectified linear function
 */
enum ResultId
{
    value,      /*!< Table to store the result */
    lastResultId = value
};

/**
 * \brief Contains version 1.0 of the Intel(R) Data Analytics Acceleration Library (Intel(R) DAAL) interface.
 */
namespace interface1
{
/**
 * <a name="DAAL-CLASS-ALGORITHMS__MATH__RELU__INPUT"></a>
 * \brief %Input objects for the rectified linear function
 * \DAAL_DEPRECATED
 */
class DAAL_EXPORT Input : public daal::algorithms::Input
{
public:
    /**
     * Default constructor
     * \DAAL_DEPRECATED_USE{ Batch() }
     */
    Input();

    /**
     * Copy constructor
     * \DAAL_DEPRECATED_USE{ Batch(const Batch<algorithmFPType, method> &other) }
     */
    Input(const Input& other) : daal::algorithms::Input(other){}

    /*
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED_VIRTUAL virtual ~Input() {}

    /**
     * Returns an input object for the rectified linear function
     * \param[in] id    Identifier of the input object
     * \return          %Input object that corresponds to the given identifier
     */
    data_management::NumericTablePtr get(InputId id) const;

    /**
     * Sets an input object for the rectified linear function
     * \param[in] id    Identifier of the input object
     * \param[in] ptr   Pointer to the object
     */
    void set(InputId id, const data_management::NumericTablePtr &ptr);

    /**
     * Checks an input object for the rectified linear function
     * \param[in] par     Function parameter
     * \param[in] method  Computation method
     *
     * \return Status of computation
     */
    services::Status check(const daal::algorithms::Parameter *par, int method) const DAAL_C11_OVERRIDE;
};

/**
 * <a name="DAAL-CLASS-ALGORITHMS__MATH__RELU__RESULT"></a>
 * \brief Results obtained with the compute() method of the rectified linear function in the batch processing mode
 * \DAAL_DEPRECATED
 */
class DAAL_EXPORT Result : public daal::algorithms::Result
{
public:
    DECLARE_SERIALIZABLE_CAST(Result);
    /**
     * Default constructor
     * \DAAL_DEPRECATED_USE{ BATCH::allocateResult() }
     */
    Result();

    /*
     * \DAAL_DEPRECATED
     */
    DAAL_DEPRECATED_VIRTUAL virtual ~Result() {};

    /**
     * Allocates memory to store the result of the rectified linear function
     * \param[in] input  %Input object for the rectified linear function
     * \param[in] par    %Parameter of the rectified linear function
     * \param[in] method Computation method of the rectified linear function
     *
     * \return Status of computation
     */
    template <typename algorithmFPType>
    DAAL_EXPORT services::Status allocate(const daal::algorithms::Input *input, const daal::algorithms::Parameter *par, const int method);

    /**
     * Returns the result of the rectified linear function
     * \param[in] id   Identifier of the result
     * \return         Result that corresponds to the given identifier
     */
    data_management::NumericTablePtr get(ResultId id) const;

    /**
     * Sets the result of the rectified linear function
     * \param[in] id    Identifier of the result
     * \param[in] ptr   Result
     */
    void set(ResultId id, const data_management::NumericTablePtr &ptr);

    /**
     * Checks the result of the rectified linear function
     * \param[in] in   %Input of the rectified linear function
     * \param[in] par     %Parameter of the rectified linear function
     * \param[in] method  Computation method of the rectified linear function
     *
     * \return Status of computation
     */
    services::Status check(const daal::algorithms::Input *in, const daal::algorithms::Parameter *par, int method) const DAAL_C11_OVERRIDE;

protected:
    /** \private */
    template<typename Archive, bool onDeserialize>
    services::Status serialImpl(Archive *arch)
    {
        return daal::algorithms::Result::serialImpl<Archive, onDeserialize>(arch);
    }
};
typedef services::SharedPtr<Result> ResultPtr;

/** @} */
} // namespace interface1
using interface1::Input;
using interface1::Result;
using interface1::ResultPtr;

} // namespace relu
} // namespace math
} // namespace algorithm
} // namespace daal
#endif
