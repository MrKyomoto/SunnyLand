#pragma once
#include <SDL3/SDL_rect.h>
#include <string>
#include <vector>

namespace engine::render {
using std::string;
using std::vector;

struct AnimationFrame {
  SDL_FRect source_rect; ///< @brief 纹理图集上此帧的区域
  float duration;        ///< @brief 此帧显示的持续时间(s)
};

class Animation final {
private:
  string name_;
  vector<AnimationFrame> frames_;
  float total_duration_ = 0.0f;
  bool loop_ = true;

public:
  Animation(const string &name = "default", bool loop = true);
  ~Animation() = default;

  void addFrame(const SDL_FRect &source_rect, float duration);
  const AnimationFrame &getFrame(float time) const;

  const string &getName() const { return name_; }
  float getTotalDuration() const { return total_duration_; }
  bool isLooping() const { return loop_; }
  bool isEmpty() const { return frames_.empty(); }
};

} // namespace engine::render