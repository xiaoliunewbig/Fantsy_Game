/**
 * @file PluginSystem.h
 * @brief 插件系统接口定义 - 支持脚本热更新和扩展
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持动态库 (.so/.dll) 加载
 * - 支持 Lua / Python 脚本插件
 * - 插件钩子注册与触发
 * - 插件卸载与资源回收
 * - 插件版本检查
 * - 插件依赖管理（TODO）
 */
#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include <string>
#include <functional>
#include <vector>

class PluginSystem {
public:
    /**
     * @brief 获取插件系统单例
     * @return PluginSystem 实例引用
     */
    static PluginSystem& GetInstance();

    /**
     * @brief 加载指定路径下的插件
     * @param path 插件路径（如 .so 或 .dll 文件）
     * @return 是否加载成功
     */
    bool LoadPlugin(const std::string& path);

    /**
     * @brief 卸载指定路径的插件
     * @param path 插件路径
     * @return 是否卸载成功
     */
    bool UnloadPlugin(const std::string& path);

    /**
     * @brief 注册插件钩子函数
     * @param hookName 钩子名称（如 “onPlayerLogin”）
     * @param callback 钩子回调函数
     */
    void RegisterHook(const std::string& hookName, const std::function<void()>& callback);

    /**
     * @brief 触发指定名称的插件钩子
     * @param hookName 钩子名称
     */
    void TriggerHook(const std::string& hookName);

    /**
     * @brief 获取已加载插件列表
     * @return 插件路径列表
     * @todo 实现插件信息获取（如作者、版本等）
     */
    std::vector<std::string> GetLoadedPlugins() const;
};

#endif // PLUGIN_SYSTEM_H