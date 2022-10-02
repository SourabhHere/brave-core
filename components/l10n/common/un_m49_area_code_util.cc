/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/l10n/common/un_m49_area_code_util.h"

#include "base/containers/contains.h"
#include "base/strings/string_piece.h"
#include "brave/components/l10n/common/un_m49_area_code_constants.h"

namespace brave_l10n {

bool IsUNM49AreaCode(const base::StringPiece area_code) {
  return base::Contains(kUnM49AreaCodes, area_code);
}

}  // namespace brave_l10n
