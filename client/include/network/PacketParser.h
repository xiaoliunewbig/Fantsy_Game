/**
 * @file PacketParser.h
 * @brief 网络数据包解析器接口定义 - 支持多种协议解析
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持Protobuf、JSON、自定义二进制格式
 * - 支持数据校验与加密解密
 * - 支持异步解析
 * - 支持压缩数据解压（TODO）
 */
#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <string>
#include <memory>

class PacketParser {
public:
    enum class FormatType {
        PROTOBUF,
        JSON,
        CUSTOM_BINARY
    };

    /**
     * @brief 解析数据包内容
     * @param format 格式类型
     * @param rawData 原始数据
     * @param outResult 输出解析结果
     * @return 是否解析成功
     */
    virtual bool Parse(FormatType format, const std::string& rawData, std::string* outResult) = 0;

    /**
     * @brief 设置解密密钥
     * @param key 密钥字符串
     */
    virtual void SetDecryptionKey(const std::string& key);

    /**
     * @brief 获取当前使用的解密密钥
     * @return 密钥字符串
     */
    virtual std::string GetDecryptionKey() const;

    /**
     * @brief 启用或禁用数据校验
     * @param enable 是否启用
     */
    virtual void EnableValidation(bool enable);

    /**
     * @brief 检查是否启用数据校验
     * @return 是否启用
     */
    virtual bool IsValidationEnabled() const;
};

#endif // PACKET_PARSER_H