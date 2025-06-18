/**
 * @file main.cpp
 * @brief Fantasy Legend 服务器主程序
 * @details 服务器启动、初始化和主循环
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>
#include <filesystem>

// 核心系统
#include "core/GameEngine.h"
#include "utils/config/ConfigManager.h"
#include "utils/resources/ResourceSystem.h"
#include "utils/LogSys/Logger.h"
#include "utils/LogSys/ConsoleSink.h"
#include "utils/LogSys/FileSink.h"

// 全局变量
static volatile bool g_running = true;
static std::unique_ptr<Fantasy::GameEngine> g_gameEngine;
static Fantasy::ConfigManager& g_configManager = Fantasy::ConfigManager::getInstance();
static Fantasy::ResourceSystem& g_resourceSystem = Fantasy::ResourceSystem::getInstance();

// 信号处理函数
void signalHandler(int signal) {
    (void)signal; // 避免未使用参数警告
    g_running = false;
    std::cout << "\n收到退出信号，正在关闭服务器..." << std::endl;
}

// 初始化日志系统
bool initializeLogging() {
    try {
        // 创建日志目录
        std::filesystem::create_directories("logs");
        
        // 初始化日志系统
        auto& logger = Fantasy::Logger::getInstance();
        logger.setLevel(Fantasy::LogLevel::INFO);
        
        // 添加控制台输出器
        auto consoleSink = std::make_shared<Fantasy::ConsoleSink>(false, true);
        logger.addSink(consoleSink);
        
        // 添加文件输出器
        auto fileSink = std::make_shared<Fantasy::FileSink>("logs", Fantasy::LogType::SYSTEM);
        logger.addSink(fileSink);
        
        FANTASY_LOG_INFO("=== Fantasy Legend 服务器启动 ===");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "日志系统初始化失败: " << e.what() << std::endl;
        return false;
    }
}

// 初始化配置管理器
bool initializeConfigManager() {
    try {
        if (!g_configManager.initialize()) {
            FANTASY_LOG_ERROR("Failed to initialize config manager");
            return false;
        }
        
        // 设置默认配置
        Fantasy::AppGameConfig gameConfig;
        gameConfig.version = "1.0.0";
        gameConfig.language = "zh-CN";
        gameConfig.fullscreen = false;
        gameConfig.resolution = {1920, 1080};
        gameConfig.volume = {100, 80, 90, 85};  // master, music, sfx, voice
        gameConfig.graphics = {"high", true, true, true};  // quality, shadows, antialiasing, vsync
        
        g_configManager.setGameConfig(gameConfig);
        
        Fantasy::SystemConfig systemConfig;
        systemConfig.autoSave = true;
        systemConfig.autoSaveInterval = 300; // 5分钟
        systemConfig.maxSaveSlots = 10;
        systemConfig.logLevel = "INFO";
        systemConfig.debugMode = false;
        
        g_configManager.setSystemConfig(systemConfig);
        
        FANTASY_LOG_INFO("配置管理器初始化成功");
        return true;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Config manager initialization error: {}", e.what());
        return false;
    }
}

// 初始化资源管理系统
bool initializeResourceSystem() {
    try {
        Fantasy::ResourceSystemConfig config;
        config.resourceRootDir = "resources";
        config.maxCacheSize = 100 * 1024 * 1024; // 100MB
        config.maxLoadingThreads = 4;
        config.enableLogging = true;
        config.enablePerformanceMonitoring = true;
        config.enableCompression = false;  // TODO: 实现压缩功能
        config.enablePreloading = false;   // TODO: 实现预加载功能
        
        if (!g_resourceSystem.initialize(config)) {
            FANTASY_LOG_ERROR("Failed to initialize resource system");
            return false;
        }
        
        FANTASY_LOG_INFO("资源管理系统初始化成功");
        return true;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Resource system initialization error: {}", e.what());
        return false;
    }
}

// 加载游戏资源
void loadGameResources() {
    try {
        FANTASY_LOG_INFO("开始加载游戏资源...");
        
        // 加载配置文件
        g_resourceSystem.loadResource("config/game.json", Fantasy::ResourceType::CONFIG);
        g_resourceSystem.loadResource("config/characters.json", Fantasy::ResourceType::CONFIG);
        g_resourceSystem.loadResource("config/levels.json", Fantasy::ResourceType::CONFIG);
        
        // 加载地图资源
        g_resourceSystem.loadResource("maps/village.tmx", Fantasy::ResourceType::MAP);
        g_resourceSystem.loadResource("maps/forest.tmx", Fantasy::ResourceType::MAP);
        g_resourceSystem.loadResource("maps/dungeon.tmx", Fantasy::ResourceType::MAP);
        
        // 加载纹理资源
        g_resourceSystem.loadResource("textures/characters.png", Fantasy::ResourceType::TEXTURE);
        g_resourceSystem.loadResource("textures/ui.png", Fantasy::ResourceType::TEXTURE);
        g_resourceSystem.loadResource("textures/tiles.png", Fantasy::ResourceType::TEXTURE);
        
        // 加载音频资源
        g_resourceSystem.loadResource("audio/background.ogg", Fantasy::ResourceType::MUSIC);
        g_resourceSystem.loadResource("audio/effects.ogg", Fantasy::ResourceType::SOUND);
        
        FANTASY_LOG_INFO("游戏资源加载完成");
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Resource loading error: {}", e.what());
    }
}

// 创建游戏配置示例
void createGameConfigs() {
    try {
        FANTASY_LOG_INFO("创建游戏配置示例...");
        
        // 创建游戏配置
        Fantasy::AppGameConfig gameConfig;
        gameConfig.version = "1.0.0";
        gameConfig.language = "zh-CN";
        gameConfig.fullscreen = false;
        gameConfig.resolution = {1920, 1080};
        gameConfig.volume = {100, 80, 90, 85};  // 使用整数而不是浮点数
        gameConfig.graphics = {"high", true, true, true};  // 使用正确的结构初始化
        
        g_configManager.setGameConfig(gameConfig);
        
        // 创建系统配置
        Fantasy::SystemConfig systemConfig;
        systemConfig.autoSave = true;
        systemConfig.autoSaveInterval = 300;
        systemConfig.maxSaveSlots = 10;
        systemConfig.logLevel = "INFO";
        systemConfig.debugMode = false;
        
        g_configManager.setSystemConfig(systemConfig);
        
        // 保存配置到文件
        g_configManager.saveConfig("game", Fantasy::ConfigLevel::APPLICATION);
        g_configManager.saveConfig("system", Fantasy::ConfigLevel::SYSTEM);
        
        FANTASY_LOG_INFO("游戏配置创建完成");
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Game config creation error: {}", e.what());
    }
}

// 显示系统状态
void displaySystemStatus() {
    try {
        FANTASY_LOG_INFO("=== 系统状态 ===");
        
        // 资源系统状态
        auto resourceStats = g_resourceSystem.getStats();
        FANTASY_LOG_INFO("资源系统:");
        FANTASY_LOG_INFO("  总资源数: {}", resourceStats.totalResources);
        FANTASY_LOG_INFO("  已加载资源: {}", resourceStats.loadedResources);
        FANTASY_LOG_INFO("  缓存资源: {}", resourceStats.cachedResources);
        FANTASY_LOG_INFO("  内存使用: {} bytes", resourceStats.totalMemoryUsage);
        FANTASY_LOG_INFO("  缓存命中率: {:.2f}%", resourceStats.cacheHitRate * 100);
        
        // 配置管理器状态
        auto configStats = g_configManager.getStats();
        FANTASY_LOG_INFO("配置管理器:");
        FANTASY_LOG_INFO("  总配置数: {}", configStats.totalConfigs);
        FANTASY_LOG_INFO("  已加载配置: {}", configStats.loadedConfigs);
        FANTASY_LOG_INFO("  已修改配置: {}", configStats.modifiedConfigs);
        FANTASY_LOG_INFO("  监听器数量: {}", configStats.listenersCount);
        
        // 游戏配置
        auto gameConfig = g_configManager.getGameConfig();
        FANTASY_LOG_INFO("游戏配置:");
        FANTASY_LOG_INFO("  版本: {}", gameConfig.version);
        FANTASY_LOG_INFO("  语言: {}", gameConfig.language);
        FANTASY_LOG_INFO("  分辨率: {}x{}", gameConfig.resolution.width, gameConfig.resolution.height);
        FANTASY_LOG_INFO("  音量: 主音量={}, 音乐={}, 音效={}, 语音={}", 
                gameConfig.volume.master, gameConfig.volume.music, 
                gameConfig.volume.sfx, gameConfig.volume.voice);
        
        auto systemConfig = g_configManager.getSystemConfig();
        FANTASY_LOG_INFO("系统配置:");
        FANTASY_LOG_INFO("  自动保存: {}", systemConfig.autoSave ? "开启" : "关闭");
        FANTASY_LOG_INFO("  自动保存间隔: {} 秒", systemConfig.autoSaveInterval);
        FANTASY_LOG_INFO("  最大存档槽位: {}", systemConfig.maxSaveSlots);
        FANTASY_LOG_INFO("  日志级别: {}", systemConfig.logLevel);
        FANTASY_LOG_INFO("  调试模式: {}", systemConfig.debugMode ? "开启" : "关闭");
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Status display error: {}", e.what());
    }
}

// 主服务器循环
void serverLoop() {
    FANTASY_LOG_INFO("服务器主循环启动");
    
    auto lastStatusTime = std::chrono::steady_clock::now();
    auto lastSaveTime = std::chrono::steady_clock::now();
    
    while (g_running) {
        try {
            auto now = std::chrono::steady_clock::now();
            
            // 每30秒显示一次状态
            if (now - lastStatusTime >= std::chrono::seconds(30)) {
                displaySystemStatus();
                lastStatusTime = now;
            }
            
            // 每5分钟自动保存
            if (now - lastSaveTime >= std::chrono::minutes(5)) {
                g_configManager.saveConfig("game", Fantasy::ConfigLevel::APPLICATION);
                g_configManager.saveConfig("system", Fantasy::ConfigLevel::SYSTEM);
                lastSaveTime = now;
            }
            
            // 处理资源系统更新 - 移除不存在的update方法调用
            // g_resourceSystem.update();
            
            // 短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            FANTASY_LOG_ERROR("Server loop error: {}", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

// 清理资源
void cleanup() {
    try {
        FANTASY_LOG_INFO("开始清理资源...");
        
        // 保存配置
        g_configManager.saveConfig("game", Fantasy::ConfigLevel::APPLICATION);
        g_configManager.saveConfig("system", Fantasy::ConfigLevel::SYSTEM);
        
        // 清理资源系统
        g_resourceSystem.shutdown();
        
        // 清理游戏引擎
        if (g_gameEngine) {
            g_gameEngine->shutdown();
        }
        
        FANTASY_LOG_INFO("资源清理完成");
        
    } catch (const std::exception& e) {
        std::cerr << "清理过程中发生错误: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    (void)argc; // 避免未使用参数警告
    (void)argv; // 避免未使用参数警告
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
    // 初始化日志系统
        if (!initializeLogging()) {
            std::cerr << "日志系统初始化失败" << std::endl;
            return 1;
        }
        
        // 初始化配置管理器
        if (!initializeConfigManager()) {
            FANTASY_LOG_ERROR("配置管理器初始化失败");
            return 1;
        }
        
        // 初始化资源管理系统
        if (!initializeResourceSystem()) {
            FANTASY_LOG_ERROR("资源管理系统初始化失败");
            return 1;
        }
        
        // 加载游戏资源
        loadGameResources();
        
        // 创建游戏配置
        createGameConfigs();
        
        // 显示初始状态
        displaySystemStatus();
        
        // 启动服务器主循环
        serverLoop();
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }
    
    // 清理资源
    cleanup();
    
    FANTASY_LOG_INFO("服务器正常关闭");
    return 0;
}