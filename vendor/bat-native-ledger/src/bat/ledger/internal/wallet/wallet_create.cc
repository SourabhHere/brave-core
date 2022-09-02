/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/wallet/wallet_create.h"

#include <utility>

#include "base/types/expected.h"
#include "bat/ledger/internal/common/security_util.h"
#include "bat/ledger/internal/common/time_util.h"
#include "bat/ledger/internal/endpoints/patch_wallets/patch_wallets.h"
#include "bat/ledger/internal/endpoints/post_wallets/post_wallets.h"
#include "bat/ledger/internal/endpoints/request_for.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/logging/event_log_keys.h"

using ledger::endpoints::PatchWallets;
using ledger::endpoints::PostWallets;
using ledger::endpoints::RequestFor;

namespace ledger::wallet {

WalletCreate::WalletCreate(LedgerImpl* ledger) : ledger_(ledger) {
  DCHECK(ledger_);
}

void WalletCreate::CreateWallet(ResultCallback callback,
                                std::string&& geo_country) {
  bool corrupted = false;
  auto wallet = ledger_->wallet()->GetWallet(&corrupted);

  if (corrupted) {
    DCHECK(!wallet);
    BLOG(0, "Rewards wallet data is corrupted - generating a new wallet!");
    ledger_->database()->SaveEventLog(log::kWalletCorrupted, "");
  }

  if (!wallet) {
    wallet = type::RewardsWallet::New();
    wallet->recovery_seed = util::Security::GenerateSeed();

    if (!ledger_->wallet()->SetWallet(std::move(wallet))) {
      BLOG(0, "Failed to set Rewards wallet!");
      return std::move(callback).Run(type::Result::LEDGER_ERROR);
    }
  } else if (!wallet->payment_id.empty()) {
    if (!geo_country.empty()) {
      auto on_update_wallet = base::BindOnce(
          &WalletCreate::OnResponse<PatchWallets::Response>,
          base::Unretained(this), std::move(callback), geo_country);

      if (RequestFor<PatchWallets> request{ledger_, std::move(geo_country)}) {
        return std::move(request).Send(std::move(on_update_wallet));
      } else {
        return std::move(on_update_wallet)
            .Run(base::unexpected(
                PatchWallets::Error::FAILED_TO_CREATE_REQUEST));
      }
    } else {
      BLOG(1, "Rewards wallet already exists.");
      return std::move(callback).Run(type::Result::WALLET_CREATED);
    }
  }

  auto on_create_wallet =
      base::BindOnce(&WalletCreate::OnResponse<PostWallets::Response>,
                     base::Unretained(this), std::move(callback), geo_country);

  if (RequestFor<PostWallets> request{ledger_, std::move(geo_country)}) {
    std::move(request).Send(std::move(on_create_wallet));
  } else {
    std::move(on_create_wallet)
        .Run(base::unexpected(PostWallets::Error::FAILED_TO_CREATE_REQUEST));
  }
}

template <typename>
inline constexpr bool dependent_false_v = false;

template <typename Response>
void WalletCreate::OnResponse(ResultCallback callback,
                              std::string&& geo_country,
                              Response&& response) {
  if (!response.has_value()) {
    if constexpr (std::is_same_v<Response, PostWallets::Response>) {
      BLOG(0, "Failed to create Rewards wallet!");
    } else if constexpr (std::is_same_v<Response, PatchWallets::Response>) {
      BLOG(0, "Failed to update Rewards wallet!");
    } else {
      static_assert(dependent_false_v<Response>,
                    "Response must be either "
                    "PostWallets::Response, or "
                    "PatchWallets::Response!");
    }

    return std::move(callback).Run(type::Result::LEDGER_ERROR);
  }

  auto wallet = ledger_->wallet()->GetWallet();
  DCHECK(wallet);
  if constexpr (std::is_same_v<Response, PostWallets::Response>) {
    DCHECK(!response.value().empty());
    wallet->payment_id = std::move(response.value());
  }
  wallet->geo_country = std::move(geo_country);

  if (!ledger_->wallet()->SetWallet(std::move(wallet))) {
    BLOG(0, "Failed to set Rewards wallet!");
    return std::move(callback).Run(type::Result::LEDGER_ERROR);
  }

  if constexpr (std::is_same_v<Response, PostWallets::Response>) {
    ledger_->state()->ResetReconcileStamp();
    if (!ledger::is_testing) {
      ledger_->state()->SetEmptyBalanceChecked(true);
      ledger_->state()->SetPromotionCorruptedMigrated(true);
    }
    ledger_->state()->SetCreationStamp(util::GetCurrentTimeStamp());
  }

  std::move(callback).Run(type::Result::WALLET_CREATED);
}

}  // namespace ledger::wallet
