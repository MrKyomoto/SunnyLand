#pragma once
#include "../component/component.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <typeindex> // 用于类型索引
#include <unordered_map>
#include <utility> // 用于完美转发

namespace engine::core {
class Context;
} // namespace engine::core

namespace engine::object {

/**
 * @brief 游戏对象类,负责管理游戏对象的组件
 *
 * 该类管理游戏对象的组件,并提供添加,获取,检查和移除组件的功能
 * 它还提供更新和渲染游戏对象的方法
 */
class GameObject final {
private:
  std::string name_;
  std::string tag_;
  std::unordered_map<std::type_index,
                     std::unique_ptr<engine::component::Component>>
      components_;
  bool need_remove_ = false; ///< @brief 延迟删除的标识,将来由场景类负责删除

public:
  GameObject(const std::string &name = "", const std::string &tag = "");

  GameObject(const GameObject &) = delete;
  GameObject &operator=(const GameObject &) = delete;
  GameObject(const GameObject &&) = delete;
  GameObject &operator=(const GameObject &&) = delete;

  void setName(const std::string &name) { name_ = name; }
  const std::string &getName() const { return name_; }
  void setTag(const std::string &tag) { tag_ = tag; }
  const std::string &getTag() const { return tag_; }
  void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }
  bool isNeedRemove() const { return need_remove_; }

  /// @brief
  /// @tparam T 组件类型
  /// @return 组件裸指针
  template <typename T, typename... Args> T *addComponent(Args &&...args) {
    // 检测组件是否合法,
    // static_assert(condition,message):静态断言,在编译期检测,无任何性能影响,
    // std::is_base_of<Base,Derived>::value 判断Base类型是否是Derived类型的基类
    static_assert(std::is_base_of<engine::component::Component, T>::value,
                  "T 必须继承自 Component");

    // 获取类型标识,
    // typeid(T)用于获取一个表达式或类型的运行时类型信息(RTTI),返回std::type_info&
    // std::type_index
    // 针对std::type_info对象的包装器,主要设计用来作为关联容器的键
    auto type_index = std::type_index(typeid(T));
    if (hasComponent<T>()) {
      return getComponent<T>();
    }

    // std::forward用于实现完美转发,传递多个参数的时候使用...标识,
    // 和前边Args&&... args配合使用,&时是左值引用,&&时就是右值引用,
    // 简单来说用这个是效率最高的
    auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
    T *ptr = new_component.get(); // 先获取裸指针以便于返回
    new_component->setOwner(this);
    components_[type_index] =
        std::move(new_component); // 移动组件,此时new_component为空不可再使用
    ptr->init();                  // 必须使用ptr而不是new_component
    spdlog::debug("GameObject::addComponent: {} added component {}", name_,
                  typeid(T).name());
    return ptr; // 返回非拥有指针
  }

  /// @brief 获取组件
  /// @tparam T 组件类型
  /// @return 组件裸指针
  template <typename T> T *getComponent() const {
    static_assert(std::is_base_of<engine::component::Component, T>::value,
                  "T 必须继承自 Component");

    auto type_index = std::type_index(typeid(T));
    auto it = components_.find(type_index);
    if (it != components_.end()) {
      return static_cast<T *>(it->second.get());
    }
    return nullptr;
  }

  /// @brief 检查是否存在组件
  /// @tparam T 组件类型
  /// @return 是否存在组件
  template <typename T> bool hasComponent() const {
    static_assert(std::is_base_of<engine::component::Component, T>::value,
                  "T 必须继承自 Component");

    auto type_index = std::type_index(typeid(T));
    return components_.contains(type_index);
  }

  /// @brief 移除组件
  /// @tparam T 组件类型
  template <typename T> void removeComponent() {
    static_assert(std::is_base_of<engine::component::Component, T>::value,
                  "T 必须继承自 Component");

    auto type_index = std::type_index(typeid(T));
    auto it = components_.find(type_index);
    if (it != components_.end()) {
      it->second->clean();
      components_.erase(it);
    }
  }

  void update(float delta_time, engine::core::Context &context);
  void render(engine::core::Context &context);
  void clean();
  void handleInput(engine::core::Context &context);
};

} // namespace engine::object
