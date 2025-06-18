# Fantasy Legend Server

Fantasy Legend 游戏服务器端，提供完整的游戏逻辑、数据持久化和网络通信功能。

## 项目结构

```
server/
├── CMakeLists.txt          # 主构建配置文件
├── build.sh               # 构建脚本
├── README.md              # 项目说明文档
├── src/                   # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── core/              # 核心游戏逻辑
│   ├── data/              # 数据管理
│   ├── utils/             # 工具类
│   └── api/               # API接口
├── include/               # 头文件目录
│   ├── core/              # 核心头文件
│   ├── data/              # 数据管理头文件
│   ├── utils/             # 工具类头文件
│   └── api/               # API头文件
├── test/                  # 测试文件
│   ├── test_data_persistence.cpp
│   └── CMakeLists.txt
├── examples/              # 示例程序
│   └── data_persistence_example.cpp
├── docs/                  # 文档目录
├── config/                # 配置文件
├── logs/                  # 日志文件
└── scripts/               # 脚本文件
```

## 快速开始

### 1. 环境要求

- **操作系统**: Linux (推荐 Ubuntu 18.04+)
- **编译器**: GCC 7+ 或 Clang 6+
- **CMake**: 3.16+
- **依赖库**:
  - SQLite3
  - JSONCPP (可选)
  - Threads (POSIX)

### 2. 安装依赖

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev libjsoncpp-dev
```

#### CentOS/RHEL:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake sqlite-devel jsoncpp-devel
```

### 3. 构建项目

#### 使用构建脚本（推荐）

```bash
# 进入服务器目录
cd Fantsy_Game/server

# 构建发布版本
./build.sh -r

# 构建调试版本
./build.sh -d

# 构建并运行测试
./build.sh -r -t

# 构建并运行示例
./build.sh -r -e

# 使用8个并行任务构建
./build.sh -r -j 8

# 详细输出
./build.sh -r -v

# 清理构建目录
./build.sh --clean

# 格式化代码
./build.sh --format

# 静态代码分析
./build.sh --analyze
```

#### 手动构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j4

# 运行测试
make test

# 安装
sudo make install
```

## 构建脚本选项

| 选项 | 描述 |
|------|------|
| `-h, --help` | 显示帮助信息 |
| `-c, --clean` | 清理构建目录 |
| `-d, --debug` | 构建调试版本 |
| `-r, --release` | 构建发布版本 |
| `-t, --test` | 构建并运行测试 |
| `-e, --examples` | 构建示例程序 |
| `-j, --jobs N` | 使用N个并行任务 (默认: 4) |
| `-v, --verbose` | 详细输出 |
| `--install` | 安装到系统 |
| `--format` | 格式化代码 |
| `--analyze` | 静态代码分析 |

## 主要功能模块

### 1. 核心游戏逻辑 (core/)
- **Character**: 角色系统
- **Combat**: 战斗系统
- **Inventory**: 背包系统
- **Quest**: 任务系统
- **World**: 世界管理

### 2. 数据管理 (data/)
- **DatabaseManager**: 数据库管理
- **DataPersistenceManager**: 数据持久化
- **CacheManager**: 缓存管理

### 3. 工具类 (utils/)
- **Logger**: 日志系统
- **ConfigManager**: 配置管理
- **EventSystem**: 事件系统

### 4. API接口 (api/)
- **NetworkManager**: 网络管理
- **HttpServer**: HTTP服务器
- **WebSocketServer**: WebSocket服务器

## 测试

项目包含完整的单元测试套件：

```bash
# 运行所有测试
./build.sh -r -t

# 或者手动运行
cd build
./FantasyLegendTests
```

## 示例程序

查看示例程序了解如何使用各个模块：

```bash
# 运行示例程序
./build.sh -r -e

# 或者手动运行
cd build
./FantasyLegendExamples
```

## 配置

配置文件位于 `config/` 目录：

- `server.conf`: 服务器配置
- `database.conf`: 数据库配置
- `logging.conf`: 日志配置

## 日志

日志文件保存在 `logs/` 目录：

- `server.log`: 服务器日志
- `error.log`: 错误日志
- `access.log`: 访问日志

## 开发指南

### 代码风格

项目使用以下代码风格：
- 使用 C++17 标准
- 遵循 Google C++ 风格指南
- 使用 4 空格缩进
- 类名使用 PascalCase
- 函数和变量使用 camelCase

### 添加新模块

1. 在 `src/` 和 `include/` 目录下创建相应的子目录
2. 添加源文件和头文件
3. 更新 `CMakeLists.txt` 文件
4. 添加相应的测试文件

### 调试

```bash
# 构建调试版本
./build.sh -d

# 使用 GDB 调试
gdb build/FantasyLegendServer

# 使用 Valgrind 检查内存泄漏
valgrind --leak-check=full build/FantasyLegendServer
```

## 性能优化

### 编译优化

```bash
# 使用优化编译
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

### 生产环境部署

```bash
# 构建发布版本
./build.sh -r

# 安装到系统
./build.sh --install

# 配置系统服务
sudo systemctl enable fantasy-legend-server
sudo systemctl start fantasy-legend-server
```

### Docker 部署

```dockerfile
FROM ubuntu:20.04

# 安装依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsqlite3-dev \
    libjsoncpp-dev

# 复制源代码
COPY . /app
WORKDIR /app/server

# 构建项目
RUN ./build.sh -r --install

# 暴露端口
EXPOSE 8080

# 启动服务器
CMD ["FantasyLegendServer"]
```

## 故障排除

### 常见问题

1. **CMake 配置失败**
   - 检查依赖库是否安装
   - 确保 CMake 版本 >= 3.16

2. **编译错误**
   - 检查编译器版本
   - 确保所有依赖头文件存在

3. **链接错误**
   - 检查库文件路径
   - 确保所有依赖库已安装

4. **运行时错误**
   - 检查配置文件
   - 查看日志文件

### 获取帮助

- 查看日志文件: `logs/`
- 运行测试: `./build.sh -t`
- 检查配置: `config/`

## 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 联系方式

- 项目维护者: [pengchengkang]
- 邮箱: [your-email@example.com]
- 项目地址: [项目仓库地址]

---

**注意**: 这是一个开发中的项目，API 可能会发生变化。请查看最新的文档和示例。 