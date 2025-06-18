#!/bin/bash

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 设置目录
THIRD_PARTY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/third_party"
mkdir -p "$THIRD_PARTY_DIR"

echo -e "${YELLOW}开始下载第三方库...${NC}"

# 下载 stb
# echo -e "${GREEN}下载 stb 库...${NC}"
# STB_DIR="$THIRD_PARTY_DIR/stb"
# if [ ! -d "$STB_DIR" ]; then
#     git clone https://github.com/nothings/stb.git "$STB_DIR"
#     # 只保留需要的头文件
#     cd "$STB_DIR"
#     git clean -fdx
#     git checkout master
#     # 创建单个头文件版本
#     echo "#define STB_IMAGE_IMPLEMENTATION" > stb_image_impl.h
#     cat stb_image.h >> stb_image_impl.h
#     echo "#define STB_IMAGE_WRITE_IMPLEMENTATION" > stb_image_write_impl.h
#     cat stb_image_write.h >> stb_image_write_impl.h
#     echo "#define STB_IMAGE_RESIZE_IMPLEMENTATION" > stb_image_resize_impl.h
#     cat stb_image_resize.h >> stb_image_resize_impl.h
#     cd - > /dev/null
# fi

# 下载 spdlog
# echo -e "${GREEN}下载 spdlog 库...${NC}"
# SPDLOG_DIR="$THIRD_PARTY_DIR/spdlog"
# if [ ! -d "$SPDLOG_DIR" ]; then
#     git clone https://github.com/gabime/spdlog.git "$SPDLOG_DIR"
#     cd "$SPDLOG_DIR"
#     git checkout v1.12.0
#     git submodule update --init --recursive
#     cd - > /dev/null
# fi

# 下载 nlohmann_json
echo -e "${GREEN}下载 nlohmann_json 库...${NC}"
JSON_DIR="$THIRD_PARTY_DIR/json"
if [ ! -d "$JSON_DIR" ]; then
    git clone https://github.com/nlohmann/json.git "$JSON_DIR"
    cd "$JSON_DIR"
    git checkout v3.11.3
    cd - > /dev/null
fi

# 下载 pybind11
echo -e "${GREEN}下载 pybind11 库...${NC}"
PYBIND11_DIR="$THIRD_PARTY_DIR/pybind11"
if [ ! -d "$PYBIND11_DIR" ]; then
    git clone https://github.com/pybind/pybind11.git "$PYBIND11_DIR"
    cd "$PYBIND11_DIR"
    git checkout v2.11.1
    git submodule update --init --recursive
    cd - > /dev/null
fi

echo -e "${GREEN}所有第三方库下载完成！${NC}"
echo -e "${YELLOW}第三方库位置: $THIRD_PARTY_DIR${NC}" 