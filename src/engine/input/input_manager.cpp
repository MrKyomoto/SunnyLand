#include "input_manager.h"
#include "../core/config.h"
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

namespace engine::input
{
    InputManager::InputManager(SDL_Renderer *sdl_renderer, const engine::core::Config *config)
        : sdl_renderer_(sdl_renderer)
    {
        initializeMappings(config);
        if (!sdl_renderer_)
        {
            spdlog::error("输入管理器: SDL_Renderer 为空指针");
            throw std::runtime_error("输入管理器: SDL_Renderer 为空指针");
        }

        // 获取鼠标初始位置
        float x, y;
        SDL_GetMouseState(&x, &y);
        mouse_position_ = {x, y};
        spdlog::trace("初始鼠标位置: ({},{})", mouse_position_.x, mouse_position_.y);
    }

    void InputManager::update()
    {
        // 每一帧开始，先把上一次的输入状态复位，再记录新的输入
        for (auto &[action_name, state] : action_states_)
        {
            if (state == ActionState::PRESSED_THIS_FRAME)
            {
                // 当某个按键按下不动时,并不会生成SDL_Event
                state = ActionState::HELD_DOWN;
            }
            else if (state == ActionState::RELEASED_THIS_FRAME)
            {
                state = ActionState::INACTIVE;
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            processEvent(event);
        }
    }

    bool InputManager::isActionDown(const string &action_name) const
    {
        if (auto it = action_states_.find(action_name); it != action_states_.end())
        {
            return it->second == ActionState::PRESSED_THIS_FRAME || it->second == ActionState::HELD_DOWN;
        }
        return false;
    }

    bool InputManager::isActionPressed(const string &action_name) const
    {
        if (auto it = action_states_.find(action_name); it != action_states_.end())
        {
            return it->second == ActionState::PRESSED_THIS_FRAME;
        }
        return false;
    }

    bool InputManager::isActionReleased(const string &action_name) const
    {
        if (auto it = action_states_.find(action_name); it != action_states_.end())
        {
            return it->second == ActionState::RELEASED_THIS_FRAME;
        }
        return false;
    }

    bool InputManager::shouldQuit() const
    {
        return should_quit_;
    }

    void InputManager::setShouldQuit(bool should_quit)
    {
        should_quit_ = should_quit;
    }

    glm::vec2 InputManager::getMousePosition() const
    {
        return mouse_position_;
    }

    glm::vec2 InputManager::getLogicMousePosition() const
    {
        glm::vec2 logical_pos;
        // 通过窗口坐标获取渲染坐标
        SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y, &logical_pos.x, &logical_pos.y);
        return logical_pos;
    }

    void InputManager::processEvent(const SDL_Event &event)
    {
        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            SDL_Scancode scancode = event.key.scancode;
            bool is_down = event.key.down;
            bool is_repeat = event.key.repeat;

            auto it = input_to_actions_map_.find(scancode);
            if (it != input_to_actions_map_.end())
            {
                const vector<string> &associated_actions = it->second;
                for (const string &action_name : associated_actions)
                {
                    updateActionState(action_name, is_down, is_repeat);
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            Uint32 button = event.button.button;
            bool is_down = event.button.down;

            auto it = input_to_actions_map_.find(button);
            if (it != input_to_actions_map_.end())
            {
                const vector<string> &associated_actions = it->second;
                for (const string &action_name : associated_actions)
                {
                    // 鼠标事件不考虑repeat
                    updateActionState(action_name, is_down, false);
                }
            }
            mouse_position_ = {event.button.x, event.button.y};
            break;
        }
        case SDL_EVENT_MOUSE_MOTION:
        {

            mouse_position_ = {event.button.x, event.button.y};
            break;
        }
        case SDL_EVENT_QUIT:
        {
            should_quit_ = true;
            break;
        }

        default:
            break;
        }
    }

    void InputManager::initializeMappings(const engine::core::Config *config)
    {
        spdlog::trace("初始化输入映射...");
        if (!config)
        {
            spdlog::error("输入管理器: Config 为空指针");
            throw std::runtime_error("输入管理器: Config 为空指针");
        }

        // 动作 -> 按键名称
        actions_to_keyname_map_ = config->input_mappings_;
        input_to_actions_map_.clear();
        action_states_.clear();

        if (actions_to_keyname_map_.find("MouseLeftClick") == actions_to_keyname_map_.end())
        {
            spdlog::debug("配置中没有定义 'MouseLeftClick' 动作,添加默认映射到 'MouseLeft'");
            actions_to_keyname_map_["MouseLeftClick"] = {"MouseLeft"};
        }

        if (actions_to_keyname_map_.find("MouseRightClick") == actions_to_keyname_map_.end())
        {
            spdlog::debug("配置中没有定义 'MouseRightClick' 动作,添加默认映射到 'MouseRight'");
            actions_to_keyname_map_["MouseRightClick"] = {"MouseRight"};
        }

        for (const auto &[action_name, key_names] : actions_to_keyname_map_)
        {
            action_states_[action_name] = ActionState::INACTIVE;
            spdlog::trace("映射动作: {}", action_name);

            // 设置 按键 -> 动作 的映射
            for (const string &key_name : key_names)
            {
                SDL_Scancode scancode = scancodeFromString(key_name);
                Uint32 mouse_button = mouseButtonUint8FromString(key_name);
                // 未来可添加其他输入类型... like 手柄

                if (scancode != SDL_SCANCODE_UNKNOWN)
                {
                    input_to_actions_map_[scancode].push_back(action_name);
                    spdlog::trace("映射按键: {} (Scancode: {}) 到动作: {}", key_name, static_cast<int>(scancode), action_name);
                }
                else if (mouse_button != 0)
                {
                    input_to_actions_map_[mouse_button].push_back(action_name);
                    spdlog::trace("映射鼠标按键: {} (Scancode: {}) 到动作: {}", key_name, static_cast<int>(mouse_button), action_name);
                }
                else
                {
                    spdlog::warn("输入映射警告: 未知按键或按钮名称 '{}' 用于动作 '{}'", key_name, action_name);
                }
            }
        }
        spdlog::trace("输入管理初始化按键映射完成");
    }

    void InputManager::updateActionState(const string &action_name, bool is_input_active, bool is_repeat_event)
    {
        auto it = action_states_.find(action_name);
        if (it == action_states_.end())
        {
            spdlog::warn("尝试更新未注册的动作状态: {}", action_name);
            return;
        }

        if (is_input_active)
        {
            if (is_repeat_event)
            {
                it->second = ActionState::HELD_DOWN;
            }
            else
            {
                it->second = ActionState::PRESSED_THIS_FRAME;
            }
        }
        else
        {
            it->second = ActionState::RELEASED_THIS_FRAME;
        }
    }

    SDL_Scancode InputManager::scancodeFromString(const string &key_name)
    {
        return SDL_GetScancodeFromName(key_name.c_str());
    }

    Uint32 InputManager::mouseButtonUint8FromString(const string &button_name)
    {
        if (button_name == "MouseLeft")
            return SDL_BUTTON_LEFT;
        if (button_name == "MouseMiddle")
            return SDL_BUTTON_MIDDLE;
        if (button_name == "MouseRight")
            return SDL_BUTTON_RIGHT;
        if (button_name == "MouseX1")
            return SDL_BUTTON_X1;
        if (button_name == "MouseX2")
            return SDL_BUTTON_X2;
        return 0;
    }

} // namespace engine::input
