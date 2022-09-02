/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_PATCH_WALLETS_PATCH_WALLETS_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_PATCH_WALLETS_PATCH_WALLETS_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/types/expected.h"
#include "bat/ledger/internal/endpoints/request_builder.h"
#include "bat/ledger/internal/endpoints/response_handler.h"
#include "bat/ledger/mojom_structs.h"
#include "brave/vendor/bat-native-ledger/src/bat/ledger/internal/endpoints/error_types.mojom.h"

// PATCH /v4/wallets/<rewards_payment_id>
//
// Request body:
// {
//   "geo_country": "US"
// }
//
// Response body: -

namespace ledger {
class LedgerImpl;

namespace endpoints {

class PatchWallets final : public RequestBuilder,
                           public ResponseHandler<PatchWallets> {
 public:
  using Value = void;
  using Error = mojom::PatchWalletsError;
  using Response = base::expected<Value, Error>;

  static Response ProcessResponse(const type::UrlResponse&);

  explicit PatchWallets(LedgerImpl*, std::string&& geo_country);
  ~PatchWallets() override;

 private:
  const char* Path() const;

  absl::optional<std::string> Url() override;
  type::UrlMethod Method() override;
  absl::optional<std::vector<std::string>> Headers() override;
  absl::optional<std::string> Content() override;
  std::string ContentType() override;

  absl::optional<std::string> content_;
};

}  // namespace endpoints
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_PATCH_WALLETS_PATCH_WALLETS_H_
