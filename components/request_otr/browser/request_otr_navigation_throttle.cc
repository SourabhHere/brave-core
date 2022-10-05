/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/request_otr/browser/request_otr_navigation_throttle.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/feature_list.h"
#include "base/metrics/histogram_macros.h"
#include "brave/components/request_otr/browser/request_otr_controller_client.h"
#include "brave/components/request_otr/browser/request_otr_page.h"
#include "brave/components/request_otr/browser/request_otr_service.h"
#include "brave/components/request_otr/browser/request_otr_tab_storage.h"
#include "brave/components/request_otr/common/features.h"
#include "components/prefs/pref_service.h"
#include "components/security_interstitials/content/security_interstitial_tab_helper.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "net/base/net_errors.h"

namespace request_otr {

// static
std::unique_ptr<RequestOTRNavigationThrottle>
RequestOTRNavigationThrottle::MaybeCreateThrottleFor(
    content::NavigationHandle* navigation_handle,
    RequestOTRService* request_otr_service,
    ephemeral_storage::EphemeralStorageService* ephemeral_storage_service,
    const std::string& locale) {
  if (!request_otr_service)
    return nullptr;
  if (!base::FeatureList::IsEnabled(request_otr::features::kBraveRequestOTR))
    return nullptr;
  // Don't block subframes.
  if (!navigation_handle->IsInMainFrame())
    return nullptr;
  return std::make_unique<RequestOTRNavigationThrottle>(
      navigation_handle, request_otr_service, ephemeral_storage_service,
      locale);
}

RequestOTRNavigationThrottle::RequestOTRNavigationThrottle(
    content::NavigationHandle* navigation_handle,
    RequestOTRService* request_otr_service,
    ephemeral_storage::EphemeralStorageService* ephemeral_storage_service,
    const std::string& locale)
    : content::NavigationThrottle(navigation_handle),
      request_otr_service_(request_otr_service),
      ephemeral_storage_service_(ephemeral_storage_service),
      locale_(locale) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

RequestOTRNavigationThrottle::~RequestOTRNavigationThrottle() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

content::NavigationThrottle::ThrottleCheckResult
RequestOTRNavigationThrottle::WillStartRequest() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  content::NavigationHandle* handle = navigation_handle();
  content::WebContents* web_contents = handle->GetWebContents();
  if (!web_contents || !handle->IsInMainFrame())
      return NavigationThrottle::PROCEED;

  GURL request_url = handle->GetURL();

  // If user has just chosen to proceed on our interstitial, don't show
  // another one.
  RequestOTRTabStorage* tab_storage =
      RequestOTRTabStorage::GetOrCreate(web_contents);
  if (tab_storage->IsProceeding()) {
    return content::NavigationThrottle::PROCEED;
  }

  // Call the request OTR service to determine whether this domain should be
  // blocked.
  if (!request_otr_service_->ShouldBlock(request_url))
    return NavigationThrottle::PROCEED;

  return ShowInterstitial();
}

content::NavigationThrottle::ThrottleCheckResult
RequestOTRNavigationThrottle::WillRedirectRequest() {
  return WillStartRequest();
}

content::NavigationThrottle::ThrottleCheckResult
RequestOTRNavigationThrottle::WillProcessResponse() {
  // If there is an RequestOTRTabStorage associated to |web_contents_|, clear
  // the IsProceeding flag.
  RequestOTRTabStorage* tab_storage = RequestOTRTabStorage::FromWebContents(
      navigation_handle()->GetWebContents());
  if (tab_storage)
    tab_storage->SetIsProceeding(false);
  return content::NavigationThrottle::PROCEED;
}

content::NavigationThrottle::ThrottleCheckResult
RequestOTRNavigationThrottle::ShowInterstitial() {
  content::NavigationHandle* handle = navigation_handle();
  content::WebContents* web_contents = handle->GetWebContents();
  const GURL& request_url = handle->GetURL();
  content::BrowserContext* context =
      handle->GetWebContents()->GetBrowserContext();
  PrefService* pref_service = user_prefs::UserPrefs::Get(context);

  // The controller client implements the actual logic to "go back" or "proceed"
  // from the interstitial.
  auto controller_client = std::make_unique<RequestOTRControllerClient>(
      web_contents, request_url, ephemeral_storage_service_, pref_service, locale_);

  // This handles populating the HTML template of the interstitial page with
  // localized strings and other information we only know at runtime,
  // including the URL of the page we're blocking. Once the user interacts with
  // the interstitial, this translates those actions into method calls on the
  // controller client.
  auto blocked_page = std::make_unique<RequestOTRPage>(
      web_contents, request_url, std::move(controller_client));

  // Get the page content before giving up ownership of |blocked_page|.
  std::string blocked_page_content = blocked_page->GetHTMLContents();

  // Replace the tab contents with our interstitial page.
  security_interstitials::SecurityInterstitialTabHelper::AssociateBlockingPage(
      handle, std::move(blocked_page));

  // Return cancel result
  return content::NavigationThrottle::ThrottleCheckResult(
      content::NavigationThrottle::CANCEL, net::ERR_BLOCKED_BY_CLIENT,
      blocked_page_content);
}

void RequestOTRNavigationThrottle::Enable1PESAndResume() {
  DCHECK(ephemeral_storage_service_);
  RequestOTRTabStorage* tab_storage = RequestOTRTabStorage::FromWebContents(
      navigation_handle()->GetWebContents());
  if (tab_storage) {
    tab_storage->Enable1PESForUrlIfPossible(
        ephemeral_storage_service_, navigation_handle()->GetURL(),
        base::BindOnce(&RequestOTRNavigationThrottle::Resume,
                       weak_ptr_factory_.GetWeakPtr()));
  }
}

const char* RequestOTRNavigationThrottle::GetNameForLogging() {
  return "RequestOTRNavigationThrottle";
}

}  // namespace request_otr
