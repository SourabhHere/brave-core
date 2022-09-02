/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <tuple>
#include <utility>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "bat/ledger/internal/endpoints/post_wallets/post_wallets.h"
#include "bat/ledger/internal/endpoints/request_for.h"
#include "bat/ledger/internal/ledger_client_mock.h"
#include "bat/ledger/internal/ledger_impl_mock.h"
#include "bat/ledger/internal/state/state_keys.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=*PostWallets*

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::Values;

namespace ledger::endpoints::test {

using Error = PostWallets::Error;
using Response = PostWallets::Response;

// clang-format off
using PostWalletsParamType = std::tuple<
    std::string,          // test name suffix
    net::HttpStatusCode,  // post create wallet endpoint response status code
    std::string,          // post create wallet endpoint response body
    Response              // expected response
>;
// clang-format on

class PostWallets : public TestWithParam<PostWalletsParamType> {
 public:
  PostWallets(const PostWallets&) = delete;
  PostWallets& operator=(const PostWallets&) = delete;

  PostWallets(PostWallets&&) = delete;
  PostWallets& operator=(PostWallets&&) = delete;

 private:
  base::test::TaskEnvironment scoped_task_environment_;

 protected:
  PostWallets()
      : mock_ledger_client_(), mock_ledger_impl_(&mock_ledger_client_) {}

  void SetUp() override {
    const std::string wallet =
        R"(
          {
            "payment_id": "",
            "recovery_seed": "AN6DLuI2iZzzDxpzywf+IKmK1nzFRarNswbaIDI3pQg="
          }
        )";

    ON_CALL(mock_ledger_client_, GetStringState(state::kWalletBrave))
        .WillByDefault(Return(wallet));
  }

  MockLedgerClient mock_ledger_client_;
  MockLedgerImpl mock_ledger_impl_;
};

TEST_P(PostWallets, Paths) {
  const auto& [ignore, status_code, body, expected_response] = GetParam();

  ON_CALL(mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(Invoke(
          [status_code = status_code, body = body](
              type::UrlRequestPtr, client::LoadURLCallback callback) mutable {
            type::UrlResponse response;
            response.status_code = status_code;
            response.body = std::move(body);
            std::move(callback).Run(response);
          }));

  RequestFor<endpoints::PostWallets> request{&mock_ledger_impl_, "geo_country"};
  EXPECT_TRUE(request);

  std::move(request).Send(base::BindLambdaForTesting(
      [expected_response = expected_response](Response&& response) {
        EXPECT_EQ(response, expected_response);
      }));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
  Endpoints,
  PostWallets,
  Values(
    PostWalletsParamType{
      "0_HTTP_201_success",
      net::HTTP_CREATED,
      R"(
        {
          "paymentId": "284a68ea-95ac-559a-b95c-5f07b4db0c72",
          "walletProvider": {
            "id": "",
            "name": "brave"
          },
          "altcurrency": "BAT",
          "publicKey": "7de76306129de620d01406cdd5a72c5e0ea2e427504f0faff2ba5788c81f2e76"
        }
      )",
      "284a68ea-95ac-559a-b95c-5f07b4db0c72"
    },
    PostWalletsParamType{
      "1_HTTP_400_invalid_request",
      net::HTTP_BAD_REQUEST,
      "",
      base::unexpected(Error::INVALID_REQUEST)
    },
    PostWalletsParamType{
      "2_HTTP_401_invalid_public_key",
      net::HTTP_UNAUTHORIZED,
      "",
      base::unexpected(Error::INVALID_PUBLIC_KEY)
    },
    PostWalletsParamType{
      "3_HTTP_403_wallet_generation_disabled_for_country_code",
      net::HTTP_FORBIDDEN,
      "",
      base::unexpected(Error::WALLET_GENERATION_DISABLED_FOR_COUNTRY_CODE)
    },
    PostWalletsParamType{
      "4_HTTP_409_wallet_already_exists",
      net::HTTP_CONFLICT,
      "",
      base::unexpected(Error::WALLET_ALREADY_EXISTS)
    },
    PostWalletsParamType{
      "5_HTTP_500_unexpected_error",
      net::HTTP_INTERNAL_SERVER_ERROR,
      "",
      base::unexpected(Error::UNEXPECTED_ERROR)
    },
    PostWalletsParamType{
      "6_HTTP_503_unexpected_status_code",
      net::HTTP_SERVICE_UNAVAILABLE,
      "",
      base::unexpected(Error::UNEXPECTED_STATUS_CODE)
    }),
  [](const TestParamInfo<PostWalletsParamType>& info) {
    return std::get<0>(info.param);
  }
);
// clang-format on

}  // namespace ledger::endpoints::test
