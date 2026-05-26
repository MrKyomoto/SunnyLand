#pragma once
#include <glm/vec2.hpp>

namespace engine::physics {
enum class ColliderType {
  NONE,
  AABB,
  CIRCLE,
  // ... i.e. Capsule, Polygon
};

/// @brief 碰撞器的抽象基类
/// 所有具体的碰撞器都应该继承此类
class Collider {
protected:
  // 覆盖 Collider 的最小包围盒尺寸
  glm::vec2 aabb_size_ = {0.0f, 0.0f};

public:
  virtual ~Collider() = default;
  virtual ColliderType getType() const = 0;

  void setAABBSize(const glm::vec2 &size) { aabb_size_ = size; }
  const glm::vec2 &getAABBSize() const { return aabb_size_; }
};

/// @brief 轴对齐包围盒(Axis-Aligned Bounding Box)碰撞器
class AABBCollider final : public Collider {
private:
  glm::vec2 size_ = {0.0f, 0.0f};

public:
  explicit AABBCollider(const glm::vec2 &size) : size_(size) {
    setAABBSize(size);
  }
  ~AABBCollider() override = default;

  ColliderType getType() const override { return ColliderType::AABB; }
  const glm::vec2 &getSize() const { return size_; }
  void setSize(const glm::vec2 &size) {
    size_ = size;
    setAABBSize(size);
  }
};

class CircleCollider final : public Collider {
private:
  float radius_ = 0.0f;

public:
  explicit CircleCollider(float radius) : radius_(radius) {
    setAABBSize(glm::vec2(radius * 2.0f, radius * 2.0f));
  }
  ~CircleCollider() override = default;

  ColliderType getType() const override { return ColliderType::CIRCLE; }
  float getRadius() const { return radius_; }
  void setRadius(float radius) {
    radius_ = radius;
    setAABBSize(glm::vec2(radius_ * 2.0f, radius_ * 2.0f));
  }
};

} // namespace engine::physics