// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_PUBLIC_CPP_SHARED_STORAGE_MOJOM_TRAITS_BLINK_H_
#define SERVICES_NETWORK_PUBLIC_CPP_SHARED_STORAGE_MOJOM_TRAITS_BLINK_H_

#include "mojo/public/cpp/bindings/struct_ptr.h"
#include "services/network/public/mojom/shared_storage.mojom-shared.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace mojo {

template <>
struct StructTraits<network::mojom::SharedStorageKeyArgumentDataView,
                    blink::String> {
  static const blink::String& data(const blink::String& input) { return input; }

  static bool Read(network::mojom::SharedStorageKeyArgumentDataView data,
                   blink::String* out) {
    if (!data.ReadData(out)) {
      return false;
    }
    const unsigned length = out->length();
    return length != 0u && length <= 5242880u / 2u;
  }
};

template <>
struct StructTraits<network::mojom::SharedStorageValueArgumentDataView,
                    blink::String> {
  static const blink::String& data(const blink::String& input) { return input; }

  static bool Read(network::mojom::SharedStorageValueArgumentDataView data,
                   blink::String* out) {
    if (!data.ReadData(out)) {
      return false;
    }
    return out->length() <= 5242880u / 2u;
  }
};

template <>
struct StructTraits<network::mojom::LockNameDataView, blink::String> {
  static const blink::String& data(const blink::String& input) { return input; }

  static bool Read(network::mojom::LockNameDataView data, blink::String* out) {
    if (!data.ReadData(out)) {
      return false;
    }
    return out->empty() || (*out)[0] != '-';
  }
};

template <>
struct StructTraits<
    network::mojom::SharedStorageBatchUpdateMethodsArgumentDataView,
    blink::Vector<mojo::StructPtr<
        network::mojom::blink::SharedStorageModifierMethodWithOptions>>> {
  static const blink::Vector<mojo::StructPtr<
      network::mojom::blink::SharedStorageModifierMethodWithOptions>>&
  data(const blink::Vector<mojo::StructPtr<
           network::mojom::blink::SharedStorageModifierMethodWithOptions>>&
           input) {
    return input;
  }

  static bool Read(
      network::mojom::SharedStorageBatchUpdateMethodsArgumentDataView data,
      blink::Vector<mojo::StructPtr<
          network::mojom::blink::SharedStorageModifierMethodWithOptions>>*
          out) {
    return data.ReadData(out);
  }
};

}  // namespace mojo

#endif  // SERVICES_NETWORK_PUBLIC_CPP_SHARED_STORAGE_MOJOM_TRAITS_BLINK_H_
