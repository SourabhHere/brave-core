/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/l10n/common/locale_util.h"

#include "absl/types/optional.h"
#include "base/check.h"
#include "base/no_destructor.h"
#include "brave/components/l10n/common/locale_constants.h"
#include "servloc.h"
#include "third_party/icu/source/common/unicode/locid.h"
#include "ui/base/l10n/l10n_util.h"

namespace brave_l10n::icu {

namespace {

constexpr char kDefaultLocale[] = "en_US";

absl::optional<::icu::Locale>& ICULocaleForTesting() {
  static base::NoDestructor<absl::optional<::icu::Locale>> icu_locale;
  return *icu_locale;
}

absl::optional<::icu::Locale>& LastICULocaleForTesting() {
  static base::NoDestructor<absl::optional<::icu::Locale>> last_icu_locale;
  return *last_icu_locale;
}

inline ::icu::Locale GetNormalizedICULocale(const std::string& locale) {
  return {l10n_util::NormalizeLocale(locale).c_str()};
}

inline std::string GetLocale(const ::icu::Locale& icu_locale) {
  std::string locale{icu_locale.getBaseName()};
  if (locale.empty()) {
    return kDefaultLocale;
  }

  return locale;
}

inline std::string GetLanguageCode(const ::icu::Locale& icu_locale) {
  std::string language_code{icu_locale.getLanguage()};
  if (language_code.empty()) {
    return kDefaultLanguageCode;
  }

  return language_code;
}

inline std::string GetCountryCode(const ::icu::Locale& icu_locale) {
  std::string country_code{icu_locale.getCountry()};
  if (country_code.empty()) {
    return kDefaultCountryCode;
  }

  return country_code;
}

}  // namespace

ScopedLocaleForTesting::ScopedLocaleForTesting(const std::string& locale) {
  LastICULocaleForTesting() = ICULocaleForTesting();
  brave_l10n::icu::SetLocaleForTesting(locale);
}

ScopedLocaleForTesting::~ScopedLocaleForTesting() {
  ICULocaleForTesting() = LastICULocaleForTesting();
}

void SetLocaleForTesting(const std::string& locale) {
  ICULocaleForTesting() = ::icu::Locale(GetNormalizedICULocale(locale));
}

std::string GetLocale() {
  if (ICULocaleForTesting()) {
    return GetLocale(*ICULocaleForTesting());
  }

  return GetLocale(::icu::Locale::getDefault());
}

std::string GetLanguageCode() {
  if (ICULocaleForTesting()) {
    return GetLanguageCode(*ICULocaleForTesting());
  }

  return GetLanguageCode(::icu::Locale::getDefault());
}

std::string GetLanguageCode(const std::string& locale) {
  const ::icu::Locale icu_locale = GetNormalizedICULocale(locale);
  if (icu_locale.isBogus()) {
    return kDefaultLanguageCode;
  }

  return GetLanguageCode(icu_locale);
}

std::string GetCountryCode() {
  if (ICULocaleForTesting()) {
    return GetCountryCode(*ICULocaleForTesting());
  }

  return GetCountryCode(::icu::Locale::getDefault());
}

std::string GetCountryCode(const std::string& locale) {
  const ::icu::Locale icu_locale = GetNormalizedICULocale(locale);
  if (icu_locale.isBogus()) {
    return kDefaultCountryCode;
  }

  return GetCountryCode(icu_locale);
}

}  // namespace brave_l10n::icu
