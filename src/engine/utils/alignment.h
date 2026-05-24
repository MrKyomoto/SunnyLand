#pragma once
namespace engine::utils
{
    /// @brief 定义对象或组件相对于其位置参考点的对齐方式
    enum class Alignment
    {
        NONE,       // 不指定对齐方式,偏移量通常为(0,0)或手动设置
        TOP_LEFT,   // 左上角
        TOP_CENTER, // 顶部中心
        TOP_RIGHT,  // 右上角

        CENTER_LEFT, // 中心左侧
        CENTER,
        CENTER_RIGHT, // 中心右侧

        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
    };
} // namespace engine::utils
