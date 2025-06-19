/**
 * @file WorldModel.h
 * @brief 全局世界模型定义 - 包含游戏世界状态信息
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 地图信息管理
 * - NPC列表维护
 * - 天气与时间系统
 * - 支持多人在线同步
 * - 支持区域加载/卸载（TODO）
 */
#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <string>
#include <unordered_map>
#include <vector>

class WorldModel {
public:
    struct NPCData {
        std::string id;
        std::string name;
        float x, y, z;
        int type;
    };

    /**
     * @brief 初始化世界模型
     */
    void Init();

    /**
     * @brief 加载指定地图
     * @param mapId 地图ID
     */
    void LoadMap(const std::string& mapId);

    /**
     * @brief 获取当前地图ID
     * @return 地图标识字符串
     */
    std::string GetCurrentMapId() const;

    /**
     * @brief 添加一个NPC到世界中
     * @param npc NPC数据
     */
    void AddNPC(const NPCData& npc);

    /**
     * @brief 获取所有NPC列表
     * @return NPC数据列表
     */
    std::vector<NPCData> GetAllNPCs() const;

    /**
     * @brief 设置当前天气状态
     * @param weather 天气描述（如 "sunny", "rainy"）
     */
    void SetWeather(const std::string& weather);

    /**
     * @brief 获取当前天气状态
     * @return 天气字符串
     */
    std::string GetWeather() const;

    /**
     * @brief 设置当前游戏时间（小时）
     * @param hour 小时数（0~23）
     */
    void SetTimeOfDay(int hour);

    /**
     * @brief 获取当前游戏时间（小时）
     * @return 小时数
     */
    int GetTimeOfDay() const;

    /**
     * @brief 更新世界状态
     * @param dt 时间间隔（秒）
     */
    void Update(float dt);

    /**
     * @brief 销毁世界模型
     */
    void Destroy();

    /**
     * @brief 获取所有区域列表
     * @return 区域ID列表
     * @todo 实现区域加载机制
     */
    std::vector<std::string> GetRegions() const;
};

#endif // WORLD_MODEL_H