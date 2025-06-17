/**
 * @file FontResource.h
 * @brief 字体资源类
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
#include <unordered_map>

namespace Fantasy {

/**
 * @brief 字体格式
 */
enum class FontFormat {
    TTF,        ///< TrueType字体
    OTF,        ///< OpenType字体
    WOFF,       ///< Web Open Font Format
    WOFF2,      ///< Web Open Font Format 2.0
    BMF,        ///< Bitmap字体
    UNKNOWN     ///< 未知格式
};

/**
 * @brief 字体样式
 */
enum class FontStyle {
    NORMAL,     ///< 正常
    BOLD,       ///< 粗体
    ITALIC,     ///< 斜体
    BOLD_ITALIC ///< 粗斜体
};

/**
 * @brief 2D向量结构
 */
struct Vec2 {
    float x, y;
    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

/**
 * @brief 字符信息
 */
struct GlyphInfo {
    uint32_t unicode;           ///< Unicode码点
    uint32_t width;             ///< 字符宽度
    uint32_t height;            ///< 字符高度
    int32_t bearingX;           ///< X方向偏移
    int32_t bearingY;           ///< Y方向偏移
    int32_t advance;            ///< 前进宽度
    std::vector<uint8_t> bitmap; ///< 位图数据
    uint32_t textureId;         ///< 纹理ID
    Vec2 texCoordMin;           ///< 纹理坐标最小值
    Vec2 texCoordMax;           ///< 纹理坐标最大值
};

/**
 * @brief 字体数据
 */
struct FontData {
    std::string familyName;     ///< 字体族名称
    std::string styleName;      ///< 样式名称
    FontFormat format;          ///< 字体格式
    FontStyle style;            ///< 字体样式
    uint32_t unitsPerEm;        ///< EM单位
    int32_t ascender;           ///< 上升高度
    int32_t descender;          ///< 下降高度
    int32_t lineGap;            ///< 行间距
    uint32_t numGlyphs;         ///< 字符数量
    std::unordered_map<uint32_t, GlyphInfo> glyphs; ///< 字符信息映射
    std::vector<uint8_t> fontData; ///< 字体数据
    bool isLoaded;              ///< 是否已加载
};

/**
 * @brief 字体资源类
 * @details 用于加载和管理字体资源
 */
class FontResource : public IResource {
public:
    /**
     * @brief 构造函数
     * @param path 字体文件路径
     * @param size 字体大小
     */
    explicit FontResource(const std::filesystem::path& path, uint32_t size = 16);

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
     * @brief 获取字体数据
     * @return 字体数据
     */
    const FontData& getData() const;

    /**
     * @brief 获取字体格式
     * @return 字体格式
     */
    FontFormat getFormat() const;

    /**
     * @brief 获取字体样式
     * @return 字体样式
     */
    FontStyle getStyle() const;

    /**
     * @brief 获取字体大小
     * @return 字体大小
     */
    uint32_t getFontSize() const;

    /**
     * @brief 获取字体族名称
     * @return 字体族名称
     */
    const std::string& getFamilyName() const;

    /**
     * @brief 获取样式名称
     * @return 样式名称
     */
    const std::string& getStyleName() const;

    /**
     * @brief 获取字符信息
     * @param unicode Unicode码点
     * @return 字符信息
     */
    const GlyphInfo* getGlyph(uint32_t unicode) const;

    /**
     * @brief 获取字符数量
     * @return 字符数量
     */
    uint32_t getGlyphCount() const;

    /**
     * @brief 获取行高
     * @return 行高
     */
    uint32_t getLineHeight() const;

    /**
     * @brief 获取字符宽度
     * @param unicode Unicode码点
     * @return 字符宽度
     */
    uint32_t getGlyphWidth(uint32_t unicode) const;

    /**
     * @brief 获取字符串宽度
     * @param text 文本内容
     * @return 字符串宽度
     */
    uint32_t getTextWidth(const std::string& text) const;

    /**
     * @brief 获取字符串高度
     * @param text 文本内容
     * @param maxWidth 最大宽度
     * @return 字符串高度
     */
    uint32_t getTextHeight(const std::string& text, uint32_t maxWidth = 0) const;

    /**
     * @brief 生成字符位图
     * @param unicode Unicode码点
     * @return 是否成功生成
     */
    bool generateGlyph(uint32_t unicode);

    /**
     * @brief 预生成字符集
     * @param startUnicode 起始Unicode码点
     * @param endUnicode 结束Unicode码点
     * @return 是否成功生成
     */
    bool generateGlyphRange(uint32_t startUnicode, uint32_t endUnicode);

    /**
     * @brief 设置字体大小
     * @param size 字体大小
     * @return 是否成功设置
     */
    bool setSize(uint32_t size);

    /**
     * @brief 设置字体样式
     * @param style 字体样式
     * @return 是否成功设置
     */
    bool setStyle(FontStyle style);

    /**
     * @brief 获取字体统计信息
     * @return 统计信息字符串
     */
    std::string getStats() const;

    /**
     * @brief 导出字符位图
     * @param unicode Unicode码点
     * @param outputPath 输出路径
     * @return 是否成功导出
     */
    bool exportGlyphBitmap(uint32_t unicode, const std::filesystem::path& outputPath) const;

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    FontData data_;                               ///< 字体数据
    uint32_t fontSize_;                           ///< 字体大小

    /**
     * @brief 加载TTF/OTF格式字体
     * @return 是否成功加载
     */
    bool loadTTF();

    /**
     * @brief 加载WOFF格式字体
     * @return 是否成功加载
     */
    bool loadWOFF();

    /**
     * @brief 加载WOFF2格式字体
     * @return 是否成功加载
     */
    bool loadWOFF2();

    /**
     * @brief 加载位图字体
     * @return 是否成功加载
     */
    bool loadBitmapFont();

    /**
     * @brief 检测字体格式
     * @return 字体格式
     */
    FontFormat detectFormat() const;

    /**
     * @brief 初始化字体信息
     * @return 是否成功初始化
     */
    bool initializeFontInfo();

    /**
     * @brief 计算字体统计信息
     */
    void calculateStats();
};

/**
 * @brief 字体资源加载器
 */
class FontResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     * @param supportedFormats 支持的字体格式列表
     */
    explicit FontResourceLoader(const std::vector<FontFormat>& supportedFormats = {});

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;

private:
    std::vector<FontFormat> supportedFormats_; ///< 支持的字体格式
};

} // namespace Fantasy 