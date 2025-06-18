# Fantasy Legend

一个基于C++开发的奇幻MMORPG游戏项目，包含完整的服务器端和客户端架构。

## 项目概述

Fantasy Legend 是一个开源的MMORPG游戏项目，采用现代化的C++技术栈开发。项目包含完整的游戏逻辑、数据持久化、网络通信、用户界面等功能模块。

## 项目特色

- 🎮 **完整的MMORPG功能**: 角色系统、战斗系统、任务系统、背包系统等
- 🏗️ **模块化架构**: 清晰的代码结构，易于扩展和维护
- 💾 **数据持久化**: 支持SQLite数据库，自动保存和备份
- 🌐 **网络通信**: 支持HTTP和WebSocket协议
- 🧪 **完整测试**: 包含单元测试和集成测试
- 📚 **详细文档**: 完整的API文档和使用示例
- 🚀 **高性能**: 优化的代码结构和缓存机制

## 技术栈

### 后端 (Server)
- **语言**: C++17
- **构建系统**: CMake 3.16+
- **数据库**: SQLite3
- **JSON处理**: JSONCPP
- **网络**: POSIX Threads
- **日志**: 自定义日志系统

### 前端 (Client)
- **语言**: C++17
- **图形库**: SFML 2.5+
- **UI框架**: 自定义UI系统
- **音频**: SFML Audio
- **网络**: SFML Network

## 项目结构

```
Fantsy_Game/
├── README.md                 # 项目说明文档
├── CMakeLists.txt            # 项目根构建文件
├── server/                   # 服务器端
│   ├── CMakeLists.txt        # 服务器构建配置
│   ├── build.sh              # 构建脚本
│   ├── README.md             # 服务器说明文档
│   ├── src/                  # 源代码
│   │   ├── main.cpp          # 主程序入口
│   │   ├── core/             # 核心游戏逻辑
│   │   ├── data/             # 数据管理
│   │   ├── utils/            # 工具类
│   │   └── api/              # API接口
│   ├── include/              # 头文件
│   ├── test/                 # 测试文件
│   ├── examples/             # 示例程序
│   ├── config/               # 配置文件
│   ├── scripts/              # 脚本文件
│   └── docs/                 # 文档
├── client/                   # 客户端
│   ├── CMakeLists.txt        # 客户端构建配置
│   ├── src/                  # 源代码
│   ├── include/              # 头文件
│   ├── assets/               # 游戏资源
│   └── docs/                 # 文档
├── shared/                   # 共享代码
│   ├── include/              # 共享头文件
│   └── src/                  # 共享源文件
└── docs/                     # 项目文档
    ├── api/                  # API文档
    ├── design/               # 设计文档
    └── user_guide/           # 用户指南
```

## 快速开始

### 环境要求

- **操作系统**: Linux (推荐 Ubuntu 18.04+), Windows 10+, macOS 10.14+
- **编译器**: GCC 7+ 或 Clang 6+ 或 MSVC 2019+
- **CMake**: 3.16+
- **依赖库**: 详见各模块的README

### 安装依赖

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev libjsoncpp-dev libsfml-dev
```

#### CentOS/RHEL:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake sqlite-devel jsoncpp-devel sfml-devel
```

#### Windows:
- 安装 Visual Studio 2019 或更新版本
- 安装 CMake
- 使用 vcpkg 安装依赖库

### 构建项目

#### 构建服务器
```bash
cd Fantsy_Game/server
./build.sh -r  # 构建发布版本
```

#### 构建客户端
```bash
cd Fantsy_Game/client
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

#### 构建整个项目
```bash
cd Fantsy_Game
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

### 运行项目

#### 启动服务器
```bash
cd Fantsy_Game/server
./scripts/start_server.sh  # 前台启动
./scripts/start_server.sh -d  # 守护进程模式
```

#### 启动客户端
```bash
cd Fantsy_Game/client/build
./FantasyLegendClient
```

## 主要功能模块

### 服务器端功能

#### 1. 核心游戏逻辑
- **角色系统**: 角色创建、属性管理、等级系统
- **战斗系统**: 实时战斗、技能系统、伤害计算
- **背包系统**: 物品管理、装备系统、交易功能
- **任务系统**: 任务链、奖励系统、进度跟踪
- **世界管理**: 地图系统、NPC管理、事件系统

#### 2. 数据管理
- **数据库管理**: SQLite数据库操作、连接池
- **数据持久化**: 自动保存、数据备份、恢复功能
- **缓存管理**: 内存缓存、LRU算法、过期策略

#### 3. 网络通信
- **HTTP服务器**: RESTful API、JSON响应
- **WebSocket服务器**: 实时通信、双向数据流
- **连接管理**: 连接池、负载均衡、安全验证

#### 4. 工具系统
- **日志系统**: 分级日志、文件轮转、远程日志
- **配置管理**: 配置文件解析、热重载
- **事件系统**: 事件驱动、观察者模式

### 客户端功能

#### 1. 用户界面
- **主菜单**: 登录、注册、服务器选择
- **游戏界面**: HUD、小地图、聊天窗口
- **背包界面**: 物品管理、装备预览
- **技能界面**: 技能树、快捷键设置

#### 2. 图形渲染
- **2D渲染**: 精灵动画、粒子效果
- **UI渲染**: 自定义UI组件、主题系统
- **地图渲染**: 瓦片地图、动态加载

#### 3. 音频系统
- **背景音乐**: 场景音乐、战斗音乐
- **音效系统**: 技能音效、环境音效
- **音频管理**: 音量控制、音频格式支持

#### 4. 网络通信
- **服务器连接**: 连接管理、断线重连
- **数据同步**: 实时数据更新、状态同步
- **消息处理**: 协议解析、消息队列

## 开发指南

### 代码风格

项目遵循以下代码规范：
- 使用 C++17 标准
- 遵循 Google C++ 风格指南
- 使用 4 空格缩进
- 类名使用 PascalCase
- 函数和变量使用 camelCase
- 常量使用 UPPER_SNAKE_CASE

### 添加新功能

1. **创建功能分支**
   ```bash
   git checkout -b feature/new-feature
   ```

2. **编写代码**
   - 在相应的模块目录下添加源文件
   - 更新 CMakeLists.txt
   - 添加必要的头文件

3. **编写测试**
   - 在 test/ 目录下添加测试文件
   - 确保测试覆盖率

4. **更新文档**
   - 更新 API 文档
   - 添加使用示例

5. **提交代码**
   ```bash
   git add .
   git commit -m "Add new feature: description"
   git push origin feature/new-feature
   ```

### 调试技巧

#### 服务器调试
```bash
# 构建调试版本
cd server && ./build.sh -d

# 使用 GDB 调试
gdb build/FantasyLegendServer

# 查看日志
tail -f logs/server.log
```

#### 客户端调试
```bash
# 构建调试版本
cd client && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# 使用 GDB 调试
gdb FantasyLegendClient
```

## 测试

### 运行测试

```bash
# 服务器测试
cd server && ./build.sh -r -t

# 客户端测试
cd client/build && make test
```

### 测试覆盖率

```bash
# 生成覆盖率报告
cd server && ./build.sh -d --coverage
```

## 性能优化

### 编译优化

```bash
# 启用优化编译
./build.sh -r -j 8

# 启用链接时优化
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### 运行时优化

- 启用缓存系统
- 使用连接池
- 配置适当的线程数
- 启用压缩

## 部署

### 生产环境

#### 服务器部署
```bash
# 构建发布版本
cd server && ./build.sh -r

# 安装到系统
./build.sh --install

# 配置系统服务
sudo systemctl enable fantasy-legend-server
sudo systemctl start fantasy-legend-server
```

#### 客户端打包
```bash
# 构建发布版本
cd client && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# 创建安装包
cpack
```

### Docker 部署

```dockerfile
# 服务器 Dockerfile
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    build-essential cmake libsqlite3-dev libjsoncpp-dev

COPY . /app
WORKDIR /app/server

RUN ./build.sh -r --install

EXPOSE 8080
CMD ["FantasyLegendServer"]
```

## 贡献指南

我们欢迎所有形式的贡献！请遵循以下步骤：

1. **Fork 项目**
2. **创建功能分支**: `git checkout -b feature/amazing-feature`
3. **提交更改**: `git commit -m 'Add amazing feature'`
4. **推送到分支**: `git push origin feature/amazing-feature`
5. **创建 Pull Request**

### 贡献类型

- 🐛 **Bug 修复**: 修复已知问题
- ✨ **新功能**: 添加新功能
- 📚 **文档**: 改进文档
- 🧪 **测试**: 添加或改进测试
- 🔧 **工具**: 改进构建工具或开发工具
- 🎨 **UI/UX**: 改进用户界面或用户体验

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

## 联系方式

- **项目维护者**: [pengchengkang]
- **邮箱**: [your-email@example.com]
- **项目地址**: [项目仓库地址]
- **问题反馈**: [Issues页面]

## 致谢

感谢所有为这个项目做出贡献的开发者和用户！

---

**注意**: 这是一个开发中的项目，API 可能会发生变化。请查看最新的文档和示例。 