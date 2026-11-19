// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/language_detection/core/language_detection_model.h"

#include "base/functional/callback.h"

namespace language_detection {

Prediction TopPrediction(const std::vector<Prediction>& predictions) {
  if (predictions.empty()) {
    return Prediction("und", 0.f);
  }
  Prediction best = predictions[0];
  for (const auto& p : predictions) {
    if (best < p) {
      best = p;
    }
  }
  return best;
}

LanguageDetectionModel::LanguageDetectionModel() = default;
LanguageDetectionModel::~LanguageDetectionModel() = default;

std::vector<Prediction> LanguageDetectionModel::Predict(
    std::u16string_view) const {
  return {};
}

std::vector<Prediction> LanguageDetectionModel::PredictWithScan(
    std::u16string_view) const {
  return {};
}

Prediction LanguageDetectionModel::PredictTopLanguageWithSamples(
    std::u16string_view) const {
  return Prediction("und", 0.f);
}

void LanguageDetectionModel::UpdateWithFile(base::File) {}

void LanguageDetectionModel::UpdateWithFileAsync(base::File,
                                                 base::OnceClosure callback) {
  std::move(callback).Run();
}

bool LanguageDetectionModel::IsAvailable() const {
  return false;
}

int64_t LanguageDetectionModel::GetModelSize() const {
  return 0;
}

void LanguageDetectionModel::AddOnModelLoadedCallback(ModelLoadedCallback) {}

std::string LanguageDetectionModel::GetModelVersion() const {
  return "hallulium-hollow";
}

}  // namespace language_detection
