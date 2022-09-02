/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoints/post_wallets/post_wallets.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "bat/ledger/internal/common/request_util.h"
#include "bat/ledger/internal/common/security_util.h"
#include "bat/ledger/internal/endpoint/promotion/promotions_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "net/http/http_status_code.h"

namespace ledger::endpoints {
using Value = PostWallets::Value;
using Error = PostWallets::Error;
using Response = PostWallets::Response;

namespace {

absl::optional<Value> ParseBody(const std::string& body) {
  const auto value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid body!");
    return absl::nullopt;
  }

  const auto* payment_id = value->GetDict().FindString("paymentId");
  if (!payment_id || payment_id->empty()) {
    BLOG(0, "Failed to get payment ID!");
    return absl::nullopt;
  }

  return *payment_id;
}

}  // namespace

// static
Response PostWallets::ProcessResponse(const type::UrlResponse& response) {
  switch (response.status_code) {
    case net::HTTP_CREATED: {  // HTTP 201
      auto payment_id = ParseBody(response.body);
      if (payment_id) {
        return std::move(*payment_id);
      } else {
        return base::unexpected(Error::FAILED_TO_PARSE_BODY);
      }
    }
    case net::HTTP_BAD_REQUEST:  // HTTP 400
      BLOG(0, "Invalid request!");
      return base::unexpected(Error::INVALID_REQUEST);
    case net::HTTP_UNAUTHORIZED:  // HTTP 401
      BLOG(0, "Invalid public key!");
      return base::unexpected(Error::INVALID_PUBLIC_KEY);
    case net::HTTP_FORBIDDEN:  // HTTP 403
      BLOG(0, "Wallet generation disabled for country code!");
      return base::unexpected(
          Error::WALLET_GENERATION_DISABLED_FOR_COUNTRY_CODE);
    case net::HTTP_CONFLICT:  // HTTP 409
      BLOG(0, "Wallet already exists!");
      return base::unexpected(Error::WALLET_ALREADY_EXISTS);
    case net::HTTP_INTERNAL_SERVER_ERROR:  // HTTP 500
      BLOG(0, "Unexpected error!");
      return base::unexpected(Error::UNEXPECTED_ERROR);
    default:
      BLOG(0, "Unexpected status code! (HTTP " << response.status_code << ')');
      return base::unexpected(Error::UNEXPECTED_STATUS_CODE);
  }
}

PostWallets::PostWallets(LedgerImpl* ledger, std::string&& geo_country)
    : RequestBuilder(ledger) {
  if (geo_country.empty()) {
    BLOG(1, "geo_country is empty - creating old wallet.");
    return;
  }

  base::Value::Dict content;
  content.Set("geo_country", std::move(geo_country));

  std::string json;
  if (!base::JSONWriter::Write(content, &json)) {
    BLOG(0, "Failed to write content to JSON!");
    return;
  }

  content_ = json;
}

PostWallets::~PostWallets() = default;

const char* PostWallets::Path() const {
  return content_ ? "/v4/wallets" : "/v3/wallet/brave";
}

absl::optional<std::string> PostWallets::Url() {
  return ledger::endpoint::promotion::GetServerUrl(Path());
}

absl::optional<std::vector<std::string>> PostWallets::Headers() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Rewards wallet is null!");
    return absl::nullopt;
  }

  return util::BuildSignHeaders(
      std::string("post ") + Path(), content_ ? *content_ : "",
      util::Security::GetPublicKeyHexFromSeed(wallet->recovery_seed),
      wallet->recovery_seed);
}

absl::optional<std::string> PostWallets::Content() {
  return content_ ? std::move(content_) : "";
}

std::string PostWallets::ContentType() {
  return "application/json; charset=utf-8";
}

}  // namespace ledger::endpoints
