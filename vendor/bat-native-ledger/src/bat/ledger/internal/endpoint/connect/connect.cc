/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoint/connect/connect.h"

#include "base/json/json_reader.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "net/http/http_status_code.h"

namespace ledger::endpoint {
Connect::Connect(LedgerImpl* ledger)
    : Endpoint<Connect, type::UrlMethod::POST>(ledger) {}

Connect::~Connect() = default;

void Connect::OnLoadURL(Callback callback, const type::UrlResponse& response) {
  ledger::LogUrlResponse(__func__, response);
  std::move(callback).Run(ProcessResponse(response));
}

type::Result Connect::ProcessResponse(const type::UrlResponse& response) {
  const auto status_code = response.status_code;

  if (status_code == net::HTTP_BAD_REQUEST) {
    BLOG(0, "Invalid request");
    return ParseBody(response.body);
  }

  if (status_code == net::HTTP_FORBIDDEN) {
    BLOG(0, "Forbidden");
    return ParseBody(response.body);
  }

  if (status_code == net::HTTP_NOT_FOUND) {
    BLOG(0, "Not found");
    return type::Result::NOT_FOUND;
  }

  if (status_code == net::HTTP_CONFLICT) {
    BLOG(0, "Conflict");
    return type::Result::DEVICE_LIMIT_REACHED;
  }

  if (status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    BLOG(0, "Internal server error");
    return type::Result::LEDGER_ERROR;
  }

  if (status_code != net::HTTP_OK) {
    BLOG(0, "Unexpected HTTP status: " << status_code);
    return type::Result::LEDGER_ERROR;
  }

  return type::Result::LEDGER_OK;
}

type::Result Connect::ParseBody(const std::string& body) {
  auto value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid body!");
    return type::Result::LEDGER_ERROR;
  }

  const base::Value::Dict& dict = value->GetDict();
  const auto* message = dict.FindString("message");
  if (!message) {
    BLOG(0, "message is missing!");
    return type::Result::LEDGER_ERROR;
  }

  if (message->find("mismatched provider accounts") != std::string::npos) {
    return type::Result::MISMATCHED_PROVIDER_ACCOUNTS;
  } else if (message->find("request signature verification failure") !=
             std::string::npos) {
    return type::Result::REQUEST_SIGNATURE_VERIFICATION_FAILURE;
  } else if (message->find("unable to link - unusual activity") !=
             std::string::npos) {
    return type::Result::FLAGGED_WALLET;
  } else if (message->find("region not supported") != std::string::npos) {
    return type::Result::REGION_NOT_SUPPORTED;
  } else {
    BLOG(0, "Unknown message!");
    return type::Result::LEDGER_ERROR;
  }
}
};  // namespace ledger::endpoint
