// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/mediastream/media_stream_audio_processor.h"

#include <memory>
#include <optional>
#include <string_view>

#include "base/memory/raw_ptr.h"
#include "base/task/single_thread_task_runner.h"
#include "build/build_config.h"
#include "media/base/audio_parameters.h"
#include "third_party/blink/public/platform/modules/webrtc/webrtc_logging.h"
#include "third_party/blink/renderer/platform/mediastream/aec_dump_agent_impl.h"
#include "third_party/blink/renderer/platform/wtf/functional.h"

namespace blink {
namespace {
void WebRtcLogStringPiece(std::string_view message) {
  WebRtcLogMessage(std::string{message});
}
}  // namespace

MediaStreamAudioProcessor::MediaStreamAudioProcessor(
    DeliverProcessedAudioCallback deliver_processed_audio_callback,
    const media::AudioProcessingSettings& settings,
    const media::AudioParameters& capture_data_source_params)
    : audio_processor_(media::AudioProcessor::Create(
          std::move(deliver_processed_audio_callback),
          /*log_callback=*/
          BindRepeating(&WebRtcLogStringPiece),
          settings,
          capture_data_source_params,
          media::AudioProcessor::GetDefaultOutputFormat(
              capture_data_source_params,
              settings),
          /*neural_residual_echo_estimator_model=*/nullptr)),
      main_thread_runner_(base::SingleThreadTaskRunner::GetCurrentDefault()),
      aec_dump_agent_impl_(AecDumpAgentImpl::Create(this)),
      stopped_(false) {
  DCHECK(main_thread_runner_);
  DETACH_FROM_THREAD(capture_thread_checker_);
}

MediaStreamAudioProcessor::~MediaStreamAudioProcessor() {
  DCHECK(main_thread_runner_->BelongsToCurrentThread());
  Stop();
}

void MediaStreamAudioProcessor::ProcessCapturedAudio(
    const media::AudioBus& audio_source,
    base::TimeTicks audio_capture_time,
    int num_preferred_channels,
    double volume) {
  DCHECK_CALLED_ON_VALID_THREAD(capture_thread_checker_);
  audio_processor_->ProcessCapturedAudio(audio_source, audio_capture_time,
                                         num_preferred_channels, volume);
}

void MediaStreamAudioProcessor::Stop() {
  DCHECK(main_thread_runner_->BelongsToCurrentThread());
  if (stopped_)
    return;
  stopped_ = true;

  aec_dump_agent_impl_.reset();
  audio_processor_->OnStopDump();
}

const media::AudioParameters&
MediaStreamAudioProcessor::GetInputFormatForTesting() const {
  return audio_processor_->input_format();
}

void MediaStreamAudioProcessor::OnStartDump(base::File dump_file) {
  DCHECK(main_thread_runner_->BelongsToCurrentThread());
  audio_processor_->OnStartDump(std::move(dump_file));
}

void MediaStreamAudioProcessor::OnStopDump() {
  DCHECK(main_thread_runner_->BelongsToCurrentThread());
  audio_processor_->OnStopDump();
}

webrtc::AudioProcessorInterface::AudioProcessorStatistics
MediaStreamAudioProcessor::GetStats(bool has_remote_tracks) {
  AudioProcessorStatistics stats;
  stats.apm_statistics = audio_processor_->GetStats();
  return stats;
}

}  // namespace blink
