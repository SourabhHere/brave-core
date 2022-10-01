/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_L10N_COMMON_LOCALE_UTIL_H_
#define BRAVE_COMPONENTS_L10N_COMMON_LOCALE_UTIL_H_

#include <string>

namespace brave_l10n::icu {

class ScopedLocaleForTesting final {
 public:
  // Override the ICU locale for testing. |locale| should be in the format
  // <language>_<country_code> or <language>-<country_code>, e.g. "en_US" or
  // "en-001". Restores the previous locale on destruction.
  explicit ScopedLocaleForTesting(const std::string& locale);

  ~ScopedLocaleForTesting();
};

// Set the ICU locale for testing. |locale| should be in the format
// <language>_<country_code> or <language>-<country_code>, e.g. "en_US" or
// "en-001".
void SetLocaleForTesting(const std::string& locale);

// Returns the ICU style C locale in <language>_<country_code> format, e.g.
// "en_US".
std::string GetLocale();

// Returns a lowercase two-letter ISO 639-1 language code for the ICU locale,
// see https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes.
std::string GetLanguageCode();

// Returns a lowercase two-letter ISO 639-1 language code for the given locale,
// see https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes. |locale| should be
// in the format <language>_<country_code> or <language>-<country_code>, e.g.
// "en_US" or "en-001".
std::string GetLanguageCode(const std::string& locale);

// Returns an uppercase two-letter ISO 3166-1 alpha-2 country code or UN-M49
// area code for the ICU locale. See
// https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2, and
// https://en.wikipedia.org/wiki/UN_M49.
std::string GetCountryCode();

// Returns an uppercase two-letter ISO 3166-1 alpha-2 country code or UN-M49
// area code for the given locale. Use |IsUNM49AreaCode| to check if the
// country code is a UN M49 area code. |locale| should be in the format
// <language>_<country_code> or <language>-<country_code>, e.g. "en_US" or
// "en-001". See https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2, and
// https://en.wikipedia.org/wiki/UN_M49.
std::string GetCountryCode(const std::string& locale);

}  // namespace brave_l10n::icu

#endif  // BRAVE_COMPONENTS_L10N_COMMON_LOCALE_UTIL_H_
