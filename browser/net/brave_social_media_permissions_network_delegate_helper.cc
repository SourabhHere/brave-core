/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/net/brave_social_media_permissions_network_delegate_helper.h"

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "brave/components/permissions/brave_permission_manager.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/language/core/browser/pref_names.h"
#include "components/permissions/permission_manager.h"
#include "components/permissions/permission_request.h"
#include "components/permissions/permission_request_id.h"
#include "components/permissions/permission_request_manager.h"
#include "components/permissions/permissions_client.h"
#include "content/public/browser/browser_context.h"
#include "extensions/common/url_pattern.h"
#include "net/base/net_errors.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

namespace brave {

namespace {
constexpr char kGoogleAuthPattern[] =
    "https://accounts.google.com/o/oauth2/auth*";
constexpr char kFirebaseContentSettingsPattern[] =
    "https://[*.]firebaseapp.com/__/auth/*";
constexpr char kFirebaseUrlPattern[] = "https://*.firebaseapp.com/__/auth/*";
}  // namespace

bool IsGoogleAuthUrl(const GURL& gurl) {
  static const std::vector<URLPattern> auth_login_patterns({
      URLPattern(URLPattern::SCHEME_HTTPS, kGoogleAuthPattern),
      URLPattern(URLPattern::SCHEME_HTTPS, kFirebaseUrlPattern),
  });
  return std::any_of(
      auth_login_patterns.begin(), auth_login_patterns.end(),
      [&gurl](URLPattern pattern) { return pattern.MatchesURL(gurl); });
}

void Set3pCookieException(HostContentSettingsMap* content_settings,
                          const ContentSettingsPattern& embedding_pattern,
                          const ContentSetting& content_setting) {
  const std::vector<std::string> auth_login_patterns(
      {kGoogleAuthPattern, kFirebaseContentSettingsPattern});
  for (const auto& auth_url_pattern : auth_login_patterns) {
    auto auth_pattern = ContentSettingsPattern::FromString(auth_url_pattern);
    content_settings->SetContentSettingCustomScope(
        auth_pattern, embedding_pattern, ContentSettingsType::BRAVE_COOKIES,
        content_setting);
  }
}

void OnPermissionRequest(
    const ResponseCallback& next_callback,
    const GURL& embedding_url,
    std::shared_ptr<BraveRequestInfo> ctx,
    const std::vector<blink::mojom::PermissionStatus>& permission_statuses) {
  Profile* profile = Profile::FromBrowserContext(ctx->browser_context);
  DCHECK(profile);
  HostContentSettingsMap* content_settings =
      HostContentSettingsMapFactory::GetForProfile(profile);

  DCHECK_EQ(1u, permission_statuses.size());
  auto embedding_pattern = ContentSettingsPattern::FromURL(embedding_url);
  bool is_consent_granted =
      permission_statuses[0] == blink::mojom::PermissionStatus::GRANTED;
  if (is_consent_granted) {
    // Add 3p exception for embedding_url for auth patterns
    Set3pCookieException(content_settings, embedding_pattern,
                         CONTENT_SETTING_ALLOW);
    next_callback.Run();
  } else {
    // Remove 3p exception for embedding_url for auth patterns
    Set3pCookieException(content_settings, embedding_pattern,
                         CONTENT_SETTING_BLOCK);
  }
  return;
}

int OnBeforeURLRequest_SocialMediaPermissionsCheck(
    const ResponseCallback& next_callback,
    std::shared_ptr<BraveRequestInfo> ctx) {
  // Check if request URL matches Google auth patterns
  GURL request_url = ctx->request_url;
  if (!IsGoogleAuthUrl(request_url)) {
    return net::OK;
  }
  GURL embedding_url = ctx->initiator_url;
  auto* web_contents =
      content::WebContents::FromFrameTreeNodeId(ctx->frame_tree_node_id);
  if (!web_contents) {
    return net::OK;
  }
  // Google Sign-In prompts are typically created in new tabs, get the opening
  // web contents since that is what we want to set the permission on
  if (web_contents->HasLiveOriginalOpenerChain()) {
    web_contents = web_contents->GetFirstWebContentsInLiveOriginalOpenerChain();
  }
  // Permission prompts are only allowed after main document is loaded
  // See: PermissionRequestManager::ShowBubble()
  if (!web_contents->IsDocumentOnLoadCompletedInPrimaryMainFrame()) {
    return net::OK;
  }
  auto* permission_manager = static_cast<permissions::BravePermissionManager*>(
      permissions::PermissionsClient::Get()->GetPermissionManager(
          web_contents->GetBrowserContext()));
  if (!permission_manager) {
    return net::OK;
  }
  

  permission_manager->RequestPermissionsForOrigin(
      {blink::PermissionType::BRAVE_GOOGLE_SIGN_IN},
      web_contents->GetPrimaryMainFrame(), embedding_url, true,
      base::BindOnce(&OnPermissionRequest, next_callback, embedding_url, ctx));
  return net::ERR_IO_PENDING;
}
}  // namespace brave
