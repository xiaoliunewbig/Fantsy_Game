/**
 * @file main.cpp
 * @brief Fantasy Legend 服务器主程序
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>

// 包含核心系统头文件
#include "include/utils/resources/ResourceSystem.h"
#include "include/utils/config/ConfigManager.h"
#include "include/utils/resources/ResourceLogger.h"

// 全局变量用于信号处理
std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\n收到信号 " << signal << "，正在关闭服务器..." << std::endl;
    g_running = false;
}

// 初始化日志系统
bool initializeLogging() {
    try {
        auto& logger = Fantasy::ResourceLogger::getInstance();
        if (!logger.initialize("logs/server.log")) {
            std::cerr << "Failed to initialize logging system" << std::endl;
            return false;
        }
        
        LOG_INFO("=== Fantasy Legend 服务器启动 ===");
        LOG_INFO("日志系统初始化成功");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Logging initialization error: " << e.what() << std::endl;
        return false;
    }
}

// 初始化配置管理器
bool initializeConfigManager() {
    try {
        auto& configManager = Fantasy::ConfigManager::getInstance();
        if (!configManager.initialize("config")) {
            LOG_ERROR("Failed to initialize config manager");
            return false;
        }
        
        // 设置默认配置
        Fantasy::GameConfig gameConfig;
        gameConfig.version = "1.0.0";
        gameConfig.language = "zh_CN";
        gameConfig.fullscreen = false;
        gameConfig.resolution.width = 1920;
        gameConfig.resolution.height = 1080;
        gameConfig.volume.master = 100;
        gameConfig.volume.music = 80;
        gameConfig.volume.sfx = 90;
        gameConfig.volume.voice = 85;
        gameConfig.graphics.quality = "high";
        gameConfig.graphics.shadows = true;
        gameConfig.graphics.antialiasing = true;
        gameConfig.graphics.vsync = true;
        
        configManager.setGameConfig(gameConfig);
        
        Fantasy::SystemConfig systemConfig;
        systemConfig.autoSave = true;
        systemConfig.autoSaveInterval = 300;
        systemConfig.maxSaveSlots = 10;
        systemConfig.logLevel = "info";
        systemConfig.debugMode = false;
        
        configManager.setSystemConfig(systemConfig);
        
        LOG_INFO("配置管理器初始化成功");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config manager initialization error: {}", e.what());
        return false;
    }
}

// 初始化资源管理系统
bool initializeResourceSystem() {
    try {
        auto& resourceSystem = Fantasy::ResourceSystem::getInstance();
        
        Fantasy::ResourceSystemConfig config;
        config.resourceRootDir = "resources";
        config.maxCacheSize = 1024 * 1024 * 100; // 100MB
        config.enableLogging = true;
        config.enablePerformanceMonitoring = true;
        config.enableCompression = true;
        config.enableVersionControl = true;
        config.enablePackaging = true;
        config.enablePreloading = true;
        config.threadCount = 4;
        config.compressionLevel = 6;
        config.preloadPriority = Fantasy::PreloadPriority::HIGH;
        
        if (!resourceSystem.initialize(config)) {
            LOG_ERROR("Failed to initialize resource system");
            return false;
        }
        
        LOG_INFO("资源管理系统初始化成功");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Resource system initialization error: {}", e.what());
        return false;
    }
}

// 加载游戏资源
void loadGameResources() {
    try {
        auto& resourceSystem = Fantasy::ResourceSystem::getInstance();
        
        LOG_INFO("开始加载游戏资源...");
        
        // 加载配置文件
        auto configResource = resourceSystem.getResourceManager().load("config/game.cfg", Fantasy::ResourceType::CONFIG);
        if (configResource) {
            LOG_INFO("游戏配置文件加载成功");
        }
        
        // 加载地图资源
        auto mapResource = resourceSystem.getResourceManager().load("maps/level1.map", Fantasy::ResourceType::MAP);
        if (mapResource) {
            LOG_INFO("地图资源加载成功");
        }
        
        // 加载脚本资源
        auto scriptResource = resourceSystem.getResourceManager().load("scripts/main.lua", Fantasy::ResourceType::SCRIPT);
        if (scriptResource) {
            LOG_INFO("脚本资源加载成功");
        }
        
        // 异步加载纹理资源
        resourceSystem.getResourceLoader().loadAsync("textures/player.png", Fantasy::ResourceType::TEXTURE, 
            [](const std::shared_ptr<Fantasy::Resource>& resource) {
                if (resource) {
                    LOG_INFO("纹理资源异步加载成功: {}", resource->getPath());
                } else {
                    LOG_ERROR("纹理资源异步加载失败");
                }
            });
        
        // 异步加载音频资源
        resourceSystem.getResourceLoader().loadAsync("audio/background.mp3", Fantasy::ResourceType::MUSIC,
            [](const std::shared_ptr<Fantasy::Resource>& resource) {
                if (resource) {
                    LOG_INFO("音频资源异步加载成功: {}", resource->getPath());
                } else {
                    LOG_ERROR("音频资源异步加载失败");
                }
            });
        
        LOG_INFO("游戏资源加载完成");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Resource loading error: {}", e.what());
    }
}

// 创建游戏配置示例
void createGameConfigs() {
    try {
        auto& configManager = Fantasy::ConfigManager::getInstance();
        
        LOG_INFO("创建游戏配置示例...");
        
        // 创建角色配置
        auto characterConfig = configManager.generateCharacterConfig("warrior", 10);
        characterConfig["name"] = "战士";
        characterConfig["skills"] = std::vector<std::string>{"slash", "shield_bash", "charge"};
        configManager.saveCharacterConfig("warrior", 10, characterConfig);
        
        // 创建关卡配置
        auto levelConfig = configManager.generateLevelConfig(1, 1.0f);
        levelConfig["name"] = "新手村";
        levelConfig["enemies"] = std::vector<std::string>{"goblin", "wolf"};
        levelConfig["rewards"] = std::vector<std::string>{"gold", "experience"};
        configManager.saveLevelConfig(1, levelConfig);
        
        // 创建物品配置
        auto itemConfig = configManager.generateItemConfig("sword", 5);
        itemConfig["name"] = "精钢剑";
        itemConfig["rarity"] = "rare";
        itemConfig["effects"] = std::vector<std::string>{"damage_boost", "durability"};
        configManager.saveItemConfig("sword_5", itemConfig);
        
        // 创建技能配置
        auto skillConfig = configManager.generateSkillConfig("fireball", 3);
        skillConfig["name"] = "火球术";
        skillConfig["type"] = "magic";
        skillConfig["effects"] = std::vector<std::string>{"burn", "area_damage"};
        configManager.saveSkillConfig("fireball_3", skillConfig);
        
        // 创建任务配置
        std::unordered_map<std::string, Fantasy::ConfigValue> questConfig;
        questConfig["id"] = "quest_001";
        questConfig["name"] = "消灭哥布林";
        questConfig["description"] = "消灭10只哥布林";
        questConfig["type"] = "kill";
        questConfig["target"] = "goblin";
        questConfig["count"] = 10;
        questConfig["reward_gold"] = 100;
        questConfig["reward_exp"] = 50;
        configManager.saveQuestConfig("quest_001", questConfig);
        
        LOG_INFO("游戏配置示例创建完成");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Game config creation error: {}", e.what());
    }
}

// 显示系统状态
void displaySystemStatus() {
    try {
        auto& resourceSystem = Fantasy::ResourceSystem::getInstance();
        auto& configManager = Fantasy::ConfigManager::getInstance();
        
        LOG_INFO("=== 系统状态 ===");
        
        // 资源系统状态
        auto resourceStats = resourceSystem.getResourceManager().getStats();
        LOG_INFO("资源管理器状态:");
        LOG_INFO("  缓存大小: {} bytes", resourceStats.cacheSize);
        LOG_INFO("  缓存命中率: {:.2f}%", resourceStats.cacheHitRate * 100);
        LOG_INFO("  加载的资源数量: {}", resourceStats.loadedResources);
        LOG_INFO("  总加载时间: {} ms", resourceStats.totalLoadTime);
        
        auto loaderStats = resourceSystem.getResourceLoader().getStats();
        LOG_INFO("资源加载器状态:");
        LOG_INFO("  队列大小: {}", loaderStats.queueSize);
        LOG_INFO("  活跃线程数: {}", loaderStats.activeThreads);
        LOG_INFO("  总加载任务: {}", loaderStats.totalTasks);
        LOG_INFO("  成功加载: {}", loaderStats.successfulTasks);
        
        // 配置管理器状态
        auto configStats = configManager.getStats();
        LOG_INFO("配置管理器状态:");
        LOG_INFO("  已加载配置: {}", configStats.loadedConfigs);
        LOG_INFO("  已保存配置: {}", configStats.savedConfigs);
        LOG_INFO("  配置错误: {}", configStats.configErrors);
        
        // 游戏配置
        auto gameConfig = configManager.getGameConfig();
        LOG_INFO("游戏配置:");
        LOG_INFO("  版本: {}", gameConfig.version);
        LOG_INFO("  语言: {}", gameConfig.language);
        LOG_INFO("  分辨率: {}x{}", gameConfig.resolution.width, gameConfig.resolution.height);
        LOG_INFO("  音量: 主音量={}, 音乐={}, 音效={}, 语音={}", 
                gameConfig.volume.master, gameConfig.volume.music, 
                gameConfig.volume.sfx, gameConfig.volume.voice);
        
        auto systemConfig = configManager.getSystemConfig();
        LOG_INFO("系统配置:");
        LOG_INFO("  自动保存: {}", systemConfig.autoSave ? "开启" : "关闭");
        LOG_INFO("  自动保存间隔: {} 秒", systemConfig.autoSaveInterval);
        LOG_INFO("  最大存档槽位: {}", systemConfig.maxSaveSlots);
        LOG_INFO("  日志级别: {}", systemConfig.logLevel);
        LOG_INFO("  调试模式: {}", systemConfig.debugMode ? "开启" : "关闭");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Status display error: {}", e.what());
    }
}

// 主服务器循环
void serverLoop() {
    LOG_INFO("服务器主循环启动");
    
    auto lastStatusTime = std::chrono::steady_clock::now();
    auto lastSaveTime = std::chrono::steady_clock::now();
    
    while (g_running) {
        try {
            auto now = std::chrono::steady_clock::now();
            
            // 每30秒显示一次状态
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatusTime).count() >= 30) {
                displaySystemStatus();
                lastStatusTime = now;
            }
            
            // 每5分钟自动保存一次
            if (std::chrono::duration_cast<std::chrono::minutes>(now - lastSaveTime).count() >= 5) {
                auto& configManager = Fantasy::ConfigManager::getInstance();
                configManager.saveConfig("game", Fantasy::ConfigManager::ConfigLevel::GAME);
                configManager.saveConfig("system", Fantasy::ConfigManager::ConfigLevel::SYSTEM);
                LOG_INFO("自动保存完成");
                lastSaveTime = now;
            }
            
            // 处理资源系统任务
            auto& resourceSystem = Fantasy::ResourceSystem::getInstance();
            resourceSystem.getResourceLoader().waitForAll();
            
            // 清理未使用的资源
            resourceSystem.getResourceManager().cleanup();
            
            // 短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            LOG_ERROR("Server loop error: {}", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

// 清理资源
void cleanup() {
    try {
        LOG_INFO("开始清理资源...");
        
        // 关闭资源管理系统
        auto& resourceSystem = Fantasy::ResourceSystem::getInstance();
        resourceSystem.shutdown();
        
        // 关闭配置管理器
        auto& configManager = Fantasy::ConfigManager::getInstance();
        configManager.shutdown();
        
        LOG_INFO("资源清理完成");
        LOG_INFO("=== Fantasy Legend 服务器关闭 ===");
        
    } catch (const std::exception& e) {
        std::cerr << "Cleanup error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Fantasy Legend 服务器启动中..." << std::endl;
    
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
            LOG_ERROR("配置管理器初始化失败");
            return 1;
        }
        
        // 初始化资源管理系统
        if (!initializeResourceSystem()) {
            LOG_ERROR("资源管理系统初始化失败");
            return 1;
        }
        
        // 创建游戏配置示例
        createGameConfigs();
        
        // 加载游戏资源
        loadGameResources();
        
        // 显示初始状态
        displaySystemStatus();
        
        // 启动服务器主循环
        serverLoop();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }
    
    // 清理资源
    cleanup();
    
    std::cout << "服务器已关闭" << std::endl;
    return 0;
}