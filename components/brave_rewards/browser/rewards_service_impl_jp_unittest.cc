/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <map>

#include "base/command_line.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/raw_ptr.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ledger/global_constants.h"
#include "bat/ledger/mojom_structs.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/components/brave_rewards/browser/rewards_service_impl.h"
#include "brave/components/brave_rewards/browser/rewards_service_observer.h"
#include "brave/components/brave_rewards/browser/test_util.h"
#include "brave/components/brave_rewards/common/features.h"
#include "brave/components/brave_rewards/common/pref_names.h"
#include "brave/components/brave_rewards/common/rewards_flags.h"
#include "brave/components/greaselion/browser/buildflags/buildflags.h"
#include "brave/components/l10n/common/locale_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/test/base/testing_browser_process.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=RewardsServiceJPTest.*

namespace brave_rewards {

using ::testing::NiceMock;
using ::testing::Return;

class RewardsServiceJPTest : public testing::Test {
 public:
  RewardsServiceJPTest() = default;
  ~RewardsServiceJPTest() override = default;

 protected:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    brave_l10n::icu::SetLocaleForTesting("ja_JP");
    profile_ = CreateBraveRewardsProfile(temp_dir_.GetPath());
    ASSERT_TRUE(profile_);
#if BUILDFLAG(ENABLE_GREASELION)
    auto* rewards_ = new RewardsServiceImpl(profile(), nullptr);
#else
    auto* rewards_ = new RewardsServiceImpl(profile());
#endif
    RewardsServiceFactory::SetServiceForTesting(std::move(rewards_));
    rewards_service_ = static_cast<RewardsServiceImpl*>(
        RewardsServiceFactory::GetForProfile(profile()));
    ASSERT_TRUE(RewardsServiceFactory::GetInstance());
    ASSERT_TRUE(rewards_service());

    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
    command_line->AppendSwitchASCII("rewards", "countryid=19024");
    rewards_service_->HandleFlags(RewardsFlags::ForCurrentProcess());
  }

  void TearDown() override {
    TestingBrowserProcess::GetGlobal()->SetLocalState(nullptr);
    delete rewards_service_;
    profile_.reset();
  }

  Profile* profile() { return profile_.get(); }
  RewardsServiceImpl* rewards_service() { return rewards_service_; }

#if BUILDFLAG(ENABLE_GEMINI_WALLET)
  void EnableGemini() {
    feature_list_.InitAndEnableFeature(features::kGeminiFeature);
  }

  void DisableGemini() {
    feature_list_.InitAndDisableFeature(features::kGeminiFeature);
  }
#endif

 protected:
  base::test::ScopedFeatureList feature_list_;

 private:
  // Need this as a very first member to run tests in UI thread
  // When this is set, class should not install any other MessageLoops, like
  // base::test::ScopedTaskEnvironment
  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<Profile> profile_;
  raw_ptr<RewardsServiceImpl> rewards_service_ = nullptr;
  base::ScopedTempDir temp_dir_;
};

#if BUILDFLAG(ENABLE_GEMINI_WALLET)
TEST_F(RewardsServiceJPTest, GetExternalWallet) {
  DisableGemini();
  EXPECT_EQ(rewards_service()->GetExternalWalletType(),
            ledger::constant::kWalletBitflyer);
}

TEST_F(RewardsServiceJPTest, GetExternalWalletMultipleCustodians) {
  EnableGemini();
  EXPECT_EQ(rewards_service()->GetExternalWalletType(),
            ledger::constant::kWalletBitflyer);
  profile()->GetPrefs()->SetString(prefs::kExternalWalletType,
                                   ledger::constant::kWalletUphold);
  EXPECT_EQ(rewards_service()->GetExternalWalletType(),
            ledger::constant::kWalletBitflyer);
  profile()->GetPrefs()->SetString(prefs::kExternalWalletType,
                                   ledger::constant::kWalletGemini);
  EXPECT_EQ(rewards_service()->GetExternalWalletType(),
            ledger::constant::kWalletBitflyer);
}
#endif

}  // namespace brave_rewards
