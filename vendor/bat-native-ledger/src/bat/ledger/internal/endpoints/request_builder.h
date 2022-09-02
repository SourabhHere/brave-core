/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_REQUEST_BUILDER_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "bat/ledger/mojom_structs.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ledger {
class LedgerImpl;

namespace endpoints {

class RequestBuilder {
 public:
  virtual ~RequestBuilder();

  absl::optional<type::UrlRequestPtr> Request();

 protected:
  explicit RequestBuilder(LedgerImpl*);

  virtual absl::optional<std::string> Url() = 0;

  virtual type::UrlMethod Method();

  virtual absl::optional<std::vector<std::string>> Headers();

  virtual absl::optional<std::string> Content();

  virtual std::string ContentType();

  virtual bool SkipLog();

  virtual uint32_t LoadFlags();

  LedgerImpl* ledger_;  // NOT OWNED
};

}  // namespace endpoints
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINTS_REQUEST_BUILDER_H_
