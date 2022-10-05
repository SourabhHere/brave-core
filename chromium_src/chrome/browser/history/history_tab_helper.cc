/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/request_otr/browser/request_otr_tab_storage.h"

#define BRAVE_GET_HISTORY_SERVICE                                     \
  request_otr::RequestOTRTabStorage* request_otr_tab_storage =        \
      request_otr::RequestOTRTabStorage::GetOrCreate(web_contents()); \
  if (request_otr_tab_storage->RequestedOTR())                        \
    return NULL;

#include "src/chrome/browser/history/history_tab_helper.cc"

#undef BRAVE_GET_HISTORY_SERVICE
