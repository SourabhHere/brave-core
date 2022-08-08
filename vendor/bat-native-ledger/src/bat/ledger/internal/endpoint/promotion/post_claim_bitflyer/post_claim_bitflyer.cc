/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoint/promotion/post_claim_bitflyer/post_claim_bitflyer.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/common/request_util.h"
#include "bat/ledger/internal/endpoint/promotion/promotions_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/mojom_structs.h"
#include "net/http/http_status_code.h"

namespace {

std::string GetPath(const std::string& payment_id) {
  return base::StringPrintf("/v3/wallet/bitflyer/%s/claim", payment_id.c_str());
}

}  // namespace

namespace ledger {
namespace endpoint {
namespace promotion {

ClaimBitflyer::ClaimBitflyer(LedgerImpl* ledger,
                             const std::string& linking_info)
    : Connect(ledger), linking_info_(linking_info) {}

ClaimBitflyer::~ClaimBitflyer() = default;

std::string ClaimBitflyer::Url() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Wallet is null");
    return "";
  }

  return GetServerUrl(GetPath(wallet->payment_id));
}

std::string ClaimBitflyer::Content() {
  return GeneratePayload(linking_info_);
}

std::vector<std::string> ClaimBitflyer::Headers() {
  const auto wallet = ledger_->wallet()->GetWallet();
  if (!wallet) {
    BLOG(0, "Wallet is null");
    // std::move(callback).Run(type::Result::LEDGER_ERROR);
    return {};
  }

  return util::BuildSignHeaders(
      base::StringPrintf("post %s", GetPath(wallet->payment_id).c_str()),
      Content(), wallet->payment_id, wallet->recovery_seed);
}

std::string ClaimBitflyer::GeneratePayload(const std::string& linking_info) {
  base::Value::Dict payload;
  payload.Set("linkingInfo", linking_info);
  std::string json;
  base::JSONWriter::Write(payload, &json);

  return json;
}

}  // namespace promotion
}  // namespace endpoint
}  // namespace ledger
