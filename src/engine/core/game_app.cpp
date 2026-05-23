#include "game_app.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include "time.h"
#include "../resource/resource_manager.h"

namespace engine::core
{
    GameApp::GameApp() = default;

    GameApp::~GameApp()
    {
        if (is_running_)
        {
            spdlog::warn("GameApp 被销毁时没有显式关闭,现在关闭...");
            close();
        }
    }

    void GameApp::run()
    {
        if (!init())
        {
            spdlog::error("GameApp Init Failed, cannot run the game");
            return;
        }
        time_->setTargetFps(144); // 将来会从配置文件读取

        while (is_running_)
        {
            time_->update();
            float delta_time = time_->getDeltaTime();

            handleEvents();
            update(delta_time);
            render();

            // spdlog::info("delta_time: {}", delta_time);
        }

        close();
    }

    bool GameApp::init()
    {
        spdlog::trace("Init GameApp...");

        if (!initSDL())
            return false;
        if (!initTime())
            return false;
        if (!initResourceManager())
            return false;

        testResourceManager();

        is_running_ = true;
        spdlog::trace("Init GameApp success");
        return true;
    }
    void GameApp::handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                is_running_ = false;
            }
        }
    }

    void GameApp::update(float delta_time)
    {
    }

    void GameApp::render()
    {
    }

    void GameApp::close()
    {
        spdlog::trace("关闭 GameApp...");

        if (sdl_renderer_ != nullptr)
        {
            SDL_DestroyRenderer(sdl_renderer_);
            sdl_renderer_ = nullptr;
        }

        if (window_ != nullptr)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        is_running_ = false;
    }

    bool GameApp::initSDL()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            spdlog::error("SDL_Init failed, SDL Error: {}", SDL_GetError());
            return false;
        }

        window_ = SDL_CreateWindow("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE);

        if (window_ == nullptr)
        {
            spdlog::error("SDL_CreateWindow failed, SDL Error: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);

        if (sdl_renderer_ == nullptr)
        {
            spdlog::error("SDL_CreateRenderer failed, SDL Error: {}", SDL_GetError());
            return false;
        }

        spdlog::trace("SDL初始化成功");
        return true;
    }

    bool GameApp::initTime()
    {
        try
        {
            time_ = std::make_unique<Time>();
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化时间管理失败: {}", e.what());
            return false;
        }
        spdlog::trace("时间管理初始化成功");
        return true;
    }

    bool GameApp::initResourceManager()
    {
        try
        {
            resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化资源管理失败: {}", e.what());
            return false;
        }
        spdlog::trace("资源管理初始化成功");
        return true;
    }

    void GameApp::testResourceManager()
    {
        resource_manager_->getTexture("assets/textures/Actors/eagle-attack.png");
        resource_manager_->getFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
        resource_manager_->getShortAudio("assets/audio/button_click.wav");

        resource_manager_->unloadTexture("assets/textures/Actors/eagle-attack.png");
        resource_manager_->unloadFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
        resource_manager_->unloadShortAudio("assets/audio/button_click.wav");
    }

} // engine::core