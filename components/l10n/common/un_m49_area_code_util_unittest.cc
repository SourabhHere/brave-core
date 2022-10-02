/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/l10n/common/un_m49_area_code_util.h"

#include "brave/components/l10n/common/un_m49_area_code_constants.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=UNM49AreaCodeUtilTest*

namespace brave_l10n {

TEST(UNM49AreaCodeUtilTest, IsUNM49AreaCode) {
  for (const auto& area_code : kUnM49AreaCodes) {
    EXPECT_TRUE(IsUNM49AreaCode(area_code));
  }
}

TEST(UNM49AreaCodeUtilTest, IsNotUNM49AreaCode) {
  EXPECT_FALSE(IsUNM49AreaCode(/*country_code*/ "US"));
}

}  // namespace brave_l10n
