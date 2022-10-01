/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/vendor/bat-native-ledger/include/bat/ledger/public/interfaces/ledger_mojom_traits.h"

#include <iterator>
#include <utility>
#include <vector>

namespace mojo {

// static
bool StructTraits<ledger::mojom::StringSetDataView, std::set<std::string>>::
    Read(ledger::mojom::StringSetDataView data, std::set<std::string>* out) {
  std::vector<std::string> vector;
  if (!data.ReadElements(&vector)) {
    return false;
  }

  *out = std::set(std::move_iterator(vector.begin()),
                  std::move_iterator(vector.end()));

  return true;
}

}  // namespace mojo
