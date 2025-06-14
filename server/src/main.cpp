#include <iostream>
#include "core/GameEngine.h"
#include "data/database/DatabaseManager.h"
#include "utils/Logger.h"

int main(int argc, char *argv[])
{
    Logger::info("启动游戏服务器...");
    
    // 初始化数据库
    DatabaseManager::instance().initialize("game_data.db");
    
    // 初始化游戏引擎
    GameEngine::instance()->startGame();
    
    Logger::info("游戏服务器启动成功");
    
    return 0;
}
