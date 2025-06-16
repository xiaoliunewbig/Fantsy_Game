#include <iostream>
#include "include/core/GameEngine.h"
#include "include/data/database/DatabaseManager.h"
#include "include/utils/Logger.h"
#include "include/utils/FileUtils.h"
#include "include/utils/ConsoleSink.h"
#include "include/utils/FileSink.h"     // 如果需要写入文件日志
// #include "utils/SignalSink.h"   // 如果后续需要 Qt 通信（暂时不启用）

int main(int argc, char *argv[])
{
    // 初始化日志系统
    auto& logger = Logger::getInstance();
    logger.setLevel(LogLevel::Debug); // 设置最低记录等级

    // 添加控制台输出
    logger.addSink(std::make_shared<ConsoleSink>());

    // 可选：添加文件日志输出
    logger.addSink(std::make_shared<FileSink>("logs/server.log"));

    // 所有日志配置完成后，再开始记录日志
    FANTASY_LOG_INFO("启动游戏服务器...");

    // 初始化数据库
    FANTASY_LOG_DEBUG("初始化数据库...");
    DatabaseManager::instance().initialize("game_data.db");

    // 初始化游戏引擎
    FANTASY_LOG_DEBUG("初始化游戏引擎...");
    GameEngine::instance()->startGame();

    FANTASY_LOG_INFO("游戏服务器启动成功");

    return 0;
}