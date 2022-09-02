/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoints/patch_wallets/patch_wallets.h"

#include <utility>

#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/common/request_util.h"
#include "bat/ledger/internal/common/security_util.h"
#include "bat/ledger/internal/endpoint/promotion/promotions_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "net/http/http_status_code.h"

namespace ledger::endpoints {
using Error = PatchWallets::Error;
using Response = PatchWallets::Response;

// static
Response PatchWallets::ProcessResponse(const type::UrlResponse& response) {
  switch (response.status_code) {
    case net::HTTP_OK:  // HTTP 200
      return {};
    case net::HTTP_BAD_REQUEST:  // HTTP 400
      BLOG(0, "Invalid request!");
      return base::unexpected(Error::INVALID_REQUEST);
    case net::HTTP_UNAUTHORIZED:  // HTTP 401
      BLOG(0, "Invalid public key!");
      return base::unexpected(Error::INVALID_PUBLIC_KEY);
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

PatchWallets::PatchWallets(LedgerImpl* ledger, std::string&& geo_country)
    : RequestBuilder(ledger) {
  if (geo_country.empty()) {
    BLOG(0, "geo_country is empty!");
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

PatchWallets::~PatchWallets() = default;

const char* PatchWallets::Path() const {
  return "/v4/wallets/%s";
}

absl::optional<std::string> PatchWallets::Url() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Rewards wallet is null!");
    return absl::nullopt;
  }

  return ledger::endpoint::promotion::GetServerUrl(
      base::StringPrintf(Path(), wallet->payment_id.c_str()));
}

type::UrlMethod PatchWallets::Method() {
  return type::UrlMethod::PATCH;
}

absl::optional<std::vector<std::string>> PatchWallets::Headers() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Rewards wallet is null!");
    return absl::nullopt;
  }

  if (!content_) {
    return absl::nullopt;
  }

  return util::BuildSignHeaders(
      "patch " + base::StringPrintf(Path(), wallet->payment_id.c_str()),
      *content_, wallet->payment_id, wallet->recovery_seed);
}

absl::optional<std::string> PatchWallets::Content() {
  return std::move(content_);
}

std::string PatchWallets::ContentType() {
  return "application/json; charset=utf-8";
}

}  // namespace ledger::endpoints
