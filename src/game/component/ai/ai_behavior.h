#pragma once

namespace game::component {
class AIComponent;
}

namespace game::component::ai {
class AIBehavior {
  friend class game::component::AIComponent;

public:
  AIBehavior() = default;
  virtual ~AIBehavior() = default;

  AIBehavior(const AIBehavior &) = delete;
  AIBehavior &operator=(const AIBehavior &) = delete;
  AIBehavior &operator=(const AIBehavior &&) = delete;
  AIBehavior(const AIBehavior &&) = delete;

protected:
  // 没有保存owner指针,因此需要传入
  // AIComponent引用,我的思考是这里老师向展示更多中不同的写法所以这么写了,实际上在AIBehavior里保存AIComponent指针完全是可以的,在构造的时候传入即可
  // 优势：
  // 降低耦合：行为不持有组件指针
  // 灵活性高：同一个行为实例可用于多个组件
  // 生命周期清晰：无需管理指针有效性
  virtual void enter(AIComponent &) {} ///< @brief enter可选是否实现,默认为空
  virtual void update(float, AIComponent &) = 0; // 具体AI行为逻辑,必须实现
};
/**
enter() 的典型用途：
┌──────────────────────┐
│ 飞行行为 enter():    │
│  - 关闭重力          │
│  - 播放飞行动画      │
├──────────────────────┤
│ 巡逻行为 enter():    │
│  - 开启重力          │
│  - 播放走路动画      │
├──────────────────────┤
│ 攻击行为 enter():    │
│  - 播放攻击音效      │
│  - 设置攻击计时器    │
└──────────────────────┘
*/

} // namespace game::component::ai