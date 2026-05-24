#pragma once
#include <string>
#include "sprite.h"
#include <optional>
#include <glm/glm.hpp>

struct SDL_Renderer;
struct SDL_FRect;

namespace engine::resource
{
    class ResourceManager;
}

namespace engine::render
{
    class Camera;

    /**
     * @brief 封装SDL3渲染操作
     *
     * 包装SDL_Renderer并提供清除屏幕,绘制精灵和呈现最终图像的方法
     * 在构造时初始化,依赖于一个有效的SDL_Renderer和ResourceManager
     * 构造失败会抛出异常
     */
    class Renderer final
    {
    private:
        // 这里直接使用裸指针是因为这两个都是外部传入的,实际上是GameApp里管理的生命周期,所以这里不需要管理
        SDL_Renderer *renderer_ = nullptr;
        engine::resource::ResourceManager *resource_manager_ = nullptr;

    public:
        /// @brief 构造函数
        /// @param sdl_renderer cant be null
        /// @param resource_manager cant be null
        /// @throw sdtd::runtime_error
        Renderer(SDL_Renderer *sdl_renderer, engine::resource::ResourceManager *resource_manager);

        /// @brief 绘制一个精灵
        /// @param sprite 包含纹理ID,源矩形和反转状态的sprite对象
        /// @param position 世界坐标中左上角的位置
        /// @param scale 缩放因子
        /// @param angle 旋转角度(度)
        void drawSprite(const Camera &camera, const Sprite &sprite, const glm::vec2 &position, const glm::vec2 &scale = {1.0f, 1.0f}, double angle = 0.0f);

        /// @brief 绘制视差滚动背景
        /// @param scroll_factor 滚动因子
        /// @param position 世界坐标中左上角的位置
        /// @param scale 缩放因子
        void drawParallax(const Camera &camera, const Sprite &sprite, const glm::vec2 &position, const glm::vec2 &scroll_factor, const glm::bvec2 &repeat = {true, true}, const glm::vec2 &scale = {1.0f, 1.0f});

        /// @brief 在屏幕坐标中直接渲染一个用于UI的Sprite对象
        /// @param position 屏幕坐标左上角的位置
        /// @param size 可选: 目标矩形的大小,如果为nullopt则使用Sprite的原始大小
        void drawUISprite(const Sprite &sprite, const glm::vec2 &position, const std::optional<glm::vec2> &size = std::nullopt);

        void present();
        void clearScreen();

        void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
        void setDrawColorFloat(float r, float g, float b, float a = 1.0f);

        SDL_Renderer *getSDLRenderer() const { return renderer_; }

        Renderer(const Renderer &) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(const Renderer &&) = delete;
        Renderer& operator=(const Renderer &&) = delete;

        private:
            std::optional<SDL_FRect> getSpriteSrcRect(const Sprite &sprite);
            bool isRectInViewport(const Camera &camera, const SDL_FRect &rect);
    };

}
