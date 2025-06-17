/**
 * @file ResourceSystemExample.cpp
 * @brief 资源管理系统使用示例
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/resources/ResourceSystem.h"
#include "utils/LogSys/Logger.h"
#include "utils/LogSys/ConsoleSink.h"
#include "utils/LogSys/FileSink.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Fantasy;

/**
 * @brief 资源管理系统使用示例
 */
void resourceSystemExample() {
    // 1. 初始化日志系统
    auto& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);
    
    // 添加控制台输出
    logger.addSink(std::make_shared<ConsoleSink>());
    
    // 添加文件日志输出
    logger.addSink(std::make_shared<FileSink>("logs/resources.log"));
    
    FANTASY_LOG_INFO("=== Resource System Example ===");
    
    // 2. 配置资源系统
    ResourceSystemConfig config;
    config.resourceRootDir = "resources";
    config.maxCacheSize = 512 * 1024 * 1024; // 512MB
    config.maxLoadingThreads = 4;
    config.cacheExpirationTime = 1800; // 30分钟
    config.enableLogging = true;
    config.enablePerformanceLogging = true;
    config.enableCompression = true;
    config.enablePackaging = true;
    config.enableVersionControl = true;
    config.enablePreloading = true;
    config.enablePerformanceMonitoring = true;
    config.monitoringInterval = 5000; // 5秒
    
    // 3. 初始化资源系统
    auto& resourceSystem = ResourceSystem::getInstance();
    if (!resourceSystem.initialize(config)) {
        FANTASY_LOG_ERROR("Failed to initialize ResourceSystem");
        return;
    }
    
    FANTASY_LOG_INFO("ResourceSystem initialized successfully");
    
    // 4. 同步加载资源示例
    FANTASY_LOG_INFO("--- Synchronous Loading Example ---");
    
    // 加载纹理资源
    auto texture = resourceSystem.loadResource("textures/player.png", ResourceType::TEXTURE);
    if (texture) {
        FANTASY_LOG_INFO("Loaded texture: {} (size: {} bytes)", 
                        texture->getId(), texture->getSize());
    }
    
    // 加载音频资源
    auto audio = resourceSystem.loadResource("audio/background_music.mp3", ResourceType::AUDIO);
    if (audio) {
        FANTASY_LOG_INFO("Loaded audio: {} (size: {} bytes)", 
                        audio->getId(), audio->getSize());
    }
    
    // 5. 异步加载资源示例
    FANTASY_LOG_INFO("--- Asynchronous Loading Example ---");
    
    std::vector<std::string> asyncResources = {
        "textures/enemy.png",
        "audio/explosion.wav",
        "fonts/main.ttf",
        "models/character.obj"
    };
    
    int completedCount = 0;
    for (const auto& path : asyncResources) {
        ResourceType type = ResourceSystem::inferResourceType(path);
        resourceSystem.loadResourceAsync(path, type, [&completedCount, path](std::shared_ptr<IResource> resource) {
            if (resource) {
                FANTASY_LOG_INFO("Async loaded: {} (size: {} bytes)", path, resource->getSize());
            } else {
                FANTASY_LOG_ERROR("Async load failed: {}", path);
            }
            completedCount++;
        });
    }
    
    // 等待异步加载完成
    while (completedCount < asyncResources.size()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 6. 预加载资源示例
    FANTASY_LOG_INFO("--- Preloading Example ---");
    
    std::vector<std::string> preloadResources = {
        "textures/ui/button.png",
        "textures/ui/background.jpg",
        "audio/ui/click.wav",
        "fonts/ui.ttf"
    };
    
    resourceSystem.preloadResources(preloadResources, PreloadPriority::NORMAL);
    
    // 7. 资源包操作示例
    FANTASY_LOG_INFO("--- Package Operations Example ---");
    
    std::vector<std::string> packageResources = {
        "textures/player.png",
        "audio/background_music.mp3",
        "fonts/main.ttf"
    };
    
    if (resourceSystem.createResourcePackage("game_resources.pak", packageResources)) {
        FANTASY_LOG_INFO("Created resource package: game_resources.pak");
        
        if (resourceSystem.openResourcePackage("game_resources.pak")) {
            FANTASY_LOG_INFO("Opened resource package successfully");
        }
    }
    
    // 8. 版本控制示例
    FANTASY_LOG_INFO("--- Version Control Example ---");
    
    Version version(1, 0, 0, "alpha");
    if (resourceSystem.addResourceVersion("textures/player.png", version)) {
        FANTASY_LOG_INFO("Added version {} for player texture", version.toString());
    }
    
    // 9. 压缩示例
    FANTASY_LOG_INFO("--- Compression Example ---");
    
    if (resourceSystem.compressResource("textures/player.png", CompressionAlgorithm::ZSTD)) {
        FANTASY_LOG_INFO("Compressed player texture");
    }
    
    // 10. 获取统计信息
    FANTASY_LOG_INFO("--- Statistics Example ---");
    
    auto stats = resourceSystem.getStats();
    FANTASY_LOG_INFO("Resource System Statistics:");
    FANTASY_LOG_INFO("  Total Resources: {}", stats.totalResources);
    FANTASY_LOG_INFO("  Loaded Resources: {}", stats.loadedResources);
    FANTASY_LOG_INFO("  Cached Resources: {}", stats.cachedResources);
    FANTASY_LOG_INFO("  Memory Usage: {}MB", stats.totalMemoryUsage / (1024 * 1024));
    FANTASY_LOG_INFO("  Cache Hit Rate: {}%", static_cast<int>(stats.cacheHitRate * 100));
    
    // 11. 性能报告
    FANTASY_LOG_INFO("--- Performance Report ---");
    std::string report = resourceSystem.getPerformanceReport();
    FANTASY_LOG_INFO("Performance Report:\n{}", report);
    
    // 12. 优化和清理
    FANTASY_LOG_INFO("--- Optimization Example ---");
    
    if (resourceSystem.optimize()) {
        FANTASY_LOG_INFO("Resource system optimized");
    }
    
    uint64_t cleanedCount = resourceSystem.cleanupUnusedResources();
    FANTASY_LOG_INFO("Cleaned up {} unused resources", cleanedCount);
    
    // 13. 完整性验证
    FANTASY_LOG_INFO("--- Integrity Verification ---");
    
    if (resourceSystem.verifyIntegrity()) {
        FANTASY_LOG_INFO("Resource system integrity verified");
    } else {
        FANTASY_LOG_ERROR("Resource system integrity check failed");
    }
    
    // 14. 便捷函数使用示例
    FANTASY_LOG_INFO("--- Utility Functions Example ---");
    
    using namespace ResourceSystemUtils;
    
    auto quickTexture = load("textures/quick.png", ResourceType::TEXTURE);
    if (quickTexture) {
        FANTASY_LOG_INFO("Quick loaded texture: {}", quickTexture->getId());
    }
    
    auto retrievedResource = get("textures/player.png_0");
    if (retrievedResource) {
        FANTASY_LOG_INFO("Retrieved resource: {}", retrievedResource->getId());
    }
    
    // 15. 等待一段时间以观察性能监控
    FANTASY_LOG_INFO("--- Performance Monitoring ---");
    FANTASY_LOG_INFO("Monitoring for 10 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 16. 关闭资源系统
    FANTASY_LOG_INFO("--- Shutdown Example ---");
    resourceSystem.shutdown();
    
    FANTASY_LOG_INFO("Resource System Example completed");
}

/**
 * @brief 主函数示例
 */
int main() {
    try {
        resourceSystemExample();
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception in resource system example: {}", e.what());
        return 1;
    }
    
    return 0;
} 