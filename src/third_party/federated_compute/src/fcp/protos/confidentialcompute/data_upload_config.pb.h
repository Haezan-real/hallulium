#ifndef FCP_PROTOS_CONFIDENTIALCOMPUTE_DATA_UPLOAD_CONFIG_PB_H_
#define FCP_PROTOS_CONFIDENTIALCOMPUTE_DATA_UPLOAD_CONFIG_PB_H_

#include <string>

namespace fcp {
namespace confidentialcompute {

class ConfidentialEncryptionConfig {
 public:
  bool has_public_key() const { return !public_key_.empty(); }
  const std::string& public_key() const { return public_key_; }
  void set_public_key(std::string v) { public_key_ = std::move(v); }

 private:
  std::string public_key_;
};

class DataUploadConfig {
 public:
  bool ParseFromString(const std::string&) { return false; }
  bool has_confidential_encryption_config() const { return false; }
  const ConfidentialEncryptionConfig& confidential_encryption_config() const {
    return cfg_;
  }
  ConfidentialEncryptionConfig* mutable_confidential_encryption_config() {
    return &cfg_;
  }

 private:
  ConfidentialEncryptionConfig cfg_;
};

}  // namespace confidentialcompute
}  // namespace fcp

#endif