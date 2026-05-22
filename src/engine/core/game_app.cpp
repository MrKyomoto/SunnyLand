#include "game_app.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

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
        while (is_running_)
        {
            float delta_time = 0.01f;
            handleEvents();
            update(delta_time);
            render();
        }

        close();
    }

    bool GameApp::init()
    {
        spdlog::trace("Init GameApp...");
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

        is_running_ = true;
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

} // engine::core