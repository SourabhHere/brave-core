/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_federated/client/federated_client.h"

#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/threading/sequence_bound.h"
#include "base/threading/sequenced_task_runner_handle.h"

#include "brave/components/brave_federated/client/model.h"
#include "brave/components/brave_federated/linear_algebra_util/linear_algebra_util.h"
#include "brave/components/brave_federated/synthetic_dataset/synthetic_dataset.h"

#include "brave/third_party/flower/src/cc/flwr/include/start.h"
#include "brave/third_party/flower/src/cc/flwr/include/typing.h"

namespace brave_federated {

FederatedClient::FederatedClient(const std::string& task_name, Model* model)
    : task_name_(task_name), model_(model) {
      DCHECK(model_); // TODO(lminto): DCHECK raw pointers
    }

FederatedClient::~FederatedClient() {
  Stop();
}

void FederatedClient::Start() {
  base::SequenceBound<start> flwr_communication( //TODO(lmint): less verbose
      base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::BEST_EFFORT,
           base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN}));
  std::string server_add = "localhost:56102"; // TODO(lminto): move to constexpr

  this->communication_in_progress_ = true;
  flwr_communication.AsyncCall(&start::start_client)
      .WithArgs(server_add, this, 536870912); // TODO(lminto): move to constexpr
}

void FederatedClient::Stop() {
  DCHECK(communication_in_progress_);
  communication_in_progress_ = false;
}

Model* FederatedClient::GetModel() {
  return model_;
}

void FederatedClient::SetTrainingData(
    std::vector<std::vector<float>> training_data) {
  training_data_ = training_data;
}

void FederatedClient::SetTestData(std::vector<std::vector<float>> test_data) {
  test_data_ = test_data;
}

// TODO(lminto): remove this

flwr::ParametersRes FederatedClient::get_parameters() {
  // Serialize
  const std::vector<float> pred_weights = this->model_->PredWeights();
  const float pred_b = this->model_->Bias();

  std::list<std::string> tensors;

  std::ostringstream oss1; 
  oss1.write(reinterpret_cast<const char*>(pred_weights.data()),
             pred_weights.size() * sizeof(float));
  tensors.push_back(oss1.str());

  std::ostringstream oss2;
  oss2.write(reinterpret_cast<const char*>(&pred_b), sizeof(float));
  tensors.push_back(oss2.str());

  std::string tensor_str = "cpp_float";
  return flwr::ParametersRes(flwr::Parameters(tensors, tensor_str));
}

void FederatedClient::set_parameters(flwr::Parameters params) {
  std::list<std::string> s = params.getTensors(); // TODO(lminto): remove all abbrvs

  if (s.empty() == 0) {
    // Layer 1 TODO(lminto): split function, single resp
    auto layer = s.begin();
    size_t num_bytes = (*layer).size();
    const char* weights_char = (*layer).c_str();
    const float* weights_float = reinterpret_cast<const float*>(weights_char);
    std::vector<float> weights(weights_float,
                               weights_float + num_bytes / sizeof(float));
    this->model_->SetPredWeights(weights);

    // Layer 2 = Bias
    auto layer_2 = std::next(layer, 1);
    num_bytes = (*layer_2).size();
    const char* bias_char = (*layer_2).c_str();
    const float* bias_float = reinterpret_cast<const float*>(bias_char);
    this->model_->SetBias(bias_float[0]);
  }
}

flwr::PropertiesRes FederatedClient::get_properties(flwr::PropertiesIns ins) {
  flwr::PropertiesRes p;
  p.setPropertiesRes(static_cast<flwr::Properties>(ins.getPropertiesIns()));
  return p;
}

flwr::FitRes FederatedClient::fit(flwr::FitIns ins) {
  auto config = ins.getConfig();
  flwr::FitRes resp;
  flwr::Parameters p = ins.getParameters();
  this->set_parameters(p);

  std::tuple<size_t, float, float> result =
      this->model_->Train(this->training_data_);

  resp.setParameters(this->get_parameters().getParameters());
  resp.setNum_example(std::get<0>(result));

  return resp;
}

flwr::EvaluateRes FederatedClient::evaluate(flwr::EvaluateIns ins) {
  flwr::EvaluateRes resp;
  flwr::Parameters p = ins.getParameters();
  this->set_parameters(p);

  // Evaluation returns a number_of_examples, a loss and an "accuracy"
  std::tuple<size_t, float, float> result =
      this->model_->Evaluate(this->test_data_);

  resp.setNum_example(std::get<0>(result));
  resp.setLoss(std::get<1>(result));

  flwr::Scalar accuracy = flwr::Scalar();
  accuracy.setDouble(static_cast<double>(std::get<2>(result)));
  std::map<std::string, flwr::Scalar> metric = {
      {"accuracy", accuracy},
  };
  resp.setMetrics(metric);
  auto m = resp.getMetrics();
  return resp;
}

}  // namespace brave_federated
