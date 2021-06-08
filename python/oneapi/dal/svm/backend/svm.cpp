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

#include "oneapi/dal/algo/svm.hpp"
#include "oneapi/dal/common/backend/common_py.hpp"
#include "oneapi/dal/primitives/backend/kernel_functions_py.hpp"

namespace py = pybind11;

namespace oneapi::dal::backend {

template <typename Float, typename Method, typename Task, typename Kernel>
auto get_descriptor(const py::dict& params) {
    using namespace svm;

    constexpr bool is_cls = std::is_same_v<Task, task::classification>;
    constexpr bool is_nu_cls = std::is_same_v<Task, task::nu_classification>;
    constexpr bool is_reg = std::is_same_v<Task, task::regression>;
    constexpr bool is_nu_reg = std::is_same_v<Task, task::nu_regression>;

    auto desc = descriptor<Float, Method, Task, Kernel>{ get_kernel_descriptor<Kernel>(params) }
        .set_max_iteration_count(params["max_iteration_count"].cast<std::int64_t>())
        .set_accuracy_threshold(params["accuracy_threshold"].cast<double>())
        .set_cache_size(params["cache_size"].cast<double>())
        .set_tau(params["tau"].cast<double>())
        .set_shrinking(params["shrinking"].cast<bool>());

    if constexpr(is_cls || is_reg || is_nu_reg) {
        desc.set_c(params["c"].cast<double>());
    }
    if constexpr(is_cls || is_nu_cls) {
        desc.set_class_count(params["class_count"].cast<std::int64_t>());
    }
    if constexpr(is_reg) {
        desc.set_epsilon(params["epsilon"].cast<double>());
    }
    if constexpr(is_nu_reg || is_nu_cls) {
        desc.set_nu(params["nu"].cast<double>());
    }

    return desc;
}

template <typename Policy, typename Input, typename Ops>
struct params_dispatcher {
    ONEDAL_DECLARE_PARAMS_DISPATCHER_CTOR()
    ONEDAL_DECLARE_PARAMS_DISPATCHER_DISPATCH(dispatch_fptype)
    ONEDAL_DECLARE_PARAMS_DISPATCHER_DISPATCH_FPTYPE(dispatch_method)

    template <typename Float>
    auto dispatch_method(const py::dict& params) {
        using namespace svm;

        auto method = params["method"].cast<std::string>();
        if constexpr (std::is_same_v<Task, task::classification>) {
            ONEDAL_PARAM_DISPATCH_VALUE(method, "thunder", dispatch_kernel, Float, method::thunder)
            ONEDAL_PARAM_DISPATCH_VALUE(method, "smo", dispatch_kernel, Float, method::smo)
            ONEDAL_PARAM_DISPATCH_SECTION_END(method)
        } else {
            ONEDAL_PARAM_DISPATCH_VALUE(method, "thunder", dispatch_kernel, Float, method::thunder)
            ONEDAL_PARAM_DISPATCH_SECTION_END(method)
        }
    }

    template <typename Float, typename Method>
    auto dispatch_kernel(const py::dict& params) {
        using namespace svm;

        auto kernel = params["kernel"].cast<std::string>();
        ONEDAL_PARAM_DISPATCH_VALUE(kernel, "linear", run, Float, Method, Task, linear_kernel::descriptor<Float>)
        ONEDAL_PARAM_DISPATCH_VALUE(kernel, "rbf", run, Float, Method, Task, rbf_kernel::descriptor<Float>)
        ONEDAL_PARAM_DISPATCH_VALUE(kernel, "poly", run, Float, Method, Task, polynomial_kernel::descriptor<Float>)
        ONEDAL_PARAM_DISPATCH_SECTION_END(kernel)
    }

    template <typename Float, typename Method, typename Task, typename Kernel>
    auto run(const py::dict& params) {
        ONEDAL_DECLARE_PARAMS_DISPATCHER_RUN_BODY(Float, Method, Task, Kernel)
    }
};

template <typename Policy, typename Task>
void init_train_ops(py::module_& m) {
    m.def("train", [](const Policy& policy,
                      const py::dict& params,
                      const table& data,
                      const table& labels,
                      const table& weights) {
        using namespace svm;
        using input_t = train_input<Task>;
        params_dispatcher d { policy, input_t{data, labels, weights}, train_ops{} };
        return d.dispatch(params);
    });
}

template <typename Policy, typename Task>
void init_infer_ops(py::module_& m) {
    m.def("infer", [](const Policy& policy,
                      const py::dict& params,
                      const svm::model<Task>& model,
                      const table& data) {
        using namespace svm;
        using input_t = infer_input<Task>;
        params_dispatcher d { policy, input_t{model, data}, infer_ops{} };
        return d.dispatch(params);
    });
}

template <typename Task>
void init_model(py::module_& m) {
    using namespace svm;
    using model_t = model<Task>;

    auto cls = py::class_<model_t>(m, "model")
        .def(py::init())
        .def(py::pickle(
            [](const model_t& m) { return serialize(m); },
            [](const py::bytes& bytes) { return deserialize<model_t>(bytes); }))
        .def_property_readonly("support_vector_count", &model_t::get_support_vector_count)
        .DEF_ONEDAL_PY_PROPERTY(support_vectors, model_t)
        .DEF_ONEDAL_PY_PROPERTY(coeffs, model_t)
        .DEF_ONEDAL_PY_PROPERTY(biases, model_t);

    constexpr bool is_classification = std::is_same_v<Task, task::classification>;
    constexpr bool is_nu_classification = std::is_same_v<Task, task::nu_classification>;

    if constexpr (is_classification || is_nu_classification) {
        cls.def_property("first_class_label", &model_t::get_first_class_label, &model_t::template set_first_class_label<>);
        cls.def_property("second_class_label", &model_t::get_second_class_label, &model_t::template set_second_class_label<>);
    }
}

template <typename Task>
void init_train_result(py::module_& m) {
    using namespace svm;
    using result_t = train_result<Task>;

    py::class_<result_t>(m, "train_result")
        .def(py::init())
        .DEF_ONEDAL_PY_PROPERTY(model, result_t)
        .DEF_ONEDAL_PY_PROPERTY(support_vectors, result_t)
        .DEF_ONEDAL_PY_PROPERTY(support_indices, result_t)
        .DEF_ONEDAL_PY_PROPERTY(coeffs, result_t)
        .DEF_ONEDAL_PY_PROPERTY(biases, result_t);
}

template <typename Task>
void init_infer_result(py::module_& m) {
    using namespace svm;
    using result_t = infer_result<Task>;

    auto cls = py::class_<result_t>(m, "infer_result")
        .def(py::init())
        .DEF_ONEDAL_PY_PROPERTY(labels, result_t);

    constexpr bool is_classification = std::is_same_v<Task, task::classification>;
    constexpr bool is_nu_classification = std::is_same_v<Task, task::nu_classification>;

    if constexpr (is_classification || is_nu_classification) {
        cls.def_property("decision_function", &result_t::get_decision_function, &result_t::template set_decision_function<>);
    }
}

ONEDAL_PY_TYPE2STR(svm::task::classification, "classification");
ONEDAL_PY_TYPE2STR(svm::task::regression, "regression");
ONEDAL_PY_TYPE2STR(svm::task::nu_classification, "nu_classification");
ONEDAL_PY_TYPE2STR(svm::task::nu_regression, "nu_regression");

ONEDAL_PY_DECLARE_INSTANTIATOR(init_model);
ONEDAL_PY_DECLARE_INSTANTIATOR(init_train_result);
ONEDAL_PY_DECLARE_INSTANTIATOR(init_infer_result);
ONEDAL_PY_DECLARE_INSTANTIATOR(init_train_ops);
ONEDAL_PY_DECLARE_INSTANTIATOR(init_infer_ops);

ONEDAL_PY_INIT_MODULE(svm) {
    using namespace svm;
    using namespace dal::detail;

    using task_list = types<task::classification, task::regression, task::nu_classification, task::nu_regression>;
    using policy_list = types<host_policy, data_parallel_policy>;
    auto sub = m.def_submodule("svm");

    ONEDAL_PY_INSTANTIATE(init_train_ops, sub, policy_list, task_list);
    ONEDAL_PY_INSTANTIATE(init_infer_ops, sub, policy_list, task_list);

    ONEDAL_PY_INSTANTIATE(init_model, sub, task_list);
    ONEDAL_PY_INSTANTIATE(init_train_result, sub, task_list);
    ONEDAL_PY_INSTANTIATE(init_infer_result, sub, task_list);
}

} // namespace oneapi::dal::backend
