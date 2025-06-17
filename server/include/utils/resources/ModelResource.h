/**
 * @file ModelResource.h
 * @brief 模型资源类
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#pragma once

#include "ResourceType.h"
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace Fantasy {

/**
 * @brief 模型格式
 */
enum class ModelFormat {
    OBJ,        ///< Wavefront OBJ格式
    FBX,        ///< Autodesk FBX格式
    GLTF,       ///< glTF格式
    GLB,        ///< glTF Binary格式
    DAE,        ///< COLLADA格式
    BLEND,      ///< Blender格式
    UNKNOWN     ///< 未知格式
};

/**
 * @brief 顶点数据
 */
struct Vertex {
    glm::vec3 position;     ///< 位置
    glm::vec3 normal;       ///< 法线
    glm::vec2 texCoord;     ///< 纹理坐标
    glm::vec3 tangent;      ///< 切线
    glm::vec3 bitangent;    ///< 副切线
    glm::vec4 color;        ///< 颜色
};

/**
 * @brief 材质数据
 */
struct Material {
    std::string name;                   ///< 材质名称
    glm::vec3 ambient;                  ///< 环境光
    glm::vec3 diffuse;                  ///< 漫反射
    glm::vec3 specular;                 ///< 镜面反射
    float shininess;                    ///< 光泽度
    std::string diffuseTexture;         ///< 漫反射纹理
    std::string normalTexture;          ///< 法线纹理
    std::string specularTexture;        ///< 镜面反射纹理
    float opacity;                      ///< 透明度
    bool hasTransparency;               ///< 是否有透明度
};

/**
 * @brief 网格数据
 */
struct Mesh {
    std::string name;                   ///< 网格名称
    std::vector<Vertex> vertices;       ///< 顶点数据
    std::vector<uint32_t> indices;      ///< 索引数据
    Material material;                  ///< 材质
    glm::mat4 transform;                ///< 变换矩阵
    uint32_t vertexCount;               ///< 顶点数量
    uint32_t indexCount;                ///< 索引数量
};

/**
 * @brief 骨骼数据
 */
struct Bone {
    std::string name;                   ///< 骨骼名称
    glm::mat4 offsetMatrix;             ///< 偏移矩阵
    glm::mat4 transform;                ///< 变换矩阵
    int parentIndex;                    ///< 父骨骼索引
    std::vector<int> childIndices;      ///< 子骨骼索引
};

/**
 * @brief 动画关键帧
 */
struct KeyFrame {
    float time;                         ///< 时间
    glm::vec3 position;                 ///< 位置
    glm::quat rotation;                 ///< 旋转
    glm::vec3 scale;                    ///< 缩放
};

/**
 * @brief 动画通道
 */
struct AnimationChannel {
    std::string boneName;               ///< 骨骼名称
    std::vector<KeyFrame> keyFrames;    ///< 关键帧
};

/**
 * @brief 动画数据
 */
struct Animation {
    std::string name;                   ///< 动画名称
    float duration;                     ///< 持续时间
    float ticksPerSecond;               ///< 每秒帧数
    std::vector<AnimationChannel> channels; ///< 动画通道
};

/**
 * @brief 模型资源类
 * @details 用于加载和管理3D模型资源
 */
class ModelResource : public IResource {
public:
    /**
     * @brief 模型数据
     */
    struct ModelData {
        std::string name;                   ///< 模型名称
        ModelFormat format;                 ///< 模型格式
        std::vector<Mesh> meshes;           ///< 网格数据
        std::vector<Material> materials;    ///< 材质数据
        std::vector<Bone> bones;            ///< 骨骼数据
        std::vector<Animation> animations;  ///< 动画数据
        glm::vec3 boundingBoxMin;          ///< 包围盒最小值
        glm::vec3 boundingBoxMax;          ///< 包围盒最大值
        float boundingRadius;               ///< 包围球半径
    };

    /**
     * @brief 构造函数
     * @param path 模型文件路径
     */
    explicit ModelResource(const std::filesystem::path& path);

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
     * @brief 获取模型数据
     * @return 模型数据
     */
    const ModelData& getData() const;

    /**
     * @brief 获取模型名称
     * @return 模型名称
     */
    const std::string& getName() const;

    /**
     * @brief 获取模型格式
     * @return 模型格式
     */
    ModelFormat getFormat() const;

    /**
     * @brief 获取网格数量
     * @return 网格数量
     */
    uint32_t getMeshCount() const;

    /**
     * @brief 获取材质数量
     * @return 材质数量
     */
    uint32_t getMaterialCount() const;

    /**
     * @brief 获取骨骼数量
     * @return 骨骼数量
     */
    uint32_t getBoneCount() const;

    /**
     * @brief 获取动画数量
     * @return 动画数量
     */
    uint32_t getAnimationCount() const;

    /**
     * @brief 获取指定网格
     * @param index 网格索引
     * @return 网格数据
     */
    const Mesh& getMesh(uint32_t index) const;

    /**
     * @brief 获取指定材质
     * @param index 材质索引
     * @return 材质数据
     */
    const Material& getMaterial(uint32_t index) const;

    /**
     * @brief 获取指定骨骼
     * @param index 骨骼索引
     * @return 骨骼数据
     */
    const Bone& getBone(uint32_t index) const;

    /**
     * @brief 获取指定动画
     * @param index 动画索引
     * @return 动画数据
     */
    const Animation& getAnimation(uint32_t index) const;

    /**
     * @brief 获取包围盒
     * @param min 最小值
     * @param max 最大值
     */
    void getBoundingBox(glm::vec3& min, glm::vec3& max) const;

    /**
     * @brief 获取包围球半径
     * @return 包围球半径
     */
    float getBoundingRadius() const;

    /**
     * @brief 优化模型数据
     * @return 是否成功优化
     */
    bool optimize();

    /**
     * @brief 生成LOD模型
     * @param level LOD级别
     * @return 是否成功生成
     */
    bool generateLOD(uint32_t level);

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    ModelData data_;                              ///< 模型数据

    /**
     * @brief 加载OBJ格式模型
     * @return 是否成功加载
     */
    bool loadOBJ();

    /**
     * @brief 加载FBX格式模型
     * @return 是否成功加载
     */
    bool loadFBX();

    /**
     * @brief 加载GLTF格式模型
     * @return 是否成功加载
     */
    bool loadGLTF();

    /**
     * @brief 计算包围盒
     */
    void calculateBoundingBox();

    /**
     * @brief 计算包围球
     */
    void calculateBoundingSphere();
};

/**
 * @brief 模型资源加载器
 */
class ModelResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     * @param supportedFormats 支持的模型格式列表
     */
    explicit ModelResourceLoader(const std::vector<ModelFormat>& supportedFormats = {});

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;

private:
    std::vector<ModelFormat> supportedFormats_; ///< 支持的模型格式
};

} // namespace Fantasy 