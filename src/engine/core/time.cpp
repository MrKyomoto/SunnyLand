#include "time.h"
#include <SDL3/SDL_timer.h>
#include <spdlog/spdlog.h>


namespace engine::core {
Time::Time() {
  // 初始化 last_time_ && frame_start_time_ 为当前时间,避免第一帧 delta 差距过大
  last_time_ = SDL_GetTicksNS();
  frame_start_time_ = last_time_;
  spdlog::trace("Time init. Last time: {}", last_time_);
}

void Time::update() {
  frame_start_time_ = SDL_GetTicksNS();
  auto current_delta_time =
      static_cast<double>(frame_start_time_ - last_time_) / 1000000000.0;
  if (target_frame_time_ > 0.0) { // 如果设置了目标帧率,则限制帧率,否则delta_time_
                                  // = current_delta_time
    limitFrameRate(current_delta_time);
  } else {
    delta_time_ = current_delta_time;
  }

  // 记录离开 update 的时间戳
  last_time_ = SDL_GetTicksNS();
}

void Time::limitFrameRate(float current_delta_time) {
  if (current_delta_time < target_frame_time_) {
    double time_to_wait = target_frame_time_ - current_delta_time;
    Uint64 ns_to_wait = static_cast<Uint64>(time_to_wait * 1000000000.0);
    SDL_DelayNS(ns_to_wait);
    delta_time_ =
        static_cast<double>(SDL_GetTicksNS() - last_time_) / 1000000000.0;
  }
}

float Time::getDeltaTime() const { return delta_time_ * time_scale_; }

float Time::getUnscaledDeltaTime() const { return delta_time_; }

int Time::getTargetFps() const { return target_fps_; }

float Time::getTimeScale() const { return time_scale_; }

void Time::setTargetFps(int fps) { target_fps_ = fps; }

void Time::setTimeScale(float scale) { time_scale_ = scale; }
} // namespace engine::core