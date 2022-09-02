/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoints/request_builder.h"

#include <utility>

namespace ledger::endpoints {

RequestBuilder::~RequestBuilder() = default;

absl::optional<type::UrlRequestPtr> RequestBuilder::Request() {
  const auto url = Url();
  if (!url) {
    return absl::nullopt;
  }

  auto headers = Headers();
  if (!headers) {
    return absl::nullopt;
  }

  const auto content = Content();
  if (!content) {
    return absl::nullopt;
  }

  return type::UrlRequest::New(*url, Method(), std::move(*headers), *content,
                               ContentType(), SkipLog(), LoadFlags());
}

RequestBuilder::RequestBuilder(LedgerImpl* ledger) : ledger_(ledger) {
  DCHECK(ledger);
}

type::UrlMethod RequestBuilder::Method() {
  return type::UrlMethod::POST;
}

absl::optional<std::vector<std::string>> RequestBuilder::Headers() {
  return std::vector<std::string>{};
}

absl::optional<std::string> RequestBuilder::Content() {
  return "";
}

std::string RequestBuilder::ContentType() {
  return "";
}

bool RequestBuilder::SkipLog() {
  return false;
}

uint32_t RequestBuilder::LoadFlags() {
  return 0;
}

};  // namespace ledger::endpoints
