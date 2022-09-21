/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/content_settings/core/browser/brave_content_settings_policy_provider.h"

#include "brave/components/constants/pref_names.h"
#include "components/content_settings/core/browser/content_settings_policy_provider.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace {

constexpr const char* kBraveManagedDefaultPrefs[] = {
    kManagedHTTPSEverywhereDefault};

}  // namespace

namespace content_settings {

// Duplicating declaration from
// components/content_settings/core/browser/content_settings_policy_provider.cc
struct PolicyProvider::PrefsForManagedDefaultMapEntry {
  ContentSettingsType content_type;
  const char* pref_name;
};

// static
const PolicyProvider::PrefsForManagedDefaultMapEntry
    BravePolicyProvider::kBravePrefsForManagedDefault[] = {
        {ContentSettingsType::BRAVE_HTTP_UPGRADABLE_RESOURCES,
         kManagedHTTPSEverywhereDefault}};

BravePolicyProvider::BravePolicyProvider(PrefService* prefs)
    : PolicyProvider(prefs) {
  ReadBraveManagedDefaultSettings();
  pref_change_registrar_.Init(prefs_);
  PrefChangeRegistrar::NamedChangeCallback callback = base::BindRepeating(
      &BravePolicyProvider::OnPreferenceChanged, base::Unretained(this));
  for (const char* pref : kBraveManagedDefaultPrefs)
    pref_change_registrar_.Add(pref, callback);
}

BravePolicyProvider::~BravePolicyProvider() = default;

// static
void BravePolicyProvider::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  PolicyProvider::RegisterProfilePrefs(registry);
  for (const char* pref : kBraveManagedDefaultPrefs)
    registry->RegisterIntegerPref(pref, CONTENT_SETTING_DEFAULT);
}

void BravePolicyProvider::ReadBraveManagedDefaultSettings() {
  for (const auto& entry : kBravePrefsForManagedDefault)
    UpdateManagedDefaultSetting(entry);
}

void BravePolicyProvider::OnPreferenceChanged(const std::string& name) {
  for (auto entry : kBravePrefsForManagedDefault) {
    if (entry.pref_name == name)
      UpdateManagedDefaultSetting(entry);
  }

  PolicyProvider::OnPreferenceChanged(name);
}
}  // namespace content_settings
