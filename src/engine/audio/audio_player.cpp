#include "audio_player.h"
#include "../resource/resource_manager.h"
#include <SDL3/SDL_properties.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

namespace engine::audio {

AudioPlayer::AudioPlayer(resource::ResourceManager *resource_manager, MIX_Mixer *mixer)
    : resource_manager_(resource_manager), mixer_(mixer) {
  if (!resource_manager_ || !mixer_) {
    throw std::runtime_error("AudioPlayer 构造失败: 空指针");
  }

  // 预创建 SFX track 池
  for (int i = 0; i < kMaxSfxChannels; ++i) {
    auto *track = MIX_CreateTrack(mixer_);
    if (!track) {
      spdlog::error("AudioPlayer: 创建 SFX track #{} 失败: {}", i,
                    SDL_GetError());
    }
    sfx_tracks_.push_back(track);
  }

  // 创建音乐 track
  music_track_ = MIX_CreateTrack(mixer_);
  if (!music_track_) {
    spdlog::error("AudioPlayer: 创建 Music track 失败: {}", SDL_GetError());
  }

  spdlog::trace("AudioPlayer 构造成功 ({} SFX tracks)", kMaxSfxChannels);
}

AudioPlayer::~AudioPlayer() {
  for (auto *t : sfx_tracks_) {
    if (t) MIX_DestroyTrack(t);
  }
  sfx_tracks_.clear();
  if (music_track_) MIX_DestroyTrack(music_track_);
  spdlog::trace("AudioPlayer 析构完成");
}

int AudioPlayer::findFreeSfxTrack() {
  for (int i = 0; i < static_cast<int>(sfx_tracks_.size()); ++i) {
    if (sfx_tracks_[i] && !MIX_TrackPlaying(sfx_tracks_[i])) {
      return i;
    }
  }
  // 全忙 —— 覆盖第一个（最旧的行为模拟）
  if (!sfx_tracks_.empty()) {
    spdlog::trace("AudioPlayer: SFX track 全忙，覆盖 track 0");
    return 0;
  }
  return -1;
}

int AudioPlayer::playSound(const std::string &sound_path, int channel) {
  MIX_Audio *audio = resource_manager_->getShortAudio(sound_path);
  if (!audio) {
    spdlog::error("AudioPlayer: 获取音效 '{}' 失败", sound_path);
    return -1;
  }

  int idx = (channel >= 0 && channel < static_cast<int>(sfx_tracks_.size()))
                ? channel
                : findFreeSfxTrack();
  if (idx < 0 || !sfx_tracks_[idx]) return -1;

  MIX_Track *track = sfx_tracks_[idx];
  if (!MIX_SetTrackAudio(track, audio)) {
    spdlog::error("AudioPlayer: 设置音效 '{}' 到 track #{} 失败: {}", sound_path,
                  idx, SDL_GetError());
    return -1;
  }

  MIX_SetTrackGain(track, sfx_gain_);

  SDL_PropertiesID prop = SDL_CreateProperties();
  SDL_SetNumberProperty(prop, MIX_PROP_PLAY_LOOPS_NUMBER, 0);
  bool ok = MIX_PlayTrack(track, prop);
  SDL_DestroyProperties(prop);

  if (!ok) {
    spdlog::error("AudioPlayer: 播放音效 '{}' 失败: {}", sound_path,
                  SDL_GetError());
    return -1;
  }
  return idx;
}

bool AudioPlayer::playMusic(const std::string &music_path, int loops,
                            int fade_in_ms) {
  if (music_path == current_music_) return true;

  MIX_Audio *audio = resource_manager_->getLongAudio(music_path);
  if (!audio) {
    spdlog::error("AudioPlayer: 获取音乐 '{}' 失败", music_path);
    return false;
  }
  current_music_ = music_path;

  if (!MIX_SetTrackAudio(music_track_, audio)) {
    spdlog::error("AudioPlayer: 设置音乐 '{}' 到 music track 失败: {}",
                  music_path, SDL_GetError());
    return false;
  }

  MIX_SetTrackGain(music_track_, music_gain_);

  SDL_PropertiesID prop = SDL_CreateProperties();
  SDL_SetNumberProperty(prop, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
  if (fade_in_ms > 0) {
    SDL_SetNumberProperty(prop, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER,
                          fade_in_ms);
  }
  bool ok = MIX_PlayTrack(music_track_, prop);
  SDL_DestroyProperties(prop);

  if (!ok) {
    spdlog::error("AudioPlayer: 播放音乐 '{}' 失败: {}", music_path,
                  SDL_GetError());
    return false;
  }
  return true;
}

void AudioPlayer::stopMusic(int fade_out_ms) {
  if (!music_track_) return;
  MIX_StopTrack(music_track_, fade_out_ms);
  current_music_.clear();
}

void AudioPlayer::pauseMusic() {
  if (music_track_) MIX_PauseTrack(music_track_);
}

void AudioPlayer::resumeMusic() {
  if (music_track_) MIX_ResumeTrack(music_track_);
}

void AudioPlayer::setSoundVolume(float volume) {
  sfx_gain_ = glm::clamp(volume, 0.0f, 1.0f);
  for (auto *t : sfx_tracks_) {
    if (t) MIX_SetTrackGain(t, sfx_gain_);
  }
}

void AudioPlayer::setMusicVolume(float volume) {
  music_gain_ = glm::clamp(volume, 0.0f, 1.0f);
  if (music_track_) MIX_SetTrackGain(music_track_, music_gain_);
}

float AudioPlayer::getSoundVolume() const { return sfx_gain_; }

float AudioPlayer::getMusicVolume() const { return music_gain_; }

} // namespace engine::audio