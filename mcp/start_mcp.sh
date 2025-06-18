#!/bin/bash

# Fantasy Legend MCP Server 启动脚本

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Fantasy Legend MCP Server"
echo "项目根目录: $PROJECT_ROOT"
echo "MCP脚本目录: $SCRIPT_DIR"
echo ""

# 检查Python版本
python_version=$(python3 --version 2>&1 | cut -d' ' -f2 | cut -d'.' -f1,2)
required_version="3.11"

if [ "$(printf '%s\n' "$required_version" "$python_version" | sort -V | head -n1)" != "$required_version" ]; then
    echo "错误: 需要Python $required_version或更高版本，当前版本: $python_version"
    exit 1
fi

# 激活虚拟环境
echo "激活虚拟环境..."
source /root/PY/venv/bin/activate

# 检查依赖
echo "检查依赖..."
python3 -c "import fastmcp" 2>/dev/null || {
    echo "安装 fastmcp..."
    pip3 install fastmcp
}

python3 -c "import psutil" 2>/dev/null || {
    echo "安装 psutil..."
    pip3 install psutil
}

python3 -c "import PySide6" 2>/dev/null || {
    echo "安装 PySide6..."
    pip3 install PySide6
}

echo "依赖检查完成"
echo ""

# 设置环境变量
export FANTASY_LEGEND_PROJECT_ROOT="$PROJECT_ROOT"
export FANTASY_LEGEND_QT_PATH="/home/pck/tools/qtlib"

# 启动MCP服务器
echo "启动MCP服务器..."
cd "$SCRIPT_DIR"
python3 use_existing_mcp.py "$@" 