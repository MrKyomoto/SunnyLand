namespace engine::render
{
    class Renderer;
    class Camera;
} // namespace engine::render
namespace engine::resource
{
    class ResourceManager;
} // namespace engine::resource
namespace engine::input
{
    class InputManager;
} // namespace engine::input

namespace engine::core
{
    /**
     * @brief 持有对核心引擎模块引用的上下文对象
     *
     * 用于简化依赖注入,传递Context即可获取引擎的各个模块
     */
    class Context final
    {
    private:
        // 使用引用,确保每个模块都有效,使用时不需要检查指针是否为空
        engine::input::InputManager &input_manager_;
        engine::render::Renderer &renderer_;
        engine::render::Camera &camera_;
        engine::resource::ResourceManager &resource_manager_;

    public:
        Context(engine::input::InputManager &input_manager,
                engine::render::Renderer &renderer,
                engine::render::Camera &camera,
                engine::resource::ResourceManager &resource_manager);

        Context(const Context &) = delete;
        Context &operator=(const Context &) = delete;
        Context(const Context &&) = delete;
        Context &operator=(const Context &&) = delete;

        // --- Getters ---
        engine::input::InputManager &getInputManager() const { return input_manager_; };
        engine::render::Renderer &getRenderer() const { return renderer_; };
        engine::render::Camera &getCamera() const { return camera_; };
        engine::resource::ResourceManager &getResourceManager() const
        {
            return resource_manager_;
        }
    };

} // namespace engine::core
