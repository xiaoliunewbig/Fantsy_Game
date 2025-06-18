# Fantasy Legend 资源管理系统 API 文档

## 概述

Fantasy Legend 资源管理系统是一个高性能、可扩展的资源管理解决方案，专为大型游戏项目设计。系统采用现代C++17标准，支持多种资源类型，具备智能缓存、异步加载、压缩优化等高级功能。

## 技术特性

### 核心技术栈
- **语言**: C++17
- **内存管理**: 智能指针 + 内存池
- **并发处理**: 多线程 + 异步加载
- **压缩算法**: zlib, LZ4, Zstandard
- **序列化**: JSON, XML, YAML, 二进制
- **第三方库**: RapidJSON, TinyXML2, YAML-CPP, STB Image

### 性能优化技术
1. **内存池管理**: 减少内存分配开销，提高内存利用率
2. **对象池**: 重用对象实例，减少GC压力
3. **智能缓存**: LRU + LFU混合策略，自适应缓存大小
4. **异步加载**: 多线程并发加载，非阻塞主线程
5. **压缩优化**: 多种压缩算法，平衡压缩比和性能
6. **LOD系统**: 多层次细节，动态调整资源质量
7. **分块加载**: 大地图分块处理，按需加载

### 架构设计
- **模块化设计**: 松耦合，高内聚
- **插件化架构**: 支持自定义资源类型
- **事件驱动**: 异步事件通知机制
- **配置驱动**: 灵活的配置系统
- **错误处理**: 完善的异常处理机制

## API 参考

### 核心类

#### ResourceSystem
资源管理系统的主控制器，负责协调各个子系统。

```cpp
class ResourceSystem {
public:
    // 单例模式
    static ResourceSystem& getInstance();
    
    // 初始化和配置
    bool initialize(const ResourceSystemConfig& config);
    void shutdown();
    
    // 资源管理
    std::shared_ptr<IResource> loadResource(const std::filesystem::path& path, ResourceType type);
    std::shared_ptr<IResource> loadResourceAsync(const std::filesystem::path& path, ResourceType type);
    bool unloadResource(const std::string& resourceId);
    bool reloadResource(const std::string& resourceId);
    
    // 预加载
    void preloadResources(const std::vector<PreloadTask>& tasks);
    void cancelPreload(const std::string& resourceId);
    
    // 缓存管理
    void clearCache();
    void optimizeCache();
    ResourceSystemStats getStats() const;
};
```

#### ResourceManager
资源管理器，负责资源的生命周期管理。

```cpp
class ResourceManager {
public:
    // 资源操作
    std::shared_ptr<IResource> load(const std::filesystem::path& path, ResourceType type);
    bool unload(const std::string& resourceId);
    bool reload(const std::string& resourceId);
    
    // 资源查询
    std::shared_ptr<IResource> get(const std::string& resourceId) const;
    bool has(const std::string& resourceId) const;
    std::vector<std::string> getAllResourceIds() const;
    
    // 批量操作
    void unloadAll();
    void reloadAll();
};
```

#### ResourceCache
智能缓存系统，支持多种缓存策略。

```cpp
class ResourceCache {
public:
    // 缓存操作
    bool put(const std::string& key, std::shared_ptr<IResource> resource);
    std::shared_ptr<IResource> get(const std::string& key);
    bool remove(const std::string& key);
    
    // 缓存管理
    void clear();
    void setMaxSize(size_t maxSize);
    size_t getSize() const;
    size_t getMaxSize() const;
    
    // 缓存策略
    void setPolicy(CachePolicy policy);
    CachePolicy getPolicy() const;
};
```

#### ResourceLoader
资源加载器，支持同步和异步加载。

```cpp
class ResourceLoader {
public:
    // 加载器注册
    void registerLoader(ResourceType type, std::shared_ptr<IResourceLoader> loader);
    
    // 资源加载
    std::shared_ptr<IResource> load(const std::filesystem::path& path, ResourceType type);
    void loadAsync(const std::filesystem::path& path, ResourceType type, 
                   std::function<void(std::shared_ptr<IResource>)> callback);
    
    // 异步控制
    bool cancelAsyncLoad(const std::filesystem::path& path);
    void waitForAll();
    void stopAll();
    
    // 线程管理
    void setThreadCount(size_t threadCount);
    size_t getQueueSize() const;
};
```

### 资源类型

#### ConfigResource
配置文件资源，支持多种格式。

```cpp
class ConfigResource : public IResource {
public:
    // 配置访问
    std::string getValue(const std::string& key, const std::string& defaultValue = "") const;
    int getIntValue(const std::string& key, int defaultValue = 0) const;
    double getDoubleValue(const std::string& key, double defaultValue = 0.0) const;
    bool getBoolValue(const std::string& key, bool defaultValue = false) const;
    std::vector<std::string> getStringArray(const std::string& key) const;
    
    // 配置查询
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getAllKeys() const;
    
    // 格式支持
    ConfigFormat getFormat() const;
    bool isCompressed() const;
};
```

#### MapResource
地图资源，支持分块加载和LOD。

```cpp
class MapResource : public IResource {
public:
    // 地图信息
    int getWidth() const;
    int getHeight() const;
    int getTileWidth() const;
    int getTileHeight() const;
    
    // 瓦片访问
    uint32_t getTile(int x, int y) const;
    uint32_t getTileLOD(int x, int y, int lodLevel) const;
    
    // 对象查询
    std::vector<MapObject> getObjectsInArea(int x, int y, int width, int height) const;
    
    // 分块访问
    MapChunk getChunk(int chunkX, int chunkY) const;
    
    // LOD控制
    void setUseLOD(bool useLOD);
    bool isUseLOD() const;
    int getMaxLODLevel() const;
};
```

#### TextureResource
纹理资源，支持多种格式和压缩。

```cpp
class TextureResource : public IResource {
public:
    // 纹理信息
    int getWidth() const;
    int getHeight() const;
    TextureFormat getFormat() const;
    int getChannels() const;
    
    // 数据访问
    const uint8_t* getData() const;
    size_t getDataSize() const;
    
    // 压缩支持
    bool isCompressed() const;
    CompressionFormat getCompressionFormat() const;
    
    // 生成Mipmap
    bool generateMipmaps();
    int getMipmapCount() const;
};
```

#### AudioResource
音频资源，支持流式播放。

```cpp
class AudioResource : public IResource {
public:
    // 音频信息
    int getSampleRate() const;
    int getChannels() const;
    AudioFormat getFormat() const;
    double getDuration() const;
    
    // 数据访问
    const uint8_t* getData() const;
    size_t getDataSize() const;
    
    // 流式播放
    bool isStreaming() const;
    std::shared_ptr<AudioStream> createStream();
};
```

### 高级功能

#### ResourcePreloader
智能预加载系统。

```cpp
class ResourcePreloader {
public:
    // 预加载任务
    void addTask(const PreloadTask& task);
    void addTasks(const std::vector<PreloadTask>& tasks);
    
    // 任务控制
    bool cancelTask(const std::string& resourceId);
    void cancelAllTasks();
    
    // 状态查询
    PreloadTaskStatus getTaskStatus(const std::string& resourceId) const;
    PreloadStats getStats() const;
    
    // 策略配置
    void setStrategy(PreloadStrategy strategy);
    PreloadStrategy getStrategy() const;
};
```

#### ResourceCompressor
资源压缩系统。

```cpp
class ResourceCompressor {
public:
    // 压缩操作
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data, 
                                 CompressionAlgorithm algorithm = CompressionAlgorithm::ZLIB,
                                 int level = 6);
    
    // 解压缩操作
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData,
                                   CompressionAlgorithm algorithm = CompressionAlgorithm::ZLIB);
    
    // 压缩信息
    bool isCompressed(const std::vector<uint8_t>& data) const;
    CompressionAlgorithm detectAlgorithm(const std::vector<uint8_t>& data) const;
    double getCompressionRatio(const std::vector<uint8_t>& original, 
                              const std::vector<uint8_t>& compressed) const;
};
```

#### ResourcePackager
资源打包系统。

```cpp
class ResourcePackager {
public:
    // 打包操作
    bool createPackage(const std::string& packageName, 
                      const std::vector<std::filesystem::path>& resources);
    
    // 解包操作
    bool extractPackage(const std::string& packagePath, 
                       const std::filesystem::path& outputDir);
    
    // 包信息
    PackageInfo getPackageInfo(const std::string& packagePath) const;
    std::vector<std::string> listPackageContents(const std::string& packagePath) const;
    
    // 包管理
    bool addToPackage(const std::string& packagePath, 
                     const std::filesystem::path& resourcePath);
    bool removeFromPackage(const std::string& packagePath, 
                          const std::string& resourceName);
};
```

#### ResourceVersionControl
资源版本控制系统。

```cpp
class ResourceVersionControl {
public:
    // 版本管理
    bool createVersion(const std::string& version, const std::string& description = "");
    bool switchVersion(const std::string& version);
    std::string getCurrentVersion() const;
    
    // 版本信息
    std::vector<VersionInfo> getAllVersions() const;
    VersionInfo getVersionInfo(const std::string& version) const;
    
    // 差异管理
    std::vector<std::string> getChangedResources(const std::string& fromVersion, 
                                                const std::string& toVersion) const;
    bool createPatch(const std::string& fromVersion, const std::string& toVersion, 
                    const std::filesystem::path& patchPath);
    bool applyPatch(const std::filesystem::path& patchPath);
};
```

## 使用示例

### 基本使用

```cpp
#include "include/utils/resources/ResourceSystem.h"

int main() {
    // 初始化资源系统
    ResourceSystemConfig config;
    config.resourceRootDir = "resources/";
    config.maxCacheSize = 100 * 1024 * 1024; // 100MB
    config.loadingThreads = 4;
    config.enableLogging = true;
    config.enablePerformanceMonitoring = true;
    
    auto& resourceSystem = ResourceSystem::getInstance();
    if (!resourceSystem.initialize(config)) {
        std::cerr << "Failed to initialize resource system" << std::endl;
        return -1;
    }
    
    // 加载资源
    auto configResource = resourceSystem.loadResource("config/game.json", ResourceType::CONFIG);
    if (configResource) {
        auto configRes = std::dynamic_pointer_cast<ConfigResource>(configResource);
        std::string serverIP = configRes->getValue("server.ip", "127.0.0.1");
        int serverPort = configRes->getIntValue("server.port", 8080);
        std::cout << "Server: " << serverIP << ":" << serverPort << std::endl;
    }
    
    // 异步加载纹理
    resourceSystem.loadResourceAsync("textures/player.png", ResourceType::TEXTURE,
        [](std::shared_ptr<IResource> resource) {
            if (resource) {
                auto textureRes = std::dynamic_pointer_cast<TextureResource>(resource);
                std::cout << "Texture loaded: " << textureRes->getWidth() 
                          << "x" << textureRes->getHeight() << std::endl;
            }
        });
    
    // 预加载资源
    std::vector<PreloadTask> preloadTasks = {
        {"maps/level1.tmx", ResourceType::MAP, 1, PreloadStrategy::IMMEDIATE},
        {"models/player.fbx", ResourceType::MODEL, 2, PreloadStrategy::BACKGROUND},
        {"sounds/background.ogg", ResourceType::SOUND, 3, PreloadStrategy::LAZY}
    };
    resourceSystem.preloadResources(preloadTasks);
    
    // 获取统计信息
    auto stats = resourceSystem.getStats();
    std::cout << "Total resources: " << stats.totalResources << std::endl;
    std::cout << "Loaded resources: " << stats.loadedResources << std::endl;
    std::cout << "Cache hit rate: " << stats.cacheHitRate << "%" << std::endl;
    
    return 0;
}
```

### 高级功能使用

```cpp
#include "include/utils/resources/ResourceSystem.h"

void advancedUsage() {
    auto& resourceSystem = ResourceSystem::getInstance();
    
    // 创建资源包
    std::vector<std::filesystem::path> resources = {
        "textures/", "models/", "sounds/"
    };
    resourceSystem.createPackage("game_assets.pak", resources);
    
    // 版本控制
    resourceSystem.createVersion("v1.0.0", "Initial release");
    resourceSystem.createVersion("v1.1.0", "Bug fixes and improvements");
    resourceSystem.switchVersion("v1.1.0");
    
    // 压缩资源
    auto texture = resourceSystem.loadResource("textures/large.png", ResourceType::TEXTURE);
    if (texture) {
        auto compressed = resourceSystem.compressResource(texture, CompressionAlgorithm::ZSTD, 9);
        std::cout << "Compression ratio: " << compressed.compressionRatio << "%" << std::endl;
    }
    
    // 性能监控
    auto performanceReport = resourceSystem.generatePerformanceReport();
    std::cout << "Average load time: " << performanceReport.averageLoadTime << "ms" << std::endl;
    std::cout << "Memory usage: " << performanceReport.memoryUsage << "MB" << std::endl;
    std::cout << "Cache efficiency: " << performanceReport.cacheEfficiency << "%" << std::endl;
}
```

## 配置选项

### ResourceSystemConfig

```cpp
struct ResourceSystemConfig {
    std::filesystem::path resourceRootDir;     // 资源根目录
    size_t maxCacheSize;                       // 最大缓存大小
    size_t loadingThreads;                     // 加载线程数
    bool enableLogging;                        // 启用日志
    bool enablePerformanceMonitoring;          // 启用性能监控
    bool enableCompression;                    // 启用压缩
    bool enablePackaging;                      // 启用打包
    bool enableVersionControl;                 // 启用版本控制
    bool enablePreloading;                     // 启用预加载
    LogLevel logLevel;                         // 日志级别
    CompressionAlgorithm defaultCompression;   // 默认压缩算法
    int compressionLevel;                      // 压缩级别
    CachePolicy cachePolicy;                   // 缓存策略
    PreloadStrategy defaultPreloadStrategy;    // 默认预加载策略
};
```

### 缓存策略

```cpp
enum class CachePolicy {
    LRU,        // 最近最少使用
    LFU,        // 最少使用频率
    FIFO,       // 先进先出
    RANDOM,     // 随机替换
    HYBRID      // 混合策略
};
```

### 预加载策略

```cpp
enum class PreloadStrategy {
    IMMEDIATE,  // 立即加载
    BACKGROUND, // 后台加载
    LAZY,       // 懒加载
    ON_DEMAND   // 按需加载
};
```

## 性能优化建议

### 1. 内存管理
- 使用内存池减少分配开销
- 合理设置缓存大小
- 及时释放不需要的资源

### 2. 加载优化
- 使用异步加载避免阻塞
- 实现智能预加载策略
- 压缩资源减少I/O时间

### 3. 缓存优化
- 选择合适的缓存策略
- 监控缓存命中率
- 定期优化缓存

### 4. 并发优化
- 合理设置线程数
- 避免资源竞争
- 使用无锁数据结构

## 错误处理

系统提供完善的错误处理机制：

```cpp
try {
    auto resource = resourceSystem.loadResource("invalid/path.txt", ResourceType::CONFIG);
    if (!resource) {
        // 处理加载失败
        std::cerr << "Failed to load resource" << std::endl;
    }
} catch (const ResourceException& e) {
    std::cerr << "Resource error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
}
```

## 日志系统

系统提供详细的日志记录：

```cpp
// 日志级别
enum class LogLevel {
    TRACE,      // 详细跟踪信息
    DEBUG,      // 调试信息
    INFO,       // 一般信息
    WARN,       // 警告信息
    ERROR,      // 错误信息
    FATAL       // 致命错误
};

// 日志宏
FANTASY_LOG_TRACE("Loading resource: {}", path);
FANTASY_LOG_DEBUG("Resource loaded in {}ms", loadTime);
FANTASY_LOG_INFO("Cache hit rate: {}%", hitRate);
FANTASY_LOG_WARN("Resource not found: {}", path);
FANTASY_LOG_ERROR("Failed to load resource: {}", error);
```

## 扩展开发

### 自定义资源类型

```cpp
class CustomResource : public IResource {
public:
    bool load(const std::filesystem::path& path) override;
    bool unload() override;
    bool reload() override;
    
    // 自定义方法
    void customMethod();
};

class CustomResourceLoader : public IResourceLoader {
public:
    std::shared_ptr<IResource> load(const std::filesystem::path& path, ResourceType type) override;
};

// 注册自定义加载器
auto loader = std::make_shared<CustomResourceLoader>();
ResourceLoader::getInstance().registerLoader(ResourceType::CUSTOM, loader);
```

### 自定义缓存策略

```cpp
class CustomCachePolicy : public ICachePolicy {
public:
    std::string selectVictim(const std::vector<std::string>& keys) override;
    void onAccess(const std::string& key) override;
    void onInsert(const std::string& key) override;
};
```

## 版本历史

### v1.0.0 (2025.06.17)
- 初始版本发布
- 支持基本资源类型
- 实现缓存和异步加载
- 添加压缩和打包功能

### 计划功能
- 网络资源加载
- 资源热更新
- 分布式缓存
- GPU内存管理
- 资源依赖分析

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 贡献指南

欢迎提交 Issue 和 Pull Request 来改进这个项目。

## 联系方式

- 项目主页: https://github.com/fantasy-legend
- 问题反馈: https://github.com/fantasy-legend/issues
- 邮箱: pengchengkang@fantasy-legend.com 