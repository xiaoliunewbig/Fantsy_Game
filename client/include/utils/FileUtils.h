/**
 * @file FileUtils.h
 * @brief 文件工具
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QVariantList>

namespace Fantasy {

/**
 * @brief 文件工具类
 * 
 * 提供文件操作相关的工具方法
 */
class FileUtils : public QObject {
    Q_OBJECT
    
public:
    static FileUtils* instance();
    
    // 路径相关
    static QString getApplicationDir();
    static QString getDataDir();
    static QString getConfigDir();
    static QString getLogDir();
    
    // 文件操作
    static bool fileExists(const QString& filePath);
    static bool directoryExists(const QString& dirPath);
    static bool createDirectory(const QString& dirPath);
    static bool createDirectories(const QString& dirPath);
    static bool removeFile(const QString& filePath);
    static bool removeDirectory(const QString& dirPath, bool recursive = false);
    static bool copyFile(const QString& sourcePath, const QString& destPath);
    static bool moveFile(const QString& sourcePath, const QString& destPath);
    
    // 文件信息
    static qint64 getFileSize(const QString& filePath);
    static QString getFileExtension(const QString& filePath);
    static QString getFileName(const QString& filePath);
    static QString getDirectoryPath(const QString& filePath);
    static QDateTime getFileModifiedTime(const QString& filePath);
    static QDateTime getFileCreatedTime(const QString& filePath);
    
    // 目录操作
    static QStringList getFilesInDirectory(const QString& dirPath, const QStringList& filters = QStringList());
    static QStringList getDirectoriesInDirectory(const QString& dirPath);
    static QStringList getAllFilesInDirectory(const QString& dirPath, bool recursive = false);
    
    // 路径操作
    static QString combinePath(const QString& path1, const QString& path2);
    static QString getAbsolutePath(const QString& relativePath);
    static QString getRelativePath(const QString& absolutePath, const QString& basePath);
    static QString normalizePath(const QString& path);
    
    // 文本文件操作
    static QString readTextFile(const QString& filePath);
    static bool writeTextFile(const QString& filePath, const QString& content);
    static bool appendTextFile(const QString& filePath, const QString& content);
    
    // JSON文件操作
    static QJsonObject readJsonFile(const QString& filePath);
    static QJsonArray readJsonArrayFile(const QString& filePath);
    static bool writeJsonFile(const QString& filePath, const QJsonObject& json);
    static bool writeJsonFile(const QString& filePath, const QJsonArray& json);
    static bool writeJsonFile(const QString& filePath, const QVariantMap& data);
    static bool writeJsonFile(const QString& filePath, const QVariantList& data);
    
    // 二进制文件操作
    static QByteArray readBinaryFile(const QString& filePath);
    static bool writeBinaryFile(const QString& filePath, const QByteArray& data);
    
    // 资源文件操作
    static QString getResourcePath(const QString& resourceName);
    static bool isResourceFile(const QString& filePath);
    static QStringList getResourceFiles(const QString& resourceDir);
    
    // 临时文件操作
    static QString createTempFile(const QString& prefix = "temp", const QString& suffix = "");
    static QString createTempDirectory(const QString& prefix = "temp");
    static void cleanupTempFiles();
    
    // 文件监控
    static bool startWatchingFile(const QString& filePath, QObject* receiver, const char* slot);
    static void stopWatchingFile(const QString& filePath);
    
signals:
    void fileChanged(const QString& filePath);
    void fileCreated(const QString& filePath);
    void fileDeleted(const QString& filePath);
    void fileRenamed(const QString& oldPath, const QString& newPath);
    
private:
    FileUtils();
    ~FileUtils();
    
    static FileUtils* s_instance;
    QString m_tempDirectory;
};

} // namespace Fantasy

#endif // FILEUTILS_H
