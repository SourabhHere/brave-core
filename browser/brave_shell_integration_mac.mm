/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_shell_integration_mac.h"

#include "base/mac/bundle_locations.h"
#include "chrome/browser/mac/dock.h"

namespace shell_integration::mac {

void AddIconToDock() {
  if (dock::ChromeIsInTheDock() == dock::ChromeInDockFalse)
    dock::AddIcon([base::mac::MainBundle() bundlePath], nullptr);
}

}  // namespace shell_integration::mac
