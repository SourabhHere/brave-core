/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_PUBLIC_INTERFACES_LEDGER_MOJOM_TRAITS_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_PUBLIC_INTERFACES_LEDGER_MOJOM_TRAITS_H_

#include <set>
#include <string>

#include "brave/vendor/bat-native-ledger/include/bat/ledger/public/interfaces/ledger.mojom.h"
#include "mojo/public/cpp/bindings/array_traits_stl.h"
#include "mojo/public/cpp/bindings/struct_traits.h"

namespace mojo {

template <>
struct StructTraits<ledger::mojom::StringSetDataView, std::set<std::string>> {
  static const std::set<std::string>& elements(
      const std::set<std::string>& set) {
    return set;
  }

  static bool Read(ledger::mojom::StringSetDataView data,
                   std::set<std::string>* out);
};

}  // namespace mojo

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_INCLUDE_BAT_LEDGER_PUBLIC_INTERFACES_LEDGER_MOJOM_TRAITS_H_
