/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/l10n/common/locale_util.h"

#include "brave/components/l10n/common/locale_constants.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=LocaleUtilTest*

namespace brave_l10n::icu {

TEST(LocaleUtilTest, DefaultLanguageCode) {
  EXPECT_EQ(std::string{"en"}, kDefaultLanguageCode);
}

TEST(LocaleUtilTest, DefaultCountryCode) {
  EXPECT_EQ(std::string{"US"}, kDefaultCountryCode);
}

TEST(LocaleUtilTest, GetLocale) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("ja_JP", GetLocale());
}

TEST(LocaleUtilTest, GetDefaultLocale) {
  brave_l10n::icu::SetLocaleForTesting({});
  EXPECT_EQ("en_US", GetLocale());
}

TEST(LocaleUtilTest, GetLanguageCode) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("ja", GetLanguageCode());
}

TEST(LocaleUtilTest, GetLanguageCodeForLocale) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("en", GetLanguageCode("en_US"));
}

TEST(LocaleUtilTest, GetCountryCode) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("JP", GetCountryCode());
}

TEST(LocaleUtilTest, GetCountryCodeForLocale) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("US", GetCountryCode("en_US"));
}

TEST(LocaleUtilTest, GetUNM49CountryCode) {
  brave_l10n::icu::SetLocaleForTesting("en_001");
  EXPECT_EQ("001", GetCountryCode());
}

TEST(LocaleUtilTest, GetUNM49CountryCodeForLocale) {
  brave_l10n::icu::SetLocaleForTesting("ja_JP");
  EXPECT_EQ("001", GetCountryCode("en_001"));
}

}  // namespace brave_l10n::icu
