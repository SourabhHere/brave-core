/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_

#include "bat/ledger/internal/endpoint/endpoint_base.h"
#include "bat/ledger/internal/logging/logging_util.h"
#include "bat/ledger/ledger_client.h"
#include "net/http/http_status_code.h"

template <typename>
struct CallbackTypeImpl;

template <typename R, typename C, typename P1, typename... Ps>
struct CallbackTypeImpl<R (C::*)(P1, Ps...)> {
  using type = P1;
};

template <typename T>
using CallbackType = typename CallbackTypeImpl<T>::type;

namespace ledger {
class LedgerImpl;

namespace endpoint {

template <typename Impl, type::UrlMethod method>
class Endpoint : public EndpointBase {
 public:
  Endpoint(LedgerImpl* ledger) : EndpointBase(ledger) {}

  virtual ~Endpoint() = default;

  virtual std::string Url() = 0;

  virtual std::vector<std::string> Headers() { return {}; }

  virtual std::string Content() { return {}; }

  virtual std::string ContentType() {
    return "application/json; charset=utf-8";
  }

  virtual bool SkipLog() { return {}; }

  virtual uint32_t LoadFlags() { return {}; }

  template <typename Callback>
  void Request(Callback callback) {
    static_assert(std::is_same_v<Callback, typename Impl::Callback>);
    static_assert(std::is_invocable_r_v<void, decltype(&Impl::OnLoadURL), Impl*,
                                        Callback, const type::UrlResponse&>,
                  "OnLoadURL() has to be a member function of Impl (void "
                  "Impl::OnLoadURL(typename Impl::Callback, const "
                  "type::UrlResponse&)!");
    static_assert(std::is_same_v<CallbackType<decltype(&Impl::OnLoadURL)>,
                                 typename Impl::Callback>);

    Retry(base::BindOnce(&Impl::OnLoadURL,
                         base::Unretained(static_cast<Impl*>(this)),
                         std::move(callback)));

    // LoadURL(std::move(request),
    //         base::BindOnce(&Impl::OnLoadURL,
    //                        base::Unretained(static_cast<Impl*>(this)),
    //                        std::move(callback)));
  }

  void Retry(absl::optional<client::LoadURLCallback> callback) {
    if (callback) {
      auto request = type::UrlRequest::New();
      request->url = Url();
      request->method = method;
      request->headers = Headers();
      request->content = Content();
      request->content_type = ContentType();
      request->skip_log = SkipLog();
      request->load_flags = LoadFlags();

      LoadURL(
          std::move(request),
          base::BindOnce(
              [](client::LoadURLCallback callback,
                 const type::UrlResponse& response) {
                ledger::LogUrlResponse(__func__, response);
                if (response.status_code == net::HTTP_TOO_MANY_REQUESTS) {
                  return absl::optional<client::LoadURLCallback>{std::move(callback)};
                } else {
                  std::move(callback).Run(response);
                  return absl::optional<client::LoadURLCallback>{};
                }
              },
              std::move(*callback))
              .Then(base::BindOnce(&Endpoint::Retry, base::Unretained(this))));
    }
  }
};

}  // namespace endpoint
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_ENDPOINT_ENDPOINT_H_
