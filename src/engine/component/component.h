#pragma once
namespace engine::object
{
    class GameObject;
} // namespace engine::object

namespace engine::core
{
    class Context;
} // namespace engine::core

namespace engine::component
{
    /**
     * @brief 组件的抽象基类
     *
     * 所有具体组件都应从此类继承
     * 定义了组件生命周期中可能调用的通用方法
     */
    class Component
    {
        friend class engine::object::GameObject; // GameObject需要调用component的init方法

    protected:
        engine::object::GameObject *owner_ = nullptr;

    public:
        Component() = default;
        /// @brief 析构函数确保正确清理派生类
        virtual ~Component() = default;

        Component(const Component &) = delete;
        Component &operator=(const Component &) = delete;
        Component(const Component &&) = delete;
        Component &operator=(const Component &&) = delete;

        void setOwner(engine::object::GameObject *owner) { owner_ = owner; }
        engine::object::GameObject *getOwner() const { return owner_; }

    protected:
        // 关键循环函数全部设为保护,只有GameObject需要(可以)调用
        /// @brief 保留两段初始化的机制,GameObject添加组件时自动调用,不需要外部调用
        virtual void init() {}
        virtual void handleInput(engine::core::Context &) {}
        virtual void update(float, engine::core::Context &) = 0; // 必须实现
        virtual void render(engine::core::Context &) {}
        virtual void clean() {}
    };

} // namespace engine::component
