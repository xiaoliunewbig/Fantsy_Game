/**
 * @file ResourceSystemExample.cpp
 * @brief 资源系统使用示例
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include "utils/resources/ResourceSystem.h"
#include "utils/LogSys/Logger.h"
#include "utils/LogSys/ConsoleSink.h"
#include "utils/LogSys/FileSink.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Fantasy;

void resourceSystemExample() {
    std::cout << "=== Fantasy Legend 资源系统示例 ===" << std::endl;
    
    // 1. 初始化日志系统
    std::cout << "\n1. 初始化日志系统..." << std::endl;
    auto& logger = Logger::getInstance();
    logger.addSink(std::make_shared<ConsoleSink>());
    logger.addSink(std::make_shared<FileSink>("logs", LogType::PROGRAM));
    
    FANTASY_LOG_INFO("资源系统示例开始");
    
    // 2. 配置资源系统
    std::cout << "\n2. 配置资源系统..." << std::endl;
    ResourceSystemConfig config;
    config.resourceRootDir = "resources/";
    config.maxCacheSize = 100 * 1024 * 1024; // 100MB
    config.maxLoadingThreads = 4;
    config.enableLogging = true;
    config.enablePerformanceMonitoring = true;
    config.enableCompression = false;  // TODO: 实现压缩功能
    config.enablePreloading = false;   // TODO: 实现预加载功能
    
    // 3. 初始化资源系统
    std::cout << "\n3. 初始化资源系统..." << std::endl;
    auto& resourceSystem = ResourceSystem::getInstance();
    if (!resourceSystem.initialize(config)) {
        FANTASY_LOG_ERROR("资源系统初始化失败");
        return;
    }
    
    // 4. 加载资源示例
    std::cout << "\n4. 加载资源示例..." << std::endl;
    
    // TODO: 实现具体的资源加载
    // auto configResource = resourceSystem.loadResource("config/game.cfg", ResourceType::CONFIG);
    // auto mapResource = resourceSystem.loadResource("maps/level1.map", ResourceType::MAP);
    // auto scriptResource = resourceSystem.loadResource("scripts/main.py", ResourceType::SCRIPT);
    // auto textureResource = resourceSystem.loadResource("textures/player.png", ResourceType::TEXTURE);
    // auto modelResource = resourceSystem.loadResource("models/character.obj", ResourceType::MODEL);
    // auto audioResource = resourceSystem.loadResource("audio/background_music.mp3", ResourceType::AUDIO);
    
    std::cout << "资源加载示例完成（功能待实现）" << std::endl;
    
    // 5. 异步加载示例
    std::cout << "\n5. 异步加载示例..." << std::endl;
    
    // TODO: 实现异步资源加载
    // std::vector<std::string> asyncResources = {
    //     "textures/ui/button.png",
    //     "audio/sfx/click.wav",
    //     "scripts/ai/basic_ai.lua"
    // };
    // 
    // int completedCount = 0;
    // for (const auto& path : asyncResources) {
    //     ResourceType type = ResourceSystem::inferResourceType(path);
    //     resourceSystem.loadResourceAsync(path, type, [&completedCount, path](std::shared_ptr<IResource> resource) {
    //         if (resource) {
    //             std::cout << "异步加载成功: " << path << std::endl;
    //         } else {
    //             std::cout << "异步加载失败: " << path << std::endl;
    //         }
    //         completedCount++;
    //     });
    // }
    // 
    // // 等待所有异步加载完成
    // while (completedCount < asyncResources.size()) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }
    
    std::cout << "异步加载示例完成（功能待实现）" << std::endl;
    
    // 6. 预加载示例
    std::cout << "\n6. 预加载示例..." << std::endl;
    
    // TODO: 实现资源预加载
    // std::vector<std::string> preloadResources = {
    //     "textures/terrain/grass.png",
    //     "textures/terrain/stone.png",
    //     "textures/terrain/water.png",
    //     "models/environment/tree.obj",
    //     "models/environment/rock.obj"
    // };
    // 
    // resourceSystem.preloadResources(preloadResources, PreloadPriority::HIGH);
    
    std::cout << "预加载示例完成（功能待实现）" << std::endl;
    
    // 7. 系统统计
    std::cout << "\n7. 系统统计..." << std::endl;
    
    auto stats = resourceSystem.getStats();
    std::cout << "总资源数: " << stats.totalResources << std::endl;
    std::cout << "已加载资源: " << stats.loadedResources << std::endl;
    std::cout << "缓存资源: " << stats.cachedResources << std::endl;
    std::cout << "内存使用: " << (stats.totalMemoryUsage / (1024 * 1024)) << "MB" << std::endl;
    std::cout << "缓存命中率: " << (stats.cacheHitRate * 100) << "%" << std::endl;
    
    // 8. 系统优化
    std::cout << "\n8. 系统优化..." << std::endl;
    
    if (resourceSystem.optimize()) {
        std::cout << "系统优化完成" << std::endl;
    } else {
        std::cout << "系统优化失败" << std::endl;
    }
    
    // 9. 清理未使用资源
    std::cout << "\n9. 清理未使用资源..." << std::endl;
    
    uint64_t cleanedCount = resourceSystem.cleanupUnusedResources();
    std::cout << "清理了 " << cleanedCount << " 个未使用的资源" << std::endl;
    
    // 10. 完整性验证
    std::cout << "\n10. 完整性验证..." << std::endl;
    
    if (resourceSystem.verifyIntegrity()) {
        std::cout << "资源完整性验证通过" << std::endl;
    } else {
        std::cout << "资源完整性验证失败" << std::endl;
    }
    
    // 11. 关闭资源系统
    std::cout << "\n11. 关闭资源系统..." << std::endl;
    
    resourceSystem.shutdown();
    
    FANTASY_LOG_INFO("资源系统示例完成");
    std::cout << "\n=== 资源系统示例完成 ===" << std::endl;
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