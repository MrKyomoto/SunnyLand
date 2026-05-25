#include "audio_manager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

AudioManager::AudioManager() {
  if (!MIX_Init()) {
    throw std::runtime_error("AudioManager 构造失败: MIX_Init 失败" +
                             string(SDL_GetError()));
  }

  mixer_ = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
  if (!mixer_) {
    MIX_Quit(); // 如果创建mixer失败,先清理Mix_Init()再抛出异常
    throw std::runtime_error("AudioManager 构造失败: Mixer指针为空" +
                             string(SDL_GetError()));
  }

  spdlog::trace("AudioManager 构造成功");
}
AudioManager::~AudioManager() {
  clearAudios();

  MIX_Quit();
  spdlog::trace("AudioManager 析构成功");
}
MIX_Audio *AudioManager::loadLongAudio(const string &file_path) {
  auto it = long_audios_.find(file_path);
  if (it != long_audios_.end()) {
    return it->second.get();
  }

  spdlog::debug("加载长音效: {}", file_path);
  MIX_Audio *longAudio = MIX_LoadAudio(mixer_, file_path.c_str(), false);
  if (!longAudio) {
    spdlog::error("加载长音效失败: '{}' : {}", file_path, SDL_GetError());
    return nullptr;
  }

  long_audios_.emplace(
      file_path, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>(longAudio));
  spdlog::debug("成功加载并缓存长音效: {}", file_path);
  return longAudio;
}
MIX_Audio *AudioManager::loadShortAudio(const string &file_path) {
  auto it = short_audios_.find(file_path);
  if (it != short_audios_.end()) {
    return it->second.get();
  }

  spdlog::debug("加载短音效: {}", file_path);
  MIX_Audio *shortAudio = MIX_LoadAudio(mixer_, file_path.c_str(), true);
  if (!shortAudio) {
    spdlog::error("加载短音效失败: '{}' : {}", file_path, SDL_GetError());
    return nullptr;
  }

  short_audios_.emplace(
      file_path, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>(shortAudio));
  spdlog::debug("成功加载并缓存短音效: {}", file_path);
  return shortAudio;
}
MIX_Audio *AudioManager::getLongAudio(const string &file_path) {
  auto it = long_audios_.find(file_path);
  if (it != long_audios_.end()) {
    return it->second.get();
  }

  spdlog::warn("音效 '{}' 未找到缓存,尝试加载", file_path);
  return loadLongAudio(file_path);
}

MIX_Audio *AudioManager::getShortAudio(const string &file_path) {
  auto it = short_audios_.find(file_path);
  if (it != short_audios_.end()) {
    return it->second.get();
  }

  spdlog::warn("音效 '{}' 未找到缓存,尝试加载", file_path);
  return loadShortAudio(file_path);
}
void AudioManager::unloadLongAudio(const string &file_path) {
  auto it = long_audios_.find(file_path);
  if (it != long_audios_.end()) {
    spdlog::debug("卸载长音效: {}", file_path);
    long_audios_.erase(it);
  } else {
    spdlog::warn("尝试卸载不存在的长音效: {}", file_path);
  }
}
void AudioManager::unloadShortAudio(const string &file_path) {
  auto it = short_audios_.find(file_path);
  if (it != short_audios_.end()) {
    spdlog::debug("卸载短音效: {}", file_path);
    short_audios_.erase(it);
  } else {
    spdlog::warn("尝试卸载不存在的短音效: {}", file_path);
  }
}
void AudioManager::clearLongAudios() {
  if (!long_audios_.empty()) {
    spdlog::debug("正载清除所有缓存的音效, 数量: {}", long_audios_.size());
    long_audios_.clear();
  }
}
void AudioManager::clearShortAudios() {
  if (!short_audios_.empty()) {
    spdlog::debug("正载清除所有缓存的音效, 数量: {}", short_audios_.size());
    short_audios_.clear();
  }
}
void AudioManager::clearAudios() {
  clearLongAudios();
  clearShortAudios();
}
} // namespace engine::resource
