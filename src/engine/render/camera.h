#pragma once
#include <optional>
#include "../utils/math.h"

namespace engine::render
{
    /**
     * @brief 相机类负责管理相机位置和视口大小,并提供坐标转换功能
     * 它还包含限制相机移动范围的边界
     */
    class Camera final
    {
    private:
        /// @brief 视口大小(屏幕大小)
        glm::vec2 viewport_size_;
        glm::vec2 position_;
        /// @brief 限制相机的移动范围,空值表示不限制
        std::optional<engine::utils::Rect> limit_bounds_;

    public:
        explicit Camera(const glm::vec2 &viewport_size, const glm::vec2 &position = glm::vec2(0.0f, 0.0f), const std::optional<engine::utils::Rect> limit_bounds = std::nullopt);

        void update(float delta_time);
        void move(const glm::vec2 &offset);

        glm::vec2 worldToScreen(const glm::vec2 &world_pos) const;
        /// @brief 带有视差滚动的坐标转换
        glm::vec2 worldToScreenWithParallax(const glm::vec2 &world_pos, const glm::vec2 &scroll_factor) const;
        glm::vec2 screenToWorld(const glm::vec2 &screen_pos) const;

        void setPosition(const glm::vec2 &position);
        void setLimitBounds(const engine::utils::Rect &bounds);

        const glm::vec2 &getPosition() const;
        std::optional<engine::utils::Rect> getLimitBounds() const;
        glm::vec2 getViewPortSize() const;

        Camera(const Camera &) = delete;
        Camera &operator=(const Camera &) = delete;
        Camera(const Camera &&) = delete;
        Camera &operator=(const Camera &&) = delete;

    private:
        void clampPosition(); // 限制相机位置在边界内
    };

}