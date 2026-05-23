#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>

extern "C"
{
    struct SDL_Renderer;
    struct SDL_Texture;
    struct MIX_Audio;
    struct TTF_Font;
}

namespace engine::resource
{

    using std::string;
    // 前向声明内部管理器
    class TextureManager;
    class AudioManager;
    class FontManager;

    /// @brief 作为访问各种资源管理器的中央控制点(外观模式 Facade), 构造时初始化其管理的子系统,构造失败会抛出异常
    class ResourceManager
    {
    private:
        std::unique_ptr<TextureManager> texture_manager_;
        std::unique_ptr<AudioManager> audio_manager_;
        std::unique_ptr<FontManager> font_manager_;

    public:
        /**
         * @brief 构造函数,执行初始化
         * @param renderer SDL_Renderer的指针,传递给需要它的子管理器,不能为空
         * @param mixer MIX_Mixer
         */
        explicit ResourceManager(SDL_Renderer *renderer); // explicit 关机案子用于防止隐式转换,对于单一参数的构造函数,通常考虑添加
        ~ResourceManager();                               // 显式声明析构函数,为了能让智能指针正确管理仅有前向声明的类, 老师的原话是如果这个地方不显式声明,那么编译器在编译阶段会内联一个析构函数, 而由于我们这里是前向声明子类,所以内联的结果是不对的,实际上我们在texture_manager.cpp里 #include了真正的子类,虽然在cpp里只是 ~() = default但就已经存在真正的子类了

        /// @brief 清空所有资源
        void clear();

        // 当前设计中只需要一个ResourceManager,所有权不变,所以不需要拷贝,直接删除这些就好
        ResourceManager(const ResourceManager &) = delete;
        ResourceManager &operator=(const ResourceManager &) = delete;
        ResourceManager(const ResourceManager &&) = delete;
        ResourceManager &operator=(const ResourceManager &&) = delete;

        // --- 统一资源访问接口 ---
        // -- Texture --
        SDL_Texture *loadTexture(const string &file_path);
        /// @brief 尝试获取已加载纹理的指针,如果未加载则尝试加载
        SDL_Texture *getTexture(const string &file_path);
        void unloadTexture(const string &file_path);
        glm::vec2 getTextureSize(const string &file_path);
        /// @brief 清空所有纹理资源
        void clearTextures();

        // -- Audio --
        MIX_Audio *loadLongAudio(const string &file_path);
        MIX_Audio *loadShortAudio(const string &file_path);
        /// @brief 尝试获取已加载Audio的指针,如果未加载则尝试加载
        MIX_Audio *getLongAudio(const string &file_path);
        MIX_Audio *getShortAudio(const string &file_path);
        void unloadLongAudio(const string &file_path);
        void unloadShortAudio(const string &file_path);
        void clearLongAudios();
        void clearShortAudios();
        /// @brief 清空所有Audio资源
        void clearAudios();

        // -- Fonts --
        TTF_Font *loadFont(const string &file_path, int point_size);
        /// @brief 尝试获取已加载Font的指针,如果未加载则尝试加载
        TTF_Font *getFont(const string &file_path, int point_size);
        void unloadFont(const string &file_path, int point_size);
        /// @brief 清空所有Font资源
        void clearFonts();
    };

}