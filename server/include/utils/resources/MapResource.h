/**
 * @file MapResource.h
 * @brief 地图资源类
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <unordered_map>

namespace Fantasy {

/**
 * @brief 地图资源类
 * @details 用于加载和管理游戏地图数据
 */
class MapResource : public IResource {
public:
    /**
     * @brief 地图层类型
     */
    enum class LayerType {
        TERRAIN,    ///< 地形层
        OBJECT,     ///< 对象层
        COLLISION,  ///< 碰撞层
        TRIGGER,    ///< 触发器层
        SPAWN,      ///< 生成点层
        DECORATION  ///< 装饰层
    };

    /**
     * @brief 地图对象类型
     */
    enum class ObjectType {
        NPC,        ///< NPC
        MONSTER,    ///< 怪物
        ITEM,       ///< 物品
        TRIGGER,    ///< 触发器
        SPAWN_POINT,///< 生成点
        DECORATION  ///< 装饰物
    };

    /**
     * @brief 地图层数据
     */
    struct Layer {
        LayerType type;                    ///< 层类型
        std::string name;                  ///< 层名称
        int width;                         ///< 宽度
        int height;                        ///< 高度
        std::vector<int> data;             ///< 层数据
        std::vector<nlohmann::json> objects; ///< 对象数据
    };

    /**
     * @brief 地图对象数据
     */
    struct Object {
        ObjectType type;                   ///< 对象类型
        std::string id;                    ///< 对象ID
        std::string name;                  ///< 对象名称
        float x;                           ///< X坐标
        float y;                           ///< Y坐标
        float width;                       ///< 宽度
        float height;                      ///< 高度
        nlohmann::json properties;         ///< 对象属性
    };

    /**
     * @brief 构造函数
     * @param path 地图文件路径
     */
    explicit MapResource(const std::filesystem::path& path);

    // IResource接口实现
    const std::string& getId() const override;
    ResourceType getType() const override;
    ResourceState getState() const override;
    const std::filesystem::path& getPath() const override;
    std::filesystem::file_time_type getLastModifiedTime() const override;
    std::uintmax_t getSize() const override;
    bool load() override;
    void unload() override;
    bool reload() override;
    bool needsReload() const override;

    /**
     * @brief 获取地图名称
     * @return 地图名称
     */
    const std::string& getName() const;

    /**
     * @brief 获取地图宽度
     * @return 地图宽度（格子数）
     */
    int getWidth() const;

    /**
     * @brief 获取地图高度
     * @return 地图高度（格子数）
     */
    int getHeight() const;

    /**
     * @brief 获取格子宽度
     * @return 格子宽度（像素）
     */
    int getTileWidth() const;

    /**
     * @brief 获取格子高度
     * @return 格子高度（像素）
     */
    int getTileHeight() const;

    /**
     * @brief 获取地图层
     * @param type 层类型
     * @return 地图层，如果不存在则返回nullptr
     */
    const Layer* getLayer(LayerType type) const;

    /**
     * @brief 获取所有地图层
     * @return 地图层列表
     */
    const std::vector<Layer>& getLayers() const;

    /**
     * @brief 获取地图对象
     * @param id 对象ID
     * @return 地图对象，如果不存在则返回nullptr
     */
    const Object* getObject(const std::string& id) const;

    /**
     * @brief 获取指定类型的所有对象
     * @param type 对象类型
     * @return 对象列表
     */
    std::vector<Object> getObjects(ObjectType type) const;

    /**
     * @brief 获取指定区域内的所有对象
     * @param x 区域X坐标
     * @param y 区域Y坐标
     * @param width 区域宽度
     * @param height 区域高度
     * @return 对象列表
     */
    std::vector<Object> getObjectsInArea(float x, float y, float width, float height) const;

    /**
     * @brief 检查指定位置是否有碰撞
     * @param x X坐标
     * @param y Y坐标
     * @return 是否有碰撞
     */
    bool hasCollision(float x, float y) const;

    /**
     * @brief 获取地图属性
     * @param key 属性键
     * @param defaultValue 默认值
     * @return 属性值
     */
    template<typename T>
    T getProperty(const std::string& key, const T& defaultValue = T()) const {
        try {
            if (!properties_.contains(key)) {
                return defaultValue;
            }
            return properties_[key].get<T>();
        } catch (const std::exception&) {
            return defaultValue;
        }
    }

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    std::string name_;                            ///< 地图名称
    int width_;                                   ///< 地图宽度
    int height_;                                  ///< 地图高度
    int tileWidth_;                               ///< 格子宽度
    int tileHeight_;                              ///< 格子高度
    std::vector<Layer> layers_;                   ///< 地图层
    std::unordered_map<std::string, Object> objects_; ///< 地图对象
    nlohmann::json properties_;                   ///< 地图属性
};

/**
 * @brief 地图资源加载器
 */
class MapResourceLoader : public IResourceLoader {
public:
    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;
};

} // namespace Fantasy
