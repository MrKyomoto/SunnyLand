#include "audio_component.h"
#include "../audio/audio_player.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "transform_component.h"
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <spdlog/spdlog.h>

namespace engine::component {

AudioComponent::AudioComponent(engine::audio::AudioPlayer *audio_player,
                               engine::render::Camera *camera,
                               float hearing_radius)
    : audio_player_(audio_player), camera_(camera),
      hearing_radius_(hearing_radius) {
  if (!audio_player_) {
    spdlog::error("AudioComponent 构造: AudioPlayer 为空");
  }
}

void AudioComponent::init() {
  if (owner_) {
    transform_ = owner_->getComponent<engine::component::TransformComponent>();
  }
}

void AudioComponent::addSound(const std::string &sound_id,
                              const std::string &sound_path) {
  sound_id_to_path_[sound_id] = sound_path;
}

void AudioComponent::playSound(const std::string &sound_id, int channel,
                               bool use_spatial) {
  if (!audio_player_) return;

  auto it = sound_id_to_path_.find(sound_id);
  if (it == sound_id_to_path_.end()) {
    spdlog::warn("AudioComponent: 未知音效 ID '{}'", sound_id);
    return;
  }

  if (use_spatial) {
    if (!camera_ || !transform_) return;
    auto dist = glm::distance(transform_->getPosition(),
                              camera_->getPosition());
    if (dist > hearing_radius_) return;
  }

  audio_player_->playSound(it->second, channel);
}

} // namespace engine::component