/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_RESPONSE_HANDLER_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_RESPONSE_HANDLER_H_

#include <utility>

#include "base/callback.h"
#include "bat/ledger/internal/logging/logging_util.h"
#include "bat/ledger/mojom_structs.h"

namespace ledger::endpoints {

template <typename T>
class ResponseHandler {
 public:
  // Defer instantiation of OnResponse(), as T is an incomplete type at
  // this point.
  template <typename Endpoint = T>
  static void OnResponse(
      base::OnceCallback<void(typename Endpoint::Response&&)> callback,
      const type::UrlResponse& response) {
    ledger::LogUrlResponse(__func__, response);
    std::move(callback).Run(Endpoint::ProcessResponse(response));
  }
};

}  // namespace ledger::endpoints

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_RESPONSE_HANDLER_H_
