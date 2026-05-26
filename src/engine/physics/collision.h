#pragma once
#include "../utils/math.h"

namespace engine::component {
class ColliderComponent;
}

namespace engine::physics::collision {

bool checkCollision(const engine::component::ColliderComponent &a,
                    const engine::component::ColliderComponent &b);

bool checkCircleOverlap(const glm::vec2 &a_center, const float a_radius,
                        const glm::vec2 &b_center, const float b_radius);
bool checkAABBOverlap(const glm::vec2 &a_pos, const glm::vec2 &a_size,
                      const glm::vec2 b_pos, const glm::vec2 &b_size);
bool checkRectOverlap(const engine::utils::Rect &a,
                      const engine::utils::Rect &b);
bool checkPointInCircle(const glm::vec2 &point, const glm::vec2 &center,
                        const float radius);
} // namespace engine::physics::collision