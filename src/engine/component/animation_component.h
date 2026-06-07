#pragma once
#include "./component.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace engine::render {
class Animation;
}

namespace engine::component {
class SpriteComponent;
}

namespace engine::component {
using std::string;
using std::unordered_map;

class AnimationComponent : public Component {
  friend class engine::object::GameObject;

private:
  unordered_map<string, std::unique_ptr<engine::render::Animation>> animations_;
  SpriteComponent *sprite_component_ = nullptr;
  engine::render::Animation *current_animation_ = nullptr;

  float animation_timer_ = 0.0f;
  bool is_playing_ = false;          // 当前是否有动画正载播放
  bool is_one_shot_removal_ = false; // 是否在动画结束后删除整个GameObject

public:
  AnimationComponent() = default;
  ~AnimationComponent() override;

  AnimationComponent(const AnimationComponent &) = delete;
  AnimationComponent &operator=(const AnimationComponent &) = delete;
  AnimationComponent(const AnimationComponent &&) = delete;
  AnimationComponent &operator=(const AnimationComponent &&) = delete;

  void addAnimation(std::unique_ptr<engine::render::Animation> animation);
  void playAnimation(const std::string &name);
  void stopAnimation() { is_playing_ = false; }

  string getCurrentAnimationName() const;
  bool isPlaying() const { return is_playing_; }
  bool isAnimationFinished() const;
  bool isOneShotRemoval() const { return is_one_shot_removal_; }
  void setOneShotRemoval(bool is_one_shot_removal) {
    is_one_shot_removal_ = is_one_shot_removal;
  }

protected:
  void init() override;
  void update(float,engine::core::Context&) override;
};

} // namespace engine::component