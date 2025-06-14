#include "FileUtils.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QCoreApplication>
#include <QDirIterator>
#include <QDebug>

FileUtils* FileUtils::s_instance = nullptr;

FileUtils* FileUtils::instance() {
    if (!s_instance) {
        s_instance = new FileUtils();
    }
    return s_instance;
}

FileUtils::FileUtils() {
    // 创建临时目录
    m_tempDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/FantasyLegend";
    createDirectories(m_tempDirectory);
}

FileUtils::~FileUtils() {
    cleanupTempFiles();
}

bool FileUtils::fileExists(const QString& filePath) {
    return QFile::exists(filePath);
}

bool FileUtils::directoryExists(const QString& dirPath) {
    return QDir(dirPath).exists();
}

bool FileUtils::createDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (dir.exists()) {
        return true;
    }
    return dir.mkdir(dirPath);
}

bool FileUtils::createDirectories(const QString& dirPath) {
    QDir dir;
    return dir.mkpath(dirPath);
}

bool FileUtils::removeFile(const QString& filePath) {
    if (!fileExists(filePath)) {
        return false;
    }
    return QFile::remove(filePath);
}

bool FileUtils::removeDirectory(const QString& dirPath, bool recursive) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return false;
    }
    
    if (recursive) {
        return dir.removeRecursively();
    } else {
        return dir.rmdir(dirPath);
    }
}

bool FileUtils::copyFile(const QString& sourcePath, const QString& destPath) {
    if (!fileExists(sourcePath)) {
        Logger::error(QString("Source file does not exist: %1").arg(sourcePath));
        return false;
    }
    
    // 确保目标目录存在
    QString destDir = getDirectoryPath(destPath);
    if (!createDirectories(destDir)) {
        Logger::error(QString("Failed to create destination directory: %1").arg(destDir));
        return false;
    }
    
    return QFile::copy(sourcePath, destPath);
}

bool FileUtils::moveFile(const QString& sourcePath, const QString& destPath) {
    if (!fileExists(sourcePath)) {
        Logger::error(QString("Source file does not exist: %1").arg(sourcePath));
        return false;
    }
    
    // 确保目标目录存在
    QString destDir = getDirectoryPath(destPath);
    if (!createDirectories(destDir)) {
        Logger::error(QString("Failed to create destination directory: %1").arg(destDir));
        return false;
    }
    
    return QFile::rename(sourcePath, destPath);
}

qint64 FileUtils::getFileSize(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.size();
}

QString FileUtils::getFileExtension(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix();
}

QString FileUtils::getFileName(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

QString FileUtils::getDirectoryPath(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.absolutePath();
}

QDateTime FileUtils::getFileModifiedTime(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.lastModified();
}

QDateTime FileUtils::getFileCreatedTime(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.birthTime();
}

QStringList FileUtils::getFilesInDirectory(const QString& dirPath, const QStringList& filters) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return QStringList();
    }
    
    if (filters.isEmpty()) {
        return dir.entryList(QDir::Files);
    } else {
        return dir.entryList(filters, QDir::Files);
    }
}

QStringList FileUtils::getDirectoriesInDirectory(const QString& dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return QStringList();
    }
    
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

QStringList FileUtils::getAllFilesInDirectory(const QString& dirPath, bool recursive) {
    QStringList files;
    
    if (recursive) {
        QDirIterator it(dirPath, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            files.append(it.next());
        }
    } else {
        QDir dir(dirPath);
        if (dir.exists()) {
            QStringList fileNames = dir.entryList(QDir::Files);
            for (const QString& fileName : fileNames) {
                files.append(combinePath(dirPath, fileName));
            }
        }
    }
    
    return files;
}

QString FileUtils::combinePath(const QString& path1, const QString& path2) {
    return QDir(path1).filePath(path2);
}

QString FileUtils::getAbsolutePath(const QString& relativePath) {
    return QFileInfo(relativePath).absoluteFilePath();
}

QString FileUtils::getRelativePath(const QString& absolutePath, const QString& basePath) {
    QDir baseDir(basePath);
    return baseDir.relativeFilePath(absolutePath);
}

QString FileUtils::normalizePath(const QString& path) {
    return QDir::cleanPath(path);
}

QString FileUtils::readTextFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error(QString("Failed to open file for reading: %1").arg(filePath));
        return QString();
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString content = stream.readAll();
    file.close();
    
    return content;
}

bool FileUtils::writeTextFile(const QString& filePath, const QString& content) {
    // 确保目录存在
    QString dirPath = getDirectoryPath(filePath);
    if (!createDirectories(dirPath)) {
        Logger::error(QString("Failed to create directory: %1").arg(dirPath));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error(QString("Failed to open file for writing: %1").arg(filePath));
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << content;
    file.close();
    
    return true;
}

bool FileUtils::appendTextFile(const QString& filePath, const QString& content) {
    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        Logger::error(QString("Failed to open file for appending: %1").arg(filePath));
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << content;
    file.close();
    
    return true;
}

QJsonObject FileUtils::readJsonFile(const QString& filePath) {
    QString content = readTextFile(filePath);
    if (content.isEmpty()) {
        return QJsonObject();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        Logger::error(QString("JSON parse error in %1: %2").arg(filePath).arg(error.errorString()));
        return QJsonObject();
    }
    
    return doc.object();
}

QJsonArray FileUtils::readJsonArrayFile(const QString& filePath) {
    QString content = readTextFile(filePath);
    if (content.isEmpty()) {
        return QJsonArray();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        Logger::error(QString("JSON parse error in %1: %2").arg(filePath).arg(error.errorString()));
        return QJsonArray();
    }
    
    return doc.array();
}

bool FileUtils::writeJsonFile(const QString& filePath, const QJsonObject& json) {
    QJsonDocument doc(json);
    QString content = doc.toJson(QJsonDocument::Indented);
    return writeTextFile(filePath, content);
}

bool FileUtils::writeJsonFile(const QString& filePath, const QJsonArray& json) {
    QJsonDocument doc(json);
    QString content = doc.toJson(QJsonDocument::Indented);
    return writeTextFile(filePath, content);
}

bool FileUtils::writeJsonFile(const QString& filePath, const QVariantMap& data) {
    QJsonObject json = QJsonObject::fromVariantMap(data);
    return writeJsonFile(filePath, json);
}

bool FileUtils::writeJsonFile(const QString& filePath, const QVariantList& data) {
    QJsonArray json = QJsonArray::fromVariantList(data);
    return writeJsonFile(filePath, json);
}

QByteArray FileUtils::readBinaryFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::error(QString("Failed to open binary file: %1").arg(filePath));
        return QByteArray();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    return data;
}

bool FileUtils::writeBinaryFile(const QString& filePath, const QByteArray& data) {
    // 确保目录存在
    QString dirPath = getDirectoryPath(filePath);
    if (!createDirectories(dirPath)) {
        Logger::error(QString("Failed to create directory: %1").arg(dirPath));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error(QString("Failed to open binary file for writing: %1").arg(filePath));
        return false;
    }
    
    qint64 bytesWritten = file.write(data);
    file.close();
    
    return bytesWritten == data.size();
}

QString FileUtils::getResourcePath(const QString& resourceName) {
    return QString(":/resources/%1").arg(resourceName);
}

bool FileUtils::isResourceFile(const QString& filePath) {
    return filePath.startsWith(":/");
}

QStringList FileUtils::getResourceFiles(const QString& resourceDir) {
    QStringList files;
    QDirIterator it(resourceDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        files.append(it.next());
    }
    return files;
}

QString FileUtils::createTempFile(const QString& prefix, const QString& suffix) {
    QString tempPath = combinePath(m_tempDirectory, QString("%1_%2%3")
        .arg(prefix)
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(suffix.isEmpty() ? "" : QString(".%1").arg(suffix)));
    
    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        return tempPath;
    }
    
    return QString();
}

QString FileUtils::createTempDirectory(const QString& prefix) {
    QString tempPath = combinePath(m_tempDirectory, QString("%1_%2")
        .arg(prefix)
        .arg(QDateTime::currentMSecsSinceEpoch()));
    
    if (createDirectory(tempPath)) {
        return tempPath;
    }
    
    return QString();
}

void FileUtils::cleanupTempFiles() {
    if (directoryExists(m_tempDirectory)) {
        removeDirectory(m_tempDirectory, true);
    }
}

bool FileUtils::startWatchingFile(const QString& filePath, QObject* receiver, const char* slot) {
    // 这里可以实现文件监控功能
    // 使用QFileSystemWatcher来监控文件变化
    Q_UNUSED(filePath)
    Q_UNUSED(receiver)
    Q_UNUSED(slot)
    return true;
}

void FileUtils::stopWatchingFile(const QString& filePath) {
    Q_UNUSED(filePath)
    // 停止文件监控
}
