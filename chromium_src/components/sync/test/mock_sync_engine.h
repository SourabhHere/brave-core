/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_COMPONENTS_SYNC_TEST_MOCK_SYNC_ENGINE_H_
#define BRAVE_CHROMIUM_SRC_COMPONENTS_SYNC_TEST_MOCK_SYNC_ENGINE_H_

#define BRAVE_MOCK_SYNC_ENGINE_H_                                   \
  MOCK_METHOD(void, PermanentlyDeleteAccount,                       \
              (base::OnceCallback<void(const SyncProtocolError&)>), \
              (override));

#include "src/components/sync/test/mock_sync_engine.h"

#undef BRAVE_MOCK_SYNC_ENGINE_H_

#endif  // BRAVE_CHROMIUM_SRC_COMPONENTS_SYNC_TEST_MOCK_SYNC_ENGINE_H_
