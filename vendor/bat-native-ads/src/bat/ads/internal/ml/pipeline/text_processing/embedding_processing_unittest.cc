/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ml/pipeline/text_processing/embedding_processing.h"

#include <tuple>
#include <vector>

#include "bat/ads/internal/base/unittest/unittest_base.h"
#include "bat/ads/internal/ml/data/vector_data.h"
#include "bat/ads/internal/resources/contextual/text_embedding/text_embedding_resource.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

namespace {

constexpr char kResourceFile[] = "wtpwsrqtjxmfdwaymauprezkunxprysm";
constexpr char kSimpleResourceFile[] =
    "resources/wtpwsrqtjxmfdwaymauprezkunxprysm_simple";

}  // namespace

class BatAdsEmbeddingProcessingPipelineTest : public UnitTestBase {
 protected:
  BatAdsEmbeddingProcessingPipelineTest() = default;

  ~BatAdsEmbeddingProcessingPipelineTest() override = default;
};

TEST_F(BatAdsEmbeddingProcessingPipelineTest, EmbedText) {
  // Arrange
  CopyFileFromTestPathToTempPath(kSimpleResourceFile, kResourceFile);

  resource::TextEmbedding resource;
  resource.Load();

  task_environment_.RunUntilIdle();
  ASSERT_TRUE(resource.IsInitialized());

  ml::pipeline::EmbeddingProcessing* embedding_processing = resource.Get();
  ASSERT_TRUE(embedding_processing);

  const std::vector<float> data1 = {0.5, 0.4, 1.0};
  const std::vector<float> data2 = {0.0, 0.0, 0.0};
  const std::vector<float> data3 = {0.85, 0.2, 1.0};
  const std::vector<std::tuple<std::string, ml::VectorData>> kSamples = {
      {"this simple unittest", ml::VectorData(data1)},
      {"this is a simple unittest", ml::VectorData(data1)},
      {"this is @ #1a simple unittest", ml::VectorData(data1)},
      {"that is a test", ml::VectorData(data2)},
      {"this 54 is simple", ml::VectorData(data3)},
      {{}, {}}};

  for (const auto& [text, expected_embedding] : kSamples) {
    // Act
    const ml::pipeline::TextEmbeddingInfo text_embedding =
        embedding_processing->EmbedText(text);
    // Assert
    EXPECT_EQ(expected_embedding.GetValuesForTesting(),
              text_embedding.embedding.GetValuesForTesting());
  }
}

}  // namespace ads
