#pragma once
#include "../component/component.h"
#include <string>
#include <unordered_map>

namespace engine::audio {
class AudioPlayer;
}

namespace engine::render {
class Camera;
}

namespace engine::component {

class TransformComponent;

class AudioComponent final : public Component {
  friend class engine::object::GameObject;

private:
  engine::audio::AudioPlayer *audio_player_ = nullptr;
  engine::render::Camera *camera_ = nullptr;
  engine::component::TransformComponent *transform_ = nullptr;

  std::unordered_map<std::string, std::string> sound_id_to_path_;

  float hearing_radius_ = 500.0f;

public:
  AudioComponent(engine::audio::AudioPlayer *audio_player,
                 engine::render::Camera *camera,
                 float hearing_radius = 500.0f);

  void addSound(const std::string &sound_id, const std::string &sound_path);

  void playSound(const std::string &sound_id, int channel = -1,
                 bool use_spatial = false);

private:
  void init() override;
  void update(float, engine::core::Context &) override {}
};

} // namespace engine::component