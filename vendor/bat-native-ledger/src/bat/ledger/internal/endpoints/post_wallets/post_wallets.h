/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_POST_WALLETS_POST_WALLETS_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_POST_WALLETS_POST_WALLETS_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/types/expected.h"
#include "bat/ledger/internal/endpoints/request_builder.h"
#include "bat/ledger/internal/endpoints/response_handler.h"
#include "bat/ledger/internal/logging/logging_util.h"
#include "bat/ledger/mojom_structs.h"
#include "brave/vendor/bat-native-ledger/src/bat/ledger/internal/endpoints/error_types.mojom.h"

// POST /v4/wallets
//
// Request body:
// {
//   "geo_country": "US"
// }
//
// clang-format off
// Response body:
// {
//   "paymentId": "33fe956b-ed15-515b-bccd-b6cc63a80e0e",
//   "walletProvider": {
//     "id": "",
//     "name": "brave"
//   },
//   "altcurrency": "BAT",
//   "publicKey": "6c859e59ad816d57a38bf895c8a4c1af173fd59c5754ea1e714ba5e1617e46aa"  // NOLINT
// }
// clang-format on

namespace ledger {
class LedgerImpl;

namespace endpoints {

class PostWallets final : public RequestBuilder,
                          public ResponseHandler<PostWallets> {
 public:
  using Value = std::string;  // payment_id
  using Error = mojom::PostWalletsError;
  using Response = base::expected<Value, Error>;

  static Response ProcessResponse(const type::UrlResponse&);

  explicit PostWallets(LedgerImpl*, std::string&& geo_country);
  ~PostWallets() override;

 private:
  const char* Path() const;

  absl::optional<std::string> Url() override;
  absl::optional<std::vector<std::string>> Headers() override;
  absl::optional<std::string> Content() override;
  std::string ContentType() override;

  absl::optional<std::string> content_;
};

}  // namespace endpoints
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_POST_WALLETS_POST_WALLETS_H_
