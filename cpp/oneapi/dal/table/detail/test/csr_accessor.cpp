/*******************************************************************************
* Copyright 2021 Intel Corporation
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

#include "oneapi/dal/table/detail/csr.hpp"
#include "oneapi/dal/table/detail/csr_accessor.hpp"
#include "oneapi/dal/backend/interop/table_conversion.hpp"
#include "oneapi/dal/test/engine/common.hpp"

namespace oneapi::dal::detail {

TEST("can read csr table via csr accessor") {
    using oneapi::dal::detail::empty_delete;

    double data[] = { 1.0, 2.0, 3.0, 4.0, 1.0, 11.0, 8.0 };
    std::int64_t column_indices[] = { 1, 2, 4, 3, 2, 4, 2 };
    std::int64_t row_indices[] = { 1, 4, 5, 7, 8 };

    const std::int64_t row_count{ 4 };
    const std::int64_t column_count{ 4 };

    detail::csr_table t{ data,
                         column_indices,
                         row_indices,
                         column_count,
                         row_count,
                         empty_delete<const double>(),
                         empty_delete<const std::int64_t>(),
                         empty_delete<const std::int64_t>() };
    const auto csr_block = detail::csr_accessor<const double>(t).pull({ 0, -1 });

    REQUIRE(data == csr_block.data.get_data());
    REQUIRE(column_indices == csr_block.column_indices.get_data());
    REQUIRE(row_indices == csr_block.row_indices.get_data());

    for (std::int64_t i = 0; i < csr_block.data.get_count(); i++) {
        REQUIRE(csr_block.data[i] == data[i]);
        REQUIRE(csr_block.column_indices[i] == column_indices[i]);
    }
    for (std::int64_t i = 0; i < csr_block.row_indices.get_count(); i++) {
        REQUIRE(csr_block.row_indices[i] == row_indices[i]);
    }
}

TEST("can read csr table via csr accessor create smaller block") {
    using oneapi::dal::detail::empty_delete;

    double data[] = { 1.0, 2.0, 3.0, 4.0, 1.0, 11.0, 8.0 };
    std::int64_t column_indices[] = { 1, 2, 4, 3, 2, 4, 2 };
    std::int64_t row_indices[] = { 1, 4, 5, 7, 8 };

    const std::int64_t row_count{ 4 };
    const std::int64_t column_count{ 4 };

    detail::csr_table t{ data,
                         column_indices,
                         row_indices,
                         column_count,
                         row_count,
                         empty_delete<const double>(),
                         empty_delete<const std::int64_t>(),
                         empty_delete<const std::int64_t>() };
    const auto csr_block = detail::csr_accessor<const double>(t).pull({ 1, 3 });

    REQUIRE(data + 3 == csr_block.data.get_data());
    REQUIRE(column_indices + 3 == csr_block.column_indices.get_data());
    REQUIRE(row_indices + 1 != csr_block.row_indices.get_data());

    for (std::int64_t i = 0; i < csr_block.data.get_count(); i++) {
        REQUIRE(csr_block.data[i] == data[3 + i]);
        REQUIRE(csr_block.column_indices[i] == column_indices[3 + i]);
    }
    for (std::int64_t i = 0; i < csr_block.row_indices.get_count(); i++) {
        REQUIRE(csr_block.row_indices[i] == row_indices[1 + i] - 3);
    }
}

TEST("can read csr table via csr accessor with conversion") {
    using oneapi::dal::detail::empty_delete;

    float data[] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 11.0f, 8.0f };
    std::int64_t column_indices[] = { 1, 2, 4, 3, 2, 4, 2 };
    std::int64_t row_indices[] = { 1, 4, 5, 7, 8 };

    const std::int64_t row_count{ 4 };
    const std::int64_t column_count{ 4 };

    detail::csr_table t{ data,
                         column_indices,
                         row_indices,
                         column_count,
                         row_count,
                         empty_delete<const float>(),
                         empty_delete<const std::int64_t>(),
                         empty_delete<const std::int64_t>() };
    const auto csr_block = detail::csr_accessor<const double>(t).pull({ 0, -1 });

    REQUIRE((void*)data != (void*)csr_block.data.get_data());
    REQUIRE(column_indices == csr_block.column_indices.get_data());
    REQUIRE(row_indices == csr_block.row_indices.get_data());

    for (std::int64_t i = 0; i < csr_block.data.get_count(); i++) {
        REQUIRE(csr_block.data[i] == Approx(static_cast<double>(data[i])));
        REQUIRE(csr_block.column_indices[i] == column_indices[i]);
    }
    for (std::int64_t i = 0; i < csr_block.row_indices.get_count(); i++) {
        REQUIRE(csr_block.row_indices[i] == row_indices[i]);
    }
}

TEST("conver csr table from dal to daal") {
    using oneapi::dal::detail::empty_delete;

    double data[] = { 1.0, 2.0, 3.0, 4.0, 1.0, 11.0, 8.0 };
    std::int64_t column_indices[] = { 1, 2, 4, 3, 2, 4, 2 };
    std::int64_t row_indices[] = { 1, 4, 5, 7, 8 };

    const std::int64_t row_count{ 4 };
    const std::int64_t column_count{ 4 };

    double* daal_data{ nullptr };
    std::size_t* daal_column_indices{ nullptr };
    std::size_t* daal_row_indices{ nullptr };

    detail::csr_table t{ data,
                         column_indices,
                         row_indices,
                         column_count,
                         row_count,
                         empty_delete<const double>(),
                         empty_delete<const std::int64_t>(),
                         empty_delete<const std::int64_t>() };
    const auto dal_csr_block = detail::csr_accessor<const double>(t).pull({ 0, -1 });

    const auto daal_csr_table = backend::interop::convert_to_daal_table<double>(t);

    daal_csr_table->getArrays(&daal_data, &daal_column_indices, &daal_row_indices);

    REQUIRE(daal_data == dal_csr_block.data.get_data());
    REQUIRE(daal_column_indices ==
            reinterpret_cast<const std::size_t*>(dal_csr_block.column_indices.get_data()));
    REQUIRE(daal_row_indices ==
            reinterpret_cast<const std::size_t*>(dal_csr_block.row_indices.get_data()));

    for (std::int64_t i = 0; i < dal_csr_block.data.get_count(); i++) {
        REQUIRE(daal_data[i] == dal_csr_block.data[i]);
        REQUIRE(daal_column_indices[i] ==
                static_cast<std::size_t>(dal_csr_block.column_indices[i]));
    }
    for (std::int64_t i = 0; i < dal_csr_block.row_indices.get_count(); i++) {
        REQUIRE(daal_row_indices[i] == static_cast<std::size_t>(dal_csr_block.row_indices[i]));
    }
}

TEST("conver csr table from daal to dal") {
    double data[] = { 1.0, 2.0, 3.0, 4.0, 1.0, 11.0, 8.0 };
    std::size_t column_indices[] = { 1, 2, 4, 3, 2, 4, 2 };
    std::size_t row_indices[] = { 1, 4, 5, 7, 8 };

    const std::int64_t row_count{ 4 };
    const std::int64_t column_count{ 4 };

    const auto daal_csr_table = daal::data_management::CSRNumericTable::create(data,
                                                                               column_indices,
                                                                               row_indices,
                                                                               column_count,
                                                                               row_count);

    const auto dal_csr_table =
        backend::interop::convert_from_daal_csr_table<double>(daal_csr_table);
    const auto csr_block = detail::csr_accessor<const double>(dal_csr_table).pull({ 0, -1 });

    REQUIRE(daal_csr_table->getNumberOfRows() ==
            static_cast<std::size_t>(dal_csr_table.get_row_count()));
    REQUIRE(daal_csr_table->getNumberOfColumns() ==
            static_cast<std::size_t>(dal_csr_table.get_column_count()));

    REQUIRE(data == csr_block.data.get_data());
    REQUIRE(column_indices ==
            reinterpret_cast<const std::size_t*>(csr_block.column_indices.get_data()));
    REQUIRE(row_indices == reinterpret_cast<const std::size_t*>(csr_block.row_indices.get_data()));

    for (std::int64_t i = 0; i < csr_block.data.get_count(); i++) {
        REQUIRE(data[i] == csr_block.data[i]);
        REQUIRE(column_indices[i] == static_cast<std::size_t>(csr_block.column_indices[i]));
    }
    for (std::int64_t i = 0; i < csr_block.row_indices.get_count(); i++) {
        REQUIRE(row_indices[i] == static_cast<std::size_t>(csr_block.row_indices[i]));
    }
}

} // namespace oneapi::dal::detail
