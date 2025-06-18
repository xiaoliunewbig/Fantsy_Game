/**
 * @file ConfigManagerExample.cpp
 * @brief 配置管理器使用示例
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 展示配置管理器与资源管理和日志系统的集成使用
 */

#include "include/utils/config/ConfigManager.h"
#include "include/utils/resources/ResourceSystem.h"
#include "include/utils/LogSys/Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Fantasy;

int main() {
    std::cout << "=== Fantasy Legend 配置管理器示例 ===" << std::endl;
    
    // 1. 初始化日志系统
    std::cout << "\n1. 初始化日志系统..." << std::endl;
    auto& logger = Logger::getInstance();
    logger.initialize();
    logger.addSink(std::make_shared<ConsoleSink>());
    logger.addSink(std::make_shared<FileSink>("logs/config_example.log"));
    
    FANTASY_LOG_INFO("配置管理器示例开始");
    
    // 2. 初始化配置管理器
    std::cout << "\n2. 初始化配置管理器..." << std::endl;
    auto& configManager = ConfigManager::getInstance();
    if (!configManager.initialize("config/")) {
        FANTASY_LOG_ERROR("配置管理器初始化失败");
        return -1;
    }
    
    // 3. 初始化资源管理系统
    std::cout << "\n3. 初始化资源管理系统..." << std::endl;
    ResourceSystemConfig resourceConfig;
    resourceConfig.resourceRootDir = "resources/";
    resourceConfig.maxCacheSize = 100 * 1024 * 1024; // 100MB
    resourceConfig.loadingThreads = 4;
    resourceConfig.enableLogging = true;
    resourceConfig.enablePerformanceMonitoring = true;
    
    auto& resourceSystem = ResourceSystem::getInstance();
    if (!resourceSystem.initialize(resourceConfig)) {
        FANTASY_LOG_ERROR("资源管理系统初始化失败");
        return -1;
    }
    
    // 4. 创建示例配置文件
    std::cout << "\n4. 创建示例配置文件..." << std::endl;
    std::filesystem::create_directories("config");
    
    // 创建游戏配置文件
    std::ofstream gameConfig("config/game.cfg");
    gameConfig << "server.ip=127.0.0.1\n";
    gameConfig << "server.port=8080\n";
    gameConfig << "game.max_players=100\n";
    gameConfig << "game.debug_mode=true\n";
    gameConfig << "graphics.resolution=1920x1080\n";
    gameConfig << "audio.master_volume=0.8\n";
    gameConfig << "audio.music_volume=0.6\n";
    gameConfig << "audio.sfx_volume=0.7\n";
    gameConfig.close();
    
    // 创建资源配置文件
    std::ofstream resourceConfigFile("config/resources.cfg");
    resourceConfigFile << "texture.quality=high\n";
    resourceConfigFile << "model.lod_level=2\n";
    resourceConfigFile << "audio.format=ogg\n";
    resourceConfigFile << "compression.enabled=true\n";
    resourceConfigFile << "compression.level=6\n";
    resourceConfigFile << "cache.max_size=100\n";
    resourceConfigFile.close();
    
    FANTASY_LOG_INFO("示例配置文件创建完成");
    
    // 5. 加载配置文件
    std::cout << "\n5. 加载配置文件..." << std::endl;
    if (!configManager.loadConfig("game")) {
        FANTASY_LOG_ERROR("游戏配置文件加载失败");
        return -1;
    }
    
    if (!configManager.loadConfig("resources")) {
        FANTASY_LOG_ERROR("资源配置文件加载失败");
        return -1;
    }
    
    // 6. 读取配置值
    std::cout << "\n6. 读取配置值..." << std::endl;
    
    // 使用宏读取配置
    std::string serverIP = CONFIG_GET_VALUE("server.ip", "127.0.0.1");
    int serverPort = CONFIG_GET_VALUE("server.port", 8080);
    int maxPlayers = CONFIG_GET_VALUE("game.max_players", 50);
    bool debugMode = CONFIG_GET_VALUE("game.debug_mode", false);
    std::string resolution = CONFIG_GET_VALUE("graphics.resolution", "1920x1080");
    double masterVolume = CONFIG_GET_VALUE("audio.master_volume", 1.0);
    
    std::cout << "服务器IP: " << serverIP << std::endl;
    std::cout << "服务器端口: " << serverPort << std::endl;
    std::cout << "最大玩家数: " << maxPlayers << std::endl;
    std::cout << "调试模式: " << (debugMode ? "开启" : "关闭") << std::endl;
    std::cout << "分辨率: " << resolution << std::endl;
    std::cout << "主音量: " << masterVolume << std::endl;
    
    // 7. 配置变更监听
    std::cout << "\n7. 设置配置变更监听..." << std::endl;
    
    configManager.addChangeListener("audio.master_volume", [](const ConfigChangeEvent& event) {
        double oldVolume = ConfigUtils::toDouble(event.oldValue, 1.0);
        double newVolume = ConfigUtils::toDouble(event.newValue, 1.0);
        FANTASY_LOG_INFO("主音量变更: {} -> {}", oldVolume, newVolume);
        std::cout << "主音量已变更: " << oldVolume << " -> " << newVolume << std::endl;
    });
    
    configManager.addChangeListener("game.debug_mode", [](const ConfigChangeEvent& event) {
        bool oldDebug = ConfigUtils::toBool(event.oldValue, false);
        bool newDebug = ConfigUtils::toBool(event.newValue, false);
        FANTASY_LOG_INFO("调试模式变更: {} -> {}", oldDebug, newDebug);
        std::cout << "调试模式已变更: " << (oldDebug ? "开启" : "关闭") 
                  << " -> " << (newDebug ? "开启" : "关闭") << std::endl;
    });
    
    // 8. 修改配置值
    std::cout << "\n8. 修改配置值..." << std::endl;
    
    CONFIG_SET_VALUE("audio.master_volume", 0.9);
    CONFIG_SET_VALUE("game.debug_mode", true);
    CONFIG_SET_VALUE("server.port", 9090);
    
    // 9. 配置验证
    std::cout << "\n9. 配置验证..." << std::endl;
    
    // 验证IP地址
    if (!ConfigUtils::isValidIP(serverIP)) {
        FANTASY_LOG_WARN("无效的服务器IP地址: {}", serverIP);
    }
    
    // 验证端口号
    if (serverPort < 1 || serverPort > 65535) {
        FANTASY_LOG_WARN("无效的端口号: {}", serverPort);
    }
    
    // 验证音量
    if (masterVolume < 0.0 || masterVolume > 1.0) {
        FANTASY_LOG_WARN("无效的音量值: {}", masterVolume);
    }
    
    // 10. 配置与资源系统集成
    std::cout << "\n10. 配置与资源系统集成..." << std::endl;
    
    // 根据配置加载资源
    std::string textureQuality = CONFIG_GET_VALUE("texture.quality", "medium");
    bool compressionEnabled = CONFIG_GET_VALUE("compression.enabled", false);
    int compressionLevel = CONFIG_GET_VALUE("compression.level", 6);
    
    std::cout << "纹理质量: " << textureQuality << std::endl;
    std::cout << "压缩启用: " << (compressionEnabled ? "是" : "否") << std::endl;
    std::cout << "压缩级别: " << compressionLevel << std::endl;
    
    // 根据配置调整资源系统
    if (compressionEnabled) {
        resourceSystem.setCompressionLevel(compressionLevel);
        FANTASY_LOG_INFO("资源压缩已启用，级别: {}", compressionLevel);
    }
    
    // 11. 热重载测试
    std::cout << "\n11. 热重载测试..." << std::endl;
    
    configManager.enableHotReload(true);
    std::cout << "热重载已启用" << std::endl;
    
    // 模拟配置文件变更
    std::ofstream updatedConfig("config/game.cfg");
    updatedConfig << "server.ip=192.168.1.100\n";
    updatedConfig << "server.port=9090\n";
    updatedConfig << "game.max_players=200\n";
    updatedConfig << "game.debug_mode=false\n";
    updatedConfig << "graphics.resolution=2560x1440\n";
    updatedConfig << "audio.master_volume=0.5\n";
    updatedConfig << "audio.music_volume=0.4\n";
    updatedConfig << "audio.sfx_volume=0.6\n";
    updatedConfig.close();
    
    // 重新加载配置
    if (configManager.reloadConfig("game")) {
        std::cout << "配置文件已重新加载" << std::endl;
        
        // 读取更新后的值
        std::string newServerIP = CONFIG_GET_VALUE("server.ip", "127.0.0.1");
        int newServerPort = CONFIG_GET_VALUE("server.port", 8080);
        std::string newResolution = CONFIG_GET_VALUE("graphics.resolution", "1920x1080");
        
        std::cout << "更新后的服务器IP: " << newServerIP << std::endl;
        std::cout << "更新后的服务器端口: " << newServerPort << std::endl;
        std::cout << "更新后的分辨率: " << newResolution << std::endl;
    }
    
    // 12. 统计信息
    std::cout << "\n12. 统计信息..." << std::endl;
    
    auto stats = configManager.getStats();
    std::cout << "总配置数: " << stats.totalConfigs << std::endl;
    std::cout << "已加载配置: " << stats.loadedConfigs << std::endl;
    std::cout << "已修改配置: " << stats.modifiedConfigs << std::endl;
    std::cout << "监听器数量: " << stats.listenersCount << std::endl;
    
    // 13. 保存配置
    std::cout << "\n13. 保存配置..." << std::endl;
    
    if (configManager.saveConfig("game")) {
        FANTASY_LOG_INFO("游戏配置已保存");
        std::cout << "游戏配置已保存" << std::endl;
    }
    
    if (configManager.saveConfig("resources")) {
        FANTASY_LOG_INFO("资源配置已保存");
        std::cout << "资源配置已保存" << std::endl;
    }
    
    // 14. 清理资源
    std::cout << "\n14. 清理资源..." << std::endl;
    
    configManager.shutdown();
    resourceSystem.shutdown();
    logger.shutdown();
    
    FANTASY_LOG_INFO("配置管理器示例完成");
    std::cout << "\n=== 配置管理器示例完成 ===" << std::endl;
    
    return 0;
} 