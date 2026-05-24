#include "game_object.h"
#include "../render/renderer.h"
#include "../input/input_manager.h"
#include "../render/camera.h"

namespace engine::object
{
    GameObject::GameObject(const std::string &name, const std::string &tag)
        : name_(name), tag_(tag)
    {
        spdlog::trace("GameObject created: {} {}", name_, tag_);
    }

    void GameObject::update(float delta_time)
    {
        for(auto & pair : components_){
            pair.second->update(delta_time);
        }
    }

    void GameObject::render()
    {
        for(auto & pair : components_){
            pair.second->render();
        }
    }

    void GameObject::clean()
    {
        spdlog::trace("Cleaning GameObject...");
        for(auto & pair : components_){
            pair.second->clean();
        }
        components_.clear();
    }

    void GameObject::handleInput()
    {
        for(auto & pair : components_){
            pair.second->handleInput();
        }
    }

} // namespace engine::object
