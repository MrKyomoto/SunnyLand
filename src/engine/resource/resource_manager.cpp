#include "resource_manager.h"
#include "font_manager.h"
#include "audio_manager.h"
#include "texture_manager.h"
#include <SDL3/SDL_render.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

namespace engine::resource
{
    ResourceManager::~ResourceManager() = default;

    ResourceManager::ResourceManager(SDL_Renderer *renderer)
    {
        texture_manager_ = std::make_unique<TextureManager>(renderer);
        audio_manager_ = std::make_unique<AudioManager>();
        font_manager_ = std::make_unique<FontManager>();

        spdlog::trace("ResourceManager 构造成功");
        // RAII: 构造成功即代表资源管理器可以正常工作,无需再初始化,无需检查指针是否为空(听不懂先记一下)
    }

    void ResourceManager::clear()
    {
        font_manager_->clearFonts();
        audio_manager_->clearAudios();
        texture_manager_->clearTextures();
        spdlog::trace("ResourceManager 中的资源通过clear()清空");
    }

    SDL_Texture *ResourceManager::loadTexture(const string &file_path)
    {
        // 构造函数已经确保了 texture_manager_ 不为空,因此不再需要进行if检查, 以免性能浪费
        return texture_manager_->loadTexture(file_path);
    }

    SDL_Texture *ResourceManager::getTexture(const string &file_path)
    {
        return texture_manager_->getTexture(file_path);
    }

    glm::vec2 ResourceManager::getTextureSize(const string &file_path)
    {
        return texture_manager_->getTextureSize(file_path);
    }

    void ResourceManager::unloadTexture(const string &file_path)
    {
        texture_manager_->unloadTexture(file_path);
    }

    void ResourceManager::clearTextures()
    {
        texture_manager_->clearTextures();
    }

    MIX_Audio *ResourceManager::loadLongAudio(const string &file_path)
    {
        return audio_manager_->loadLongAudio(file_path);
    }

    MIX_Audio *ResourceManager::loadShortAudio(const string &file_path)
    {
        return audio_manager_->loadShortAudio(file_path);
    }
    MIX_Audio *ResourceManager::getLongAudio(const string &file_path)
    {
        return audio_manager_->getLongAudio(file_path);
    }
    MIX_Audio *ResourceManager::getShortAudio(const string &file_path)
    {
        return audio_manager_->getShortAudio(file_path);
    }
    void ResourceManager::unloadLongAudio(const string &file_path)
    {
        audio_manager_->unloadLongAudio(file_path);
    }
    void ResourceManager::unloadShortAudio(const string &file_path)
    {
        audio_manager_->unloadShortAudio(file_path);
    }
    void ResourceManager::clearLongAudios()
    {
        audio_manager_->clearLongAudios();
    }
    void ResourceManager::clearShortAudios()
    {
        audio_manager_->clearShortAudios();
    }
    void ResourceManager::clearAudios()
    {
        audio_manager_->clearAudios();
    }
    TTF_Font *ResourceManager::loadFont(const string &file_path, int point_size)
    {
        return font_manager_->loadFont(file_path, point_size);
    }
    TTF_Font *ResourceManager::getFont(const string &file_path, int point_size)
    {
        return font_manager_->getFont(file_path, point_size);
    }
    void ResourceManager::unloadFont(const string &file_path, int point_size)
    {
        font_manager_->unloadFont(file_path, point_size);
    }
    void ResourceManager::clearFonts()
    {
        font_manager_->clearFonts();
    }
} // namespace engine::resource