/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_FEDERATED_CLIENT_FEDERATED_CLIENT_H_
#define BRAVE_COMPONENTS_BRAVE_FEDERATED_CLIENT_FEDERATED_CLIENT_H_

#include <memory>
#include <string>
#include <vector>

#include "brave/third_party/flower/src/cc/flwr/include/client.h"

namespace brave_federated {

class Model;

class FederatedClient final : public flwr::Client {
 public:
  FederatedClient(const std::string& task_name, Model* model);
  ~FederatedClient();

  Model* GetModel();

  void Start();
  void Stop();

  void SetTrainingData(std::vector<std::vector<float>> training_data);
  void SetTestData(std::vector<std::vector<float>> test_data);

  void set_parameters(flwr::Parameters params); // TODO(lminto): not snakecase
  bool is_communicating() override {
    return is_communicating_;
  } 
  flwr::ParametersRes get_parameters() override;
  flwr::PropertiesRes get_properties(flwr::PropertiesIns ins) override;
  flwr::EvaluateRes evaluate(flwr::EvaluateIns ins) override;
  flwr::FitRes fit(flwr::FitIns ins) override;

 private:
  bool is_communicating_ = false;
  std::string client_id_;
  std::string task_name_;
  Model* model_ = nullptr; //TODO(lminto): rawptr
  std::vector<std::vector<float>> training_data_; // TODO(lminto): alias
  std::vector<std::vector<float>> test_data_;
};

}  // namespace brave_federated

#endif  // BRAVE_COMPONENTS_BRAVE_FEDERATED_CLIENT_FEDERATED_CLIENT_H_
