/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoints/post_connect/bitflyer/post_connect_bitflyer.h"

#include <utility>

#include "base/json/json_writer.h"
#include "bat/ledger/internal/logging/logging.h"

namespace ledger::endpoints {

PostConnectBitflyer::PostConnectBitflyer(LedgerImpl* ledger,
                                         std::string&& linking_info)
    : PostConnect(ledger) {
  if (linking_info.empty()) {
    BLOG(0, "linking_info is empty!");
    return;
  }

  base::Value::Dict content;
  content.Set("linkingInfo", std::move(linking_info));

  std::string json;
  if (!base::JSONWriter::Write(content, &json)) {
    BLOG(0, "Failed to write content to JSON!");
    return;
  }

  content_ = json;
}

PostConnectBitflyer::~PostConnectBitflyer() = default;

const char* PostConnectBitflyer::Path() const {
  return "/v3/wallet/bitflyer/%s/claim";
}

}  // namespace ledger::endpoints
