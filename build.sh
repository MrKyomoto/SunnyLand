#!/bin/bash

# 默认编译模式为 Release
BUILD_TYPE="Release"

# 如果传入了参数，则使用参数作为编译模式 (Debug 或 Release)
if [ $# -ge 1 ]; then
  BUILD_TYPE=$(echo "$1" | tr '[:lower:]' '[:upper:]')
  if [ "$BUILD_TYPE" != "DEBUG" ] && [ "$BUILD_TYPE" != "RELEASE" ]; then
    echo "❌ 错误：只支持 Debug 或 Release"
    exit 1
  fi
fi

echo "========================================"
echo "  编译模式: $BUILD_TYPE"
echo "========================================"

# 创建 build 目录
mkdir -p build
cd build || exit 1

# 执行 CMake
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

make -j"$(nproc)"

cd ..

TARGET="SunnyLand-Linux"

if [ -z "$TARGET" ]; then
  echo "❌ 找不到可执行文件"
  exit 1
fi

echo "✅ 编译完成！运行: $TARGET"
echo ""

# 直接运行
./"$TARGET"
