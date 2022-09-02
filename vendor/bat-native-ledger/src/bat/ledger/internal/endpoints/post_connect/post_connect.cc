/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoints/post_connect/post_connect.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/common/request_util.h"
#include "bat/ledger/internal/endpoint/promotion/promotions_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "net/http/http_status_code.h"

namespace ledger::endpoints {
using Error = PostConnect::Error;
using Response = PostConnect::Response;

namespace {

Response ParseBody(const std::string& body) {
  const auto value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid body!");
    return base::unexpected(Error::FAILED_TO_PARSE_BODY);
  }

  const base::Value::Dict& dict = value->GetDict();
  const auto* message = dict.FindString("message");
  if (!message) {
    BLOG(0, "message is missing!");
    return base::unexpected(Error::FAILED_TO_PARSE_BODY);
  }

  if (message->find("KYC required") != std::string::npos) {
    // HTTP 403: Uphold
    return base::unexpected(Error::KYC_REQUIRED);
  } else if (message->find("mismatched provider accounts") !=
             std::string::npos) {
    // HTTP 403: bitFlyer, Gemini, Uphold
    return base::unexpected(Error::MISMATCHED_PROVIDER_ACCOUNTS);
  } else if (message->find("transaction verification failure") !=
             std::string::npos) {
    // HTTP 403: Uphold
    return base::unexpected(Error::TRANSACTION_VERIFICATION_FAILURE);
  } else if (message->find("request signature verification failure") !=
             std::string::npos) {
    // HTTP 403: bitFlyer, Gemini
    return base::unexpected(Error::REQUEST_SIGNATURE_VERIFICATION_FAILURE);
  } else if (message->find("unable to link - unusual activity") !=
             std::string::npos) {
    // HTTP 400: bitFlyer, Gemini, Uphold
    return base::unexpected(Error::FLAGGED_WALLET);
  } else if (message->find("region not supported") != std::string::npos) {
    // HTTP 400: bitFlyer, Gemini, Uphold
    return base::unexpected(Error::REGION_NOT_SUPPORTED);
  } else if (message->find("mismatched provider account regions") !=
             std::string::npos) {
    // HTTP 400: bitFlyer, Gemini, Uphold
    return base::unexpected(Error::MISMATCHED_PROVIDER_ACCOUNT_REGIONS);
  } else {
    // bitFlyer, Gemini, Uphold
    BLOG(0, "Unknown message!");
    return base::unexpected(Error::UNKNOWN_MESSAGE);
  }
}

}  // namespace

// static
Response PostConnect::ProcessResponse(const type::UrlResponse& response) {
  switch (response.status_code) {
    case net::HTTP_OK:  // HTTP 200
      return {};
    case net::HTTP_BAD_REQUEST:  // HTTP 400
      BLOG(0, "Invalid request");
      return ParseBody(response.body);
    case net::HTTP_FORBIDDEN:  // HTTP 403
      BLOG(0, "Forbidden");
      return ParseBody(response.body);
    case net::HTTP_NOT_FOUND:  // HTTP 404
      BLOG(0, "KYC required!");
      return base::unexpected(Error::KYC_REQUIRED);
    case net::HTTP_CONFLICT:  // HTTP 409
      BLOG(0, "Device limit reached!");
      return base::unexpected(Error::DEVICE_LIMIT_REACHED);
    case net::HTTP_INTERNAL_SERVER_ERROR:  // HTTP 500
      BLOG(0, "Unexpected error!");
      return base::unexpected(Error::UNEXPECTED_ERROR);
    default:
      BLOG(0, "Unexpected status code! (HTTP " << response.status_code << ')');
      return base::unexpected(Error::UNEXPECTED_STATUS_CODE);
  }
}

// static
type::Result PostConnect::ResponseToResult(const Response& response) {
  if (!response.has_value()) {
    switch (response.error()) {
      case Error::FAILED_TO_CREATE_REQUEST:
        return type::Result::LEDGER_ERROR;
      case Error::FLAGGED_WALLET:  // HTTP 400
        return type::Result::FLAGGED_WALLET;
      case Error::MISMATCHED_PROVIDER_ACCOUNT_REGIONS:  // HTTP 400
        return type::Result::MISMATCHED_PROVIDER_ACCOUNT_REGIONS;
      case Error::REGION_NOT_SUPPORTED:  // HTTP 400
        return type::Result::REGION_NOT_SUPPORTED;
      case Error::UNKNOWN_MESSAGE:  // HTTP 400, HTTP 403
        return type::Result::LEDGER_ERROR;
      case Error::KYC_REQUIRED:  // HTTP 403, HTTP 404
        return type::Result::NOT_FOUND;
      case Error::MISMATCHED_PROVIDER_ACCOUNTS:  // HTTP 403
        return type::Result::MISMATCHED_PROVIDER_ACCOUNTS;
      case Error::REQUEST_SIGNATURE_VERIFICATION_FAILURE:  // HTTP 403
        return type::Result::REQUEST_SIGNATURE_VERIFICATION_FAILURE;
      case Error::TRANSACTION_VERIFICATION_FAILURE:  // HTTP 403
        return type::Result::UPHOLD_TRANSACTION_VERIFICATION_FAILURE;
      case Error::DEVICE_LIMIT_REACHED:  // HTTP 409
        return type::Result::DEVICE_LIMIT_REACHED;
      case Error::UNEXPECTED_ERROR:  // HTTP 500
        return type::Result::LEDGER_ERROR;
      case Error::UNEXPECTED_STATUS_CODE:  // HTTP xxx
        return type::Result::LEDGER_ERROR;
      case Error::FAILED_TO_PARSE_BODY:
        return type::Result::LEDGER_ERROR;
    }
  }

  return type::Result::LEDGER_OK;
}

PostConnect::PostConnect(LedgerImpl* ledger) : RequestBuilder(ledger) {}

PostConnect::~PostConnect() = default;

absl::optional<std::string> PostConnect::Url() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Rewards wallet is null!");
    return absl::nullopt;
  }

  return ledger::endpoint::promotion::GetServerUrl(
      base::StringPrintf(Path(), wallet->payment_id.c_str()));
}

absl::optional<std::vector<std::string>> PostConnect::Headers() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Rewards wallet is null!");
    return absl::nullopt;
  }

  if (!content_) {
    return absl::nullopt;
  }

  return util::BuildSignHeaders(
      "post " + base::StringPrintf(Path(), wallet->payment_id.c_str()),
      *content_, wallet->payment_id, wallet->recovery_seed);
}

absl::optional<std::string> PostConnect::Content() {
  return std::move(content_);
}

std::string PostConnect::ContentType() {
  return "application/json; charset=utf-8";
}
};  // namespace ledger::endpoints
