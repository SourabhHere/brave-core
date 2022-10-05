/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_PAGE_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_PAGE_H_

#include <memory>
#include <string>

#include "base/values.h"
#include "components/security_interstitials/content/security_interstitial_page.h"
#include "url/gurl.h"

namespace content {
class WebContents;
}  // namespace content

namespace security_interstitials {
class SecurityInterstitialControllerClient;
}  // namespace security_interstitials

namespace request_otr {

// RequestOTRPage is the interstitial page which will be shown when the
// browser blocks a top-level, first-party request. A proceed button is
// provided in the page, along with a checkbox to remember the domain and
// bypass this interstitial for this domain in the future.
class RequestOTRPage
    : public security_interstitials::SecurityInterstitialPage {
 public:
  // Interstitial type, used in tests.
  static const security_interstitials::SecurityInterstitialPage::TypeID
      kTypeForTesting;

  RequestOTRPage(
      content::WebContents* web_contents,
      const GURL& request_url,
      std::unique_ptr<
          security_interstitials::SecurityInterstitialControllerClient>
          controller);
  ~RequestOTRPage() override;

  RequestOTRPage(const RequestOTRPage&) = delete;
  RequestOTRPage& operator=(const RequestOTRPage&) = delete;

  // SecurityInterstitialPage:: overrides
  void OnInterstitialClosing() override {}
  void CommandReceived(const std::string& command) override;
  security_interstitials::SecurityInterstitialPage::TypeID GetTypeForTesting()
      override;

 protected:
  // SecurityInterstitialPage:: overrides
  void PopulateInterstitialStrings(base::Value::Dict& load_time_data) override;
  int GetHTMLTemplateId() override;
};

}  // namespace request_otr

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_REQUEST_OTR_PAGE_H_
