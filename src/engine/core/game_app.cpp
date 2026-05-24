#include "game_app.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include "time.h"
#include "../resource/resource_manager.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "config.h"
#include "../input/input_manager.h"

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
            time_->update();
            float delta_time = time_->getDeltaTime();
            // 每帧先更新输入管理器
            input_manager_->update();

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

        if (!initConfig())
            return false;
        if (!initSDL())
            return false;
        if (!initTime())
            return false;
        if (!initResourceManager())
            return false;
        if (!initRenderer())
            return false;
        if (!initCamera())
            return false;
        if (!initInputManager())
            return false;

        testResourceManager();

        is_running_ = true;
        spdlog::trace("Init GameApp success");
        return true;
    }
    void GameApp::handleEvents()
    {
        if (input_manager_->shouldQuit())
        {
            spdlog::trace("GameApp 收到来自 InputManager 的退出请求");
            is_running_ = false;
            return;
        }

        testInputManager();
    }

    void GameApp::update(float delta_time)
    {
        camera_->update(delta_time);
        testCamera();
    }

    void GameApp::render()
    {
        renderer_->clearScreen();
        testRenderer();
        renderer_->present();
    }

    void GameApp::close()
    {
        spdlog::trace("关闭 GameApp...");

        resource_manager_.reset();

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

    bool GameApp::initConfig()
    {
        try
        {
            config_ = std::make_unique<engine::core::Config>("assets/config.json");
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化Config失败: {}", e.what());
        }
        spdlog::trace("Config初始化成功");
        return true;
    }

    bool GameApp::initSDL()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        {
            spdlog::error("SDL_Init failed, SDL Error: {}", SDL_GetError());
            return false;
        }

        window_ = SDL_CreateWindow("SunnyLand", config_->window_width_, config_->window_height_, SDL_WINDOW_RESIZABLE);

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

        // 设置 VSync (开启时,驱动程序会尝试将帧率限制到显示器刷新率,有可能会覆盖我们手动设置的target_fps)
        int vsync_mode = config_->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
        SDL_SetRenderVSync(sdl_renderer_, vsync_mode);
        spdlog::trace("VSync 设置为: {}", config_->vsync_enabled_ ? "Enabled" : "Disabled");

        // 设置逻辑分辨率,视口大小应该与这个一致(针对像素游戏,逻辑分辨率设置为窗口大小的一半, 设成一半的原因是内部只渲染1/4的像素量,计算量小,同时每个原始像素对应4个屏幕像素,像素边缘锐利,不会出现半像素模糊,锯齿发虚等问题)
        SDL_SetRenderLogicalPresentation(sdl_renderer_, config_->window_width_ / 2, config_->window_height_ / 2, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
        time_->setTargetFps(config_->target_fps_);
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

    bool GameApp::initRenderer()
    {
        try
        {
            renderer_ = std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化渲染器失败: {}", e.what());
            return false;
        }
        spdlog::trace("渲染器初始化成功");
        return true;
    }

    bool GameApp::initCamera()
    {
        try
        {
            camera_ = std::make_unique<engine::render::Camera>(glm::vec2(config_->window_width_ / 2, config_->window_height_ / 2));
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化Camera失败: {}", e.what());
            return false;
        }
        spdlog::trace("Camera初始化成功");
        return true;
    }

    bool GameApp::initInputManager()
    {
        try
        {
            input_manager_ = std::make_unique<engine::input::InputManager>(sdl_renderer_, config_.get());
        }
        catch (const std::exception &e)
        {
            spdlog::error("初始化InputManager失败: {}", e.what());
            return false;
        }
        spdlog::trace("InputMangaer初始化成功");
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

    void GameApp::testRenderer()
    {
        engine::render::Sprite sprite_world("assets/textures/Actors/frog.png");
        engine::render::Sprite sprite_ui("assets/textures/UI/buttons/Start1.png");
        engine::render::Sprite sprite_parallax("assets/textures/Layers/back.png");

        static float rotation = 0.0f;
        rotation += 0.01f;

        renderer_->drawParallax(*camera_, sprite_parallax, glm::vec2(100, 100), glm::vec2(0.5f, 0.5f), glm::bvec2(true, false));
        renderer_->drawSprite(*camera_, sprite_world, glm::vec2(200, 200), glm::vec2(1.0f, 1.0f), rotation);
        renderer_->drawUISprite(sprite_ui, glm::vec2(100, 100));
    }

    void GameApp::testCamera()
    {
        auto key_state = SDL_GetKeyboardState(nullptr);
        if (key_state[SDL_SCANCODE_UP])
            camera_->move(glm::vec2(0, -0.1));
        if (key_state[SDL_SCANCODE_DOWN])
            camera_->move(glm::vec2(0, 0.1));
        if (key_state[SDL_SCANCODE_LEFT])
            camera_->move(glm::vec2(-0.1, 0));
        if (key_state[SDL_SCANCODE_RIGHT])
            camera_->move(glm::vec2(0.1, 0));
    }

    void GameApp::testInputManager()
    {
        vector<string> actions = {
            "move_up",
            "move_down",
            "move_left",
            "move_right",
            "jump",
            "attack",
            "pause",
            "MouseLeftClick",
            "MouseRightClick",
        };

        for (const auto &action : actions)
        {
            if (input_manager_->isActionPressed(action))
            {
                spdlog::info("{} 按下", action);
            }
            if (input_manager_->isActionReleased(action))
            {
                spdlog::info("{} 抬起", action);
            }
            if (input_manager_->isActionDown(action))
            {
                spdlog::info("{} 按下中", action);
            }
        }
    }

} // engine::core