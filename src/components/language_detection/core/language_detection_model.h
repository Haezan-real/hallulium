// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_LANGUAGE_DETECTION_CORE_LANGUAGE_DETECTION_MODEL_H_
#define COMPONENTS_LANGUAGE_DETECTION_CORE_LANGUAGE_DETECTION_MODEL_H_

#include <string>
#include <string_view>
#include <vector>

#include "base/component_export.h"
#include "base/files/file.h"
#include "base/functional/callback_forward.h"
#include "base/sequence_checker.h"

namespace language_detection {

inline constexpr size_t kModelTruncationLength = 256;

struct Prediction {
  Prediction(const std::string& language, float score)
      : language(language), score(score) {}
  Prediction() = delete;
  std::string language;
  float score;
  bool operator<(const Prediction& other) const { return score < other.score; }
};

COMPONENT_EXPORT(LANGUAGE_DETECTION)
Prediction TopPrediction(const std::vector<Prediction>& predictions);

enum class LanguageDetectionModelState {
  kUnknown,
  kModelFileInvalid,
  kModelFileValid,
  kModelAvailable,
  kMaxValue = kModelAvailable,
};

// Hallulium hollow: no TFLite language detection.
class COMPONENT_EXPORT(LANGUAGE_DETECTION) LanguageDetectionModel {
 public:
  using ModelLoadedCallback = base::OnceCallback<void(LanguageDetectionModel&)>;

  LanguageDetectionModel();
  ~LanguageDetectionModel();

  std::vector<Prediction> Predict(std::u16string_view contents) const;
  std::vector<Prediction> PredictWithScan(std::u16string_view contents) const;
  Prediction PredictTopLanguageWithSamples(std::u16string_view contents) const;
  void UpdateWithFile(base::File model_file);
  void UpdateWithFileAsync(base::File model_file, base::OnceClosure callback);
  bool IsAvailable() const;
  int64_t GetModelSize() const;
  void AddOnModelLoadedCallback(ModelLoadedCallback callback);
  std::string GetModelVersion() const;
  void DetachFromSequence() { DETACH_FROM_SEQUENCE(sequence_checker_); }

  static constexpr size_t kTextSampleLength = 256;
  static constexpr int kNumTextSamples = 3;
  static constexpr size_t kScanWindowSize = 128;

 private:
  SEQUENCE_CHECKER(sequence_checker_);
};

}  // namespace language_detection

#endif  // COMPONENTS_LANGUAGE_DETECTION_CORE_LANGUAGE_DETECTION_MODEL_H_
