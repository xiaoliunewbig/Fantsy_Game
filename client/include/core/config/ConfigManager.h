/**
 * @file ConfigManager.h
 * @brief 配置管理系统接口定义 - 支持读取和保存配置文件
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持 JSON/YAML/INI 格式配置
 * - 支持图形设置、键位映射等
 * - 自动保存修改后的配置
 * - 支持默认值回退机制
 */
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <unordered_map>

class ConfigManager {
public:
    /**
     * @brief 加载指定路径的配置文件
     * @param filePath 配置文件路径
     * @return 是否加载成功
     */
    bool LoadFromFile(const std::string& filePath);

    /**
     * @brief 保存当前配置到文件
     * @param filePath 保存路径
     * @return 是否保存成功
     */
    bool SaveToFile(const std::string& filePath);

    /**
     * @brief 获取指定键的字符串值
     * @param key 键名
     * @return 值字符串
     */
    std::string GetString(const std::string& key) const;

    /**
     * @brief 设置指定键的字符串值
     * @param key 键名
     * @param value 值
     */
    void SetString(const std::string& key, const std::string& value);

    /**
     * @brief 获取指定键的整数值
     * @param key 键名
     * @param defaultValue 默认值
     * @return 整数结果
     */
    int GetInt(const std::string& key, int defaultValue = 0) const;

    /**
     * @brief 设置指定键的整数值
     * @param key 键名
     * @param value 值
     */
    void SetInt(const std::string& key, int value);

    /**
     * @brief 获取指定键的浮点数值
     * @param key 键名
     * @param defaultValue 默认值
     * @return 浮点数结果
     */
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;

    /**
     * @brief 设置指定键的浮点数值
     * @param key 键名
     * @param value 值
     */
    void SetFloat(const std::string& key, float value);

    /**
     * @brief 获取指定键是否存在
     * @param key 键名
     * @return 是否存在
     */
    bool HasKey(const std::string& key) const;
};

#endif // CONFIG_MANAGER_H