# Fantasy Legend MCP Server

为Fantasy Legend游戏项目提供MCP (Model Context Protocol) 工具支持。

## 功能特性

### 编译工具
- `compile_client` - 编译客户端项目
- `compile_server` - 编译服务器项目
- `check_compilation_errors` - 检查编译错误

### 运行工具
- `run_client` - 运行客户端
- `run_server` - 运行服务器

### 项目管理
- `get_project_status` - 获取项目状态信息
- `fix_include_paths` - 修复头文件包含路径问题
- `create_missing_files` - 创建缺失的文件和目录

### 交互反馈
- `interactive_feedback` - 请求用户交互反馈

## 安装

1. 确保已安装Python 3.11+
2. 安装依赖：
```bash
pip install fastmcp psutil pyside6
```

## 使用方法

### 在Cursor中配置

1. 打开Cursor设置
2. 找到MCP配置部分
3. 添加新的MCP服务器：

```json
{
  "mcpServers": {
    "fantasy-legend": {
      "command": "python",
      "args": ["/path/to/Fantsy_Game/mcp/fantasy_legend_mcp.py"]
    }
  }
}
```

### 命令行使用

```bash
# 直接运行MCP服务器
python fantasy_legend_mcp.py

# 或者通过stdio传输
python fantasy_legend_mcp.py --stdio
```

## 工具说明

### compile_client
编译客户端项目，支持debug、release和clean模式。

**参数：**
- `build_type` (可选): "debug", "release", 或 "clean"

**示例：**
```python
# 调试模式编译
compile_client(build_type="debug")

# 清理构建目录
compile_client(build_type="clean")
```

### compile_server
编译服务器项目，支持debug、release和clean模式。

**参数：**
- `build_type` (可选): "debug", "release", 或 "clean"

### check_compilation_errors
检查项目中的编译错误。

**返回：**
- 成功：`{"success": true, "message": "No compilation errors found"}`
- 失败：`{"success": false, "errors": ["错误列表"]}`

### get_project_status
获取项目的当前状态信息。

**返回：**
```json
{
  "success": true,
  "status": {
    "project_root": "/path/to/project",
    "client_dir": "/path/to/client",
    "server_dir": "/path/to/server",
    "client_compiled": true,
    "server_compiled": false,
    "qt_path": "/home/pck/tools/qtlib",
    "client_cmake_exists": true,
    "server_cmake_exists": true
  }
}
```

### fix_include_paths
自动修复头文件包含路径问题。

**功能：**
- 将 `#include "include/xxx.h"` 改为 `#include "xxx.h"`
- 适用于客户端项目中的所有.cpp文件

### create_missing_files
创建项目所需的缺失文件和目录。

**创建内容：**
- assets/ui/styles/ - UI样式目录
- assets/images/ - 图片资源目录
- assets/sounds/ - 音频资源目录
- logs/ - 日志目录
- assets/ui/styles/main.qss - 默认样式文件

## 项目结构

```
Fantsy_Game/
├── mcp/
│   ├── fantasy_legend_mcp.py    # MCP服务器主文件
│   ├── pyproject.toml          # 项目配置
│   └── README.md               # 说明文档
├── client/                     # 客户端项目
└── server/                     # 服务器项目
```

## 故障排除

### 常见问题

1. **Qt路径问题**
   - 确保Qt6安装在 `/home/pck/tools/qtlib`
   - 在CMakeLists.txt中正确设置CMAKE_PREFIX_PATH

2. **编译错误**
   - 使用 `check_compilation_errors` 检查具体错误
   - 使用 `fix_include_paths` 修复包含路径问题

3. **权限问题**
   - 确保有足够的权限创建和修改文件
   - 检查构建目录的写入权限

## 开发

### 添加新工具

1. 在 `fantasy_legend_mcp.py` 中添加新的函数
2. 使用 `@mcp.tool()` 装饰器
3. 添加适当的类型注解和文档字符串

### 测试

```bash
# 测试MCP服务器
python fantasy_legend_mcp.py --test

# 测试特定工具
python -c "from fantasy_legend_mcp import get_project_status; print(get_project_status())"
```

## 许可证

本项目采用MIT许可证。 