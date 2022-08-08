/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_CONNECT_CONNECT_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_CONNECT_CONNECT_H_

#include "bat/ledger/internal/endpoint/endpoint.h"
#include "bat/ledger/ledger.h"

namespace ledger {
class LedgerImpl;

namespace endpoint {
class Connect : public Endpoint<Connect, type::UrlMethod::POST> {
  friend Endpoint<Connect, type::UrlMethod::POST>;

 public:
  using Callback = base::OnceCallback<void(type::Result)>;

  Connect(LedgerImpl* ledger);

  ~Connect() override;

 private:
  void OnLoadURL(Callback callback, const type::UrlResponse& response);

  type::Result ProcessResponse(const type::UrlResponse& response);

  type::Result ParseBody(const std::string& body);
};
}  // namespace endpoint
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_CONNECT_CONNECT_H_
