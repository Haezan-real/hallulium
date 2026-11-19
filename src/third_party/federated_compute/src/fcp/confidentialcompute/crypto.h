#ifndef FCP_CONFIDENTIALCOMPUTE_CRYPTO_H_
#define FCP_CONFIDENTIALCOMPUTE_CRYPTO_H_

#include <string>
#include <string_view>

#include "absl/status/statusor.h"

namespace fcp {
namespace confidential_compute {

struct EncryptResult {
  std::string ciphertext;
  std::string encrypted_symmetric_key;
  std::string encapped_key;
};

class MessageEncryptor {
 public:
  absl::StatusOr<EncryptResult> Encrypt(std::string_view,
                                        std::string_view,
                                        std::string_view) {
    return absl::FailedPreconditionError("Hallulium: federated_compute purged");
  }
};

}  // namespace confidential_compute
}  // namespace fcp

#endif