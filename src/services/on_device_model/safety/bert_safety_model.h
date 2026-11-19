// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_ON_DEVICE_MODEL_SAFETY_BERT_SAFETY_MODEL_H_
#define SERVICES_ON_DEVICE_MODEL_SAFETY_BERT_SAFETY_MODEL_H_

#include <memory>
#include <string>
#include <string_view>

#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/unique_receiver_set.h"
#include "services/on_device_model/public/mojom/on_device_model.mojom.h"
#include "services/on_device_model/public/mojom/on_device_model_service.mojom.h"

namespace on_device_model {

// Hallulium hollow: no TFLite BERT safety model.
class BertSafetyModel final : public mojom::TextSafetyModel,
                              public mojom::TextSafetySession {
 public:
  ~BertSafetyModel() override;

  static std::unique_ptr<BertSafetyModel> Create(
      mojom::TextSafetyModelParamsPtr params);

  void StartSession(
      mojo::PendingReceiver<mojom::TextSafetySession> session) override;

  void ClassifyTextSafety(const std::string& text,
                          ClassifyTextSafetyCallback callback) override;
  void DetectLanguage(const std::string& text,
                      DetectLanguageCallback callback) override;
  void Clone(mojo::PendingReceiver<mojom::TextSafetySession> session) override;

  mojom::SafetyInfoPtr ClassifyTextSafety(const std::string& text);
  mojom::LanguageDetectionResultPtr DetectLanguage(std::string_view text);

 private:
  BertSafetyModel();
  mojo::ReceiverSet<mojom::TextSafetySession> sessions_;
};

}  // namespace on_device_model

#endif  // SERVICES_ON_DEVICE_MODEL_SAFETY_BERT_SAFETY_MODEL_H_
