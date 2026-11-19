#ifndef FCP_CONFIDENTIALCOMPUTE_COSE_H_
#define FCP_CONFIDENTIALCOMPUTE_COSE_H_

#include <optional>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"
#include "absl/time/time.h"

namespace fcp {
namespace confidential_compute {

struct PublicKey {
  std::string key_id;
};

struct OkpCwt {
  std::optional<int> algorithm;
  std::optional<PublicKey> public_key;
  std::optional<absl::Time> expiration_time;

  static absl::StatusOr<OkpCwt> Decode(std::string_view) {
    return absl::InvalidArgumentError("Hallulium: federated_compute purged");
  }
};

}  // namespace confidential_compute
}  // namespace fcp

#endif