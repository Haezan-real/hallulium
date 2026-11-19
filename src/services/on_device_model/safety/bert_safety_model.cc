// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/on_device_model/safety/bert_safety_model.h"

#include "base/memory/ptr_util.h"

namespace on_device_model {

BertSafetyModel::BertSafetyModel() = default;
BertSafetyModel::~BertSafetyModel() = default;

std::unique_ptr<BertSafetyModel> BertSafetyModel::Create(
    mojom::TextSafetyModelParamsPtr params) {
  return base::WrapUnique(new BertSafetyModel());
}

void BertSafetyModel::StartSession(
    mojo::PendingReceiver<mojom::TextSafetySession> session) {
  sessions_.Add(this, std::move(session));
}

void BertSafetyModel::ClassifyTextSafety(const std::string& text,
                                         ClassifyTextSafetyCallback callback) {
  std::move(callback).Run(ClassifyTextSafety(text));
}

void BertSafetyModel::DetectLanguage(const std::string& text,
                                     DetectLanguageCallback callback) {
  std::move(callback).Run(DetectLanguage(text));
}

void BertSafetyModel::Clone(
    mojo::PendingReceiver<mojom::TextSafetySession> session) {
  sessions_.Add(this, std::move(session));
}

mojom::SafetyInfoPtr BertSafetyModel::ClassifyTextSafety(
    const std::string& text) {
  return nullptr;
}

mojom::LanguageDetectionResultPtr BertSafetyModel::DetectLanguage(
    std::string_view text) {
  return nullptr;
}

}  // namespace on_device_model
