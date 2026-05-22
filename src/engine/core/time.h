#pragma once
#include <SDL3/SDL.h>

namespace engine::core
{
    /**
     * @brief 管理游戏循环中的时间,计算帧间时间差 Δt
     *
     * 使用SDL的高精度性能计数器确保时间测量的准确性
     * 提供获取缩放和未缩放Δt的方法,以及设置时间缩放因子的能力
     */
    class Time
    {
    private:
        Uint64 last_time_ = 0;        ///< @brief 上一帧的时间戳(用于计算Δ)
        Uint64 frame_start_time_ = 0; ///< @brief 当前帧开始的时间戳(用于帧率限制)
        double delta_time_ = 0.0;     ///< @brief 未缩放的帧时间差(秒)
        double time_scale_ = 1.0;     ///< 时间缩放因子

        // 帧率限制相关
        int target_fps_ = 0;             ///< @brief 目标FPS (0表示无限制)
        double target_frame_time_ = 0.0; ///< @brief 目标每帧时间(秒)

    public:
        Time();

        /**
         * @brief 每帧开始时调用,更新内部时间状态并计算Δt
         */
        void update();

        /**
         * @brief 获取经过时间缩放调整后的帧间时间差Δt
         *
         * @return double 缩放后的Δt'(秒)
         */
        float getDeltaTime() const;

        /**
         * @brief 获取未经时间缩放调整的帧间时间差Δt
         *
         * @return double 未缩放的Δt(秒)
         */
        float getUnscaledDeltaTime() const;

        void setTimeScale(float scale);

        float getTimeScale() const;

        /**
         * @brief 设置FPS
         *
         * @param fps 目标每秒帧数,设置为0表示不限制帧率,负值视为0
         */
        void setTargetFps(int fps);

        int getTargetFps() const;

    private:
        /**
         * @brief update中调用,用于限制帧率,如果设置了target_fps_ > 0, 且当前帧执行时间小于目标帧时间,则会调用SDL_Delay
         *
         * @param current_delta_time 当前帧的执行时间(s)
         */
        void limitFrameRate(float current_delta_time);
    };

}