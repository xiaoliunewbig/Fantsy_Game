/**
 * @file FileUtils.h
 * @brief 文件工具类 - 提供常用文件操作方法
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 文件读写支持
 * - 文件路径处理
 * - 文件存在性检查
 * - 支持目录遍历
 * - 支持压缩文件处理（TODO）
 */
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include <fstream>

class FileUtils {
public:
    /**
     * @brief 读取文件全部内容为字符串
     * @param filePath 文件路径
     * @return 文件内容字符串
     */
    static std::string ReadFileToString(const std::string& filePath);

    /**
     * @brief 将字符串写入文件
     * @param filePath 文件路径
     * @param content 内容
     * @return 是否成功
     */
    static bool WriteStringToFile(const std::string& filePath, const std::string& content);

    /**
     * @brief 检查文件是否存在
     * @param filePath 文件路径
     * @return 是否存在
     */
    static bool FileExists(const std::string& filePath);

    /**
     * @brief 获取文件大小
     * @param filePath 文件路径
     * @return 文件大小（字节）
     */
    static long long GetFileSize(const std::string& filePath);

    /**
     * @brief 创建目录（递归创建）
     * @param dirPath 目录路径
     * @return 是否成功
     */
    static bool CreateDirectory(const std::string& dirPath);

    /**
     * @brief 获取目录下的所有文件列表
     * @param dirPath 目录路径
     * @return 文件路径列表
     */
    static std::vector<std::string> GetFilesInDirectory(const std::string& dirPath);

    /**
     * @brief 删除指定文件
     * @param filePath 文件路径
     * @return 是否成功
     */
    static bool DeleteFile(const std::string& filePath);

    /**
     * @brief 解压 ZIP/RAR 文件到指定目录
     * @param archivePath 压缩包路径
     * @param outputDir 输出目录
     * @todo 实现解压功能
     */
    static bool ExtractArchive(const std::string& archivePath, const std::string& outputDir);
};

#endif // FILE_UTILS_H