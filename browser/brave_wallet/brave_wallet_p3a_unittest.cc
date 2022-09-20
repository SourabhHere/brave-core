/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/brave_wallet_p3a.h"
#include "base/test/metrics/histogram_tester.h"
#include "brave/browser/brave_wallet/brave_wallet_service_factory.h"
#include "brave/browser/brave_wallet/keyring_service_factory.h"
#include "brave/components/brave_wallet/browser/brave_wallet_service.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/keyring_service.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/components/brave_wallet/common/features.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace brave_wallet {

class BraveWalletP3AUnitTest : public testing::Test {
 public:
  BraveWalletP3AUnitTest() {
    histogram_tester_ = std::make_unique<base::HistogramTester>();
  }

  void SetUp() override {
    TestingProfile::Builder builder;
    profile_ = builder.Build();
    keyring_service_ =
        KeyringServiceFactory::GetServiceForContext(profile_.get());
    wallet_service_ =
        brave_wallet::BraveWalletServiceFactory::GetServiceForContext(
            profile_.get());
  }
  void WaitForResponse() { task_environment_.RunUntilIdle(); }

  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<TestingProfile> profile_;
  std::unique_ptr<base::HistogramTester> histogram_tester_;
  KeyringService* keyring_service_;
  BraveWalletService* wallet_service_;
};

TEST_F(BraveWalletP3AUnitTest, DefaultEthereumWalletSetting) {
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWalletPreferExtension), 1);
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::None), 0);
  wallet_service_->SetDefaultEthereumWallet(mojom::DefaultWallet::None);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::None), 1);
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::CryptoWallets), 0);
  wallet_service_->SetDefaultEthereumWallet(
      mojom::DefaultWallet::CryptoWallets);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::CryptoWallets), 1);
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWallet), 0);
  wallet_service_->SetDefaultEthereumWallet(mojom::DefaultWallet::BraveWallet);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWallet), 1);
  wallet_service_->SetDefaultEthereumWallet(
      mojom::DefaultWallet::BraveWalletPreferExtension);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWalletPreferExtension), 2);
}

TEST_F(BraveWalletP3AUnitTest, DefaultSolanaWalletSetting) {
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWalletPreferExtension), 1);
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::None), 0);
  wallet_service_->SetDefaultSolanaWallet(mojom::DefaultWallet::None);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::None), 1);
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::CryptoWallets), 0);
  wallet_service_->SetDefaultSolanaWallet(mojom::DefaultWallet::BraveWallet);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWallet), 1);
  wallet_service_->SetDefaultSolanaWallet(
      mojom::DefaultWallet::BraveWalletPreferExtension);
  WaitForResponse();
  histogram_tester_->ExpectBucketCount(
      "Brave.Wallet.DefaultSolanaWalletSetting",
      static_cast<int>(mojom::DefaultWallet::BraveWalletPreferExtension), 2);
}

TEST_F(BraveWalletP3AUnitTest, KeyringCreated) {
  base::test::ScopedFeatureList feature_list;
  base::FieldTrialParams parameters;
  parameters[features::kCreateDefaultSolanaAccount.name] = "false";

  std::vector<base::test::ScopedFeatureList::FeatureAndParams> enabled_features;
  enabled_features.emplace_back(
      brave_wallet::features::kBraveWalletSolanaFeature, parameters);

  feature_list.InitWithFeaturesAndParameters(enabled_features, {});

  histogram_tester_->ExpectBucketCount("Brave.Wallet.KeyringCreated", 0, 1);
  keyring_service_->CreateWallet("testing123", base::DoNothing());
  WaitForResponse();
  histogram_tester_->ExpectBucketCount("Brave.Wallet.KeyringCreated", 1, 1);
}

TEST_F(BraveWalletP3AUnitTest, ActiveAccounts) {
  wallet_service_->RecordActiveWalletCountForP3A(0, mojom::CoinType::ETH);
  wallet_service_->RecordActiveWalletCountForP3A(0, mojom::CoinType::FIL);
  wallet_service_->RecordActiveWalletCountForP3A(0, mojom::CoinType::SOL);

  // Should not record zero to histogram if user never had an active account
  histogram_tester_->ExpectTotalCount(kEthActiveAccountHistogramName, 0);
  histogram_tester_->ExpectTotalCount(kFilActiveAccountHistogramName, 0);
  histogram_tester_->ExpectTotalCount(kSolActiveAccountHistogramName, 0);

  wallet_service_->RecordActiveWalletCountForP3A(3, mojom::CoinType::ETH);
  wallet_service_->RecordActiveWalletCountForP3A(9, mojom::CoinType::FIL);
  wallet_service_->RecordActiveWalletCountForP3A(7, mojom::CoinType::SOL);

  histogram_tester_->ExpectBucketCount(kEthActiveAccountHistogramName, 3, 1);
  histogram_tester_->ExpectBucketCount(kFilActiveAccountHistogramName, 5, 1);
  histogram_tester_->ExpectBucketCount(kSolActiveAccountHistogramName, 4, 1);

  wallet_service_->RecordActiveWalletCountForP3A(0, mojom::CoinType::ETH);
  wallet_service_->RecordActiveWalletCountForP3A(1, mojom::CoinType::FIL);
  wallet_service_->RecordActiveWalletCountForP3A(2, mojom::CoinType::SOL);

  histogram_tester_->ExpectBucketCount(kEthActiveAccountHistogramName, 0, 1);
  histogram_tester_->ExpectBucketCount(kFilActiveAccountHistogramName, 1, 1);
  histogram_tester_->ExpectBucketCount(kSolActiveAccountHistogramName, 2, 1);
}

}  // namespace brave_wallet
