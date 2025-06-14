#include "ResourceManager.h"
#include "Logger.h"
#include "FileUtils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths>
#include <QApplication>
#include <QThreadPool>
#include <QBuffer>
#include <QDataStream>
#include <QImageReader>
#include <QImageWriter>

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::ResourceManager() 
    : m_cacheEnabled(true)
    , m_maxCacheSize(100)
    , m_cachePolicy("lru")
    , m_autoSyncEnabled(false)
    , m_syncInterval(300) // 5分钟
    , m_monitoringEnabled(false)
    , m_preloadWatcher(nullptr)
    , m_isPreloading(false)
    , m_preloadCurrent(0)
    , m_preloadTotal(0) {
    
    Logger::instance()->log("ResourceManager", "ResourceManager instance created");
    
    // 初始化资源路径
    initializeResourcePaths();
    
    // 设置自动同步
    setupAutoSync();
    
    // 加载资源索引
    loadResourceIndex();
}

ResourceManager::~ResourceManager() {
    // 保存资源索引
    saveResourceIndex();
    
    // 清理资源
    clearCache();
    
    // 清理异步操作
    for (auto watcher : m_asyncWatchers) {
        if (watcher->isRunning()) {
            watcher->waitForFinished();
        }
        delete watcher;
    }
    
    // 清理预加载
    if (m_preloadWatcher && m_preloadWatcher->isRunning()) {
        m_preloadWatcher->cancel();
        m_preloadWatcher->waitForFinished();
        delete m_preloadWatcher;
    }
    
    Logger::instance()->log("ResourceManager", "ResourceManager instance destroyed");
}

ResourceManager* ResourceManager::instance() {
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

void ResourceManager::initializeResourcePaths() {
    // 设置默认资源路径
    m_resourcePaths << ":/resources";
    m_resourcePaths << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/resources";
    m_resourcePaths << QApplication::applicationDirPath() + "/resources";
    
    // 设置缓存和备份目录
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/resources";
    m_backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backup/resources";
    
    // 创建目录
    QDir().mkpath(m_cacheDir);
    QDir().mkpath(m_backupDir);
}

void ResourceManager::setupAutoSync() {
    m_autoSyncTimer = new QTimer(this);
    connect(m_autoSyncTimer, &QTimer::timeout, this, &ResourceManager::onAutoSyncTimer);
    
    if (m_autoSyncEnabled) {
        m_autoSyncTimer->start(m_syncInterval * 1000);
    }
}

void ResourceManager::loadResourceIndex() {
    QString indexPath = m_cacheDir + "/resource_index.json";
    QFile indexFile(indexPath);
    
    if (indexFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(indexFile.readAll());
        m_resourceIndex = doc.object().toVariantMap();
    }
}

void ResourceManager::saveResourceIndex() {
    QString indexPath = m_cacheDir + "/resource_index.json";
    QFile indexFile(indexPath);
    
    if (indexFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc = QJsonDocument::fromVariant(m_resourceIndex);
        indexFile.write(doc.toJson());
    }
}

// 资源管理方法
bool ResourceManager::loadResource(const QString& resourcePath, const QString& resourceType) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (isResourceLoaded(resourcePath)) {
        return true;
    }
    
    QString fullPath = findResource(resourcePath);
    if (fullPath.isEmpty()) {
        Logger::instance()->log("ResourceManager", QString("Resource not found: %1").arg(resourcePath));
        emit resourceError(resourcePath, "Resource not found");
        return false;
    }
    
    bool success = false;
    
    if (resourceType == "image") {
        success = loadImage(fullPath);
    } else if (resourceType == "sound") {
        success = loadSound(fullPath);
    } else if (resourceType == "font") {
        success = loadFont(fullPath);
    } else if (resourceType == "animation") {
        success = loadAnimation(fullPath);
    } else if (resourceType == "data") {
        success = loadDataFile(fullPath);
    }
    
    if (success) {
        // 更新资源索引
        QVariantMap resourceInfo;
        resourceInfo["type"] = resourceType;
        resourceInfo["path"] = fullPath;
        resourceInfo["loaded_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        resourceInfo["size"] = getResourceSize(fullPath);
        m_resourceIndex[resourcePath] = resourceInfo;
        
        emit resourceLoaded(resourcePath, resourceType);
    }
    
    return success;
}

bool ResourceManager::unloadResource(const QString& resourcePath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (!isResourceLoaded(resourcePath)) {
        return false;
    }
    
    // 从缓存中移除
    m_imageCache.remove(resourcePath);
    m_soundCache.remove(resourcePath);
    m_fontCache.remove(resourcePath);
    m_animationCache.remove(resourcePath);
    m_dataCache.remove(resourcePath);
    
    // 从索引中移除
    m_resourceIndex.remove(resourcePath);
    
    emit resourceUnloaded(resourcePath);
    return true;
}

bool ResourceManager::isResourceLoaded(const QString& resourcePath) const {
    QMutexLocker locker(&m_resourceMutex);
    return m_imageCache.contains(resourcePath) ||
           m_soundCache.contains(resourcePath) ||
           m_fontCache.contains(resourcePath) ||
           m_animationCache.contains(resourcePath) ||
           m_dataCache.contains(resourcePath);
}

QStringList ResourceManager::getLoadedResources(const QString& resourceType) const {
    QMutexLocker locker(&m_resourceMutex);
    QStringList resources;
    
    if (resourceType.isEmpty() || resourceType == "image") {
        resources << m_imageCache.keys();
    }
    if (resourceType.isEmpty() || resourceType == "sound") {
        resources << m_soundCache.keys();
    }
    if (resourceType.isEmpty() || resourceType == "font") {
        resources << m_fontCache.keys();
    }
    if (resourceType.isEmpty() || resourceType == "animation") {
        resources << m_animationCache.keys();
    }
    if (resourceType.isEmpty() || resourceType == "data") {
        resources << m_dataCache.keys();
    }
    
    return resources;
}

// 图片资源管理方法
bool ResourceManager::loadImage(const QString& imagePath) {
    if (!m_cacheEnabled) {
        return true; // 不缓存，直接返回成功
    }
    
    QPixmap pixmap;
    if (!pixmap.load(imagePath)) {
        Logger::instance()->log("ResourceManager", QString("Failed to load image: %1").arg(imagePath));
        return false;
    }
    
    m_imageCache[imagePath] = pixmap;
    updateResourceUsage(imagePath, pixmap.sizeInBytes());
    
    return true;
}

QPixmap ResourceManager::getImage(const QString& imagePath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_imageCache.contains(imagePath)) {
        return m_imageCache[imagePath];
    }
    
    // 如果不在缓存中，尝试加载
    if (loadImage(imagePath)) {
        return m_imageCache[imagePath];
    }
    
    return QPixmap();
}

QImage ResourceManager::getImageData(const QString& imagePath) {
    QPixmap pixmap = getImage(imagePath);
    return pixmap.toImage();
}

bool ResourceManager::preloadImages(const QStringList& imagePaths) {
    bool success = true;
    for (const QString& imagePath : imagePaths) {
        success &= loadImage(imagePath);
    }
    return success;
}

void ResourceManager::clearImageCache() {
    QMutexLocker locker(&m_resourceMutex);
    m_imageCache.clear();
    emit cacheCleared("image");
}

// 音频资源管理方法
bool ResourceManager::loadSound(const QString& soundPath) {
    if (!m_cacheEnabled) {
        return true; // 不缓存，直接返回成功
    }
    
    QSound* sound = new QSound(soundPath);
    if (!sound->isAvailable()) {
        delete sound;
        Logger::instance()->log("ResourceManager", QString("Failed to load sound: %1").arg(soundPath));
        return false;
    }
    
    m_soundCache[soundPath] = sound;
    updateResourceUsage(soundPath, QFileInfo(soundPath).size());
    
    return true;
}

QSound* ResourceManager::getSound(const QString& soundPath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_soundCache.contains(soundPath)) {
        return m_soundCache[soundPath];
    }
    
    // 如果不在缓存中，尝试加载
    if (loadSound(soundPath)) {
        return m_soundCache[soundPath];
    }
    
    return nullptr;
}

bool ResourceManager::preloadSounds(const QStringList& soundPaths) {
    bool success = true;
    for (const QString& soundPath : soundPaths) {
        success &= loadSound(soundPath);
    }
    return success;
}

void ResourceManager::clearSoundCache() {
    QMutexLocker locker(&m_resourceMutex);
    
    // 删除QSound对象
    for (auto sound : m_soundCache.values()) {
        delete sound;
    }
    m_soundCache.clear();
    
    emit cacheCleared("sound");
}

// 字体资源管理方法
bool ResourceManager::loadFont(const QString& fontPath) {
    if (!m_cacheEnabled) {
        return true; // 不缓存，直接返回成功
    }
    
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId == -1) {
        Logger::instance()->log("ResourceManager", QString("Failed to load font: %1").arg(fontPath));
        return false;
    }
    
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont font(fontFamilies.first());
        m_fontCache[fontPath] = font;
        updateResourceUsage(fontPath, QFileInfo(fontPath).size());
    }
    
    return true;
}

QFont ResourceManager::getFont(const QString& fontPath, int size) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_fontCache.contains(fontPath)) {
        QFont font = m_fontCache[fontPath];
        font.setPointSize(size);
        return font;
    }
    
    // 如果不在缓存中，尝试加载
    if (loadFont(fontPath)) {
        QFont font = m_fontCache[fontPath];
        font.setPointSize(size);
        return font;
    }
    
    return QFont();
}

bool ResourceManager::preloadFonts(const QStringList& fontPaths) {
    bool success = true;
    for (const QString& fontPath : fontPaths) {
        success &= loadFont(fontPath);
    }
    return success;
}

void ResourceManager::clearFontCache() {
    QMutexLocker locker(&m_resourceMutex);
    m_fontCache.clear();
    emit cacheCleared("font");
}

// 动画资源管理方法
bool ResourceManager::loadAnimation(const QString& animationPath) {
    if (!m_cacheEnabled) {
        return true; // 不缓存，直接返回成功
    }
    
    QMovie* movie = new QMovie(animationPath);
    if (!movie->isValid()) {
        delete movie;
        Logger::instance()->log("ResourceManager", QString("Failed to load animation: %1").arg(animationPath));
        return false;
    }
    
    m_animationCache[animationPath] = movie;
    updateResourceUsage(animationPath, QFileInfo(animationPath).size());
    
    return true;
}

QMovie* ResourceManager::getAnimation(const QString& animationPath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_animationCache.contains(animationPath)) {
        return m_animationCache[animationPath];
    }
    
    // 如果不在缓存中，尝试加载
    if (loadAnimation(animationPath)) {
        return m_animationCache[animationPath];
    }
    
    return nullptr;
}

bool ResourceManager::preloadAnimations(const QStringList& animationPaths) {
    bool success = true;
    for (const QString& animationPath : animationPaths) {
        success &= loadAnimation(animationPath);
    }
    return success;
}

void ResourceManager::clearAnimationCache() {
    QMutexLocker locker(&m_resourceMutex);
    
    // 删除QMovie对象
    for (auto movie : m_animationCache.values()) {
        delete movie;
    }
    m_animationCache.clear();
    
    emit cacheCleared("animation");
}

// 数据资源管理方法
bool ResourceManager::loadDataFile(const QString& dataPath) {
    if (!m_cacheEnabled) {
        return true; // 不缓存，直接返回成功
    }
    
    QFile file(dataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ResourceManager", QString("Failed to load data file: %1").arg(dataPath));
        return false;
    }
    
    QByteArray data = file.readAll();
    m_dataCache[dataPath] = data;
    updateResourceUsage(dataPath, data.size());
    
    return true;
}

QByteArray ResourceManager::getDataFile(const QString& dataPath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_dataCache.contains(dataPath)) {
        return m_dataCache[dataPath];
    }
    
    // 如果不在缓存中，尝试加载
    if (loadDataFile(dataPath)) {
        return m_dataCache[dataPath];
    }
    
    return QByteArray();
}

bool ResourceManager::preloadDataFiles(const QStringList& dataPaths) {
    bool success = true;
    for (const QString& dataPath : dataPaths) {
        success &= loadDataFile(dataPath);
    }
    return success;
}

void ResourceManager::clearDataCache() {
    QMutexLocker locker(&m_resourceMutex);
    m_dataCache.clear();
    emit cacheCleared("data");
}

// 资源包管理方法
bool ResourceManager::loadResourcePack(const QString& packPath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (isResourcePackLoaded(packPath)) {
        return true;
    }
    
    // 这里应该实现资源包的加载逻辑
    // 暂时简单地将包路径添加到已加载列表中
    m_loadedPacks.append(packPath);
    
    emit resourcePackLoaded(packPath);
    return true;
}

bool ResourceManager::unloadResourcePack(const QString& packPath) {
    QMutexLocker locker(&m_resourceMutex);
    
    if (!isResourcePackLoaded(packPath)) {
        return false;
    }
    
    m_loadedPacks.removeOne(packPath);
    
    emit resourcePackUnloaded(packPath);
    return true;
}

QStringList ResourceManager::getLoadedResourcePacks() const {
    QMutexLocker locker(&m_resourceMutex);
    return m_loadedPacks;
}

bool ResourceManager::isResourcePackLoaded(const QString& packPath) const {
    QMutexLocker locker(&m_resourceMutex);
    return m_loadedPacks.contains(packPath);
}

// 资源预加载方法
void ResourceManager::preloadResources(const QString& resourceType, const QStringList& resourcePaths) {
    if (m_isPreloading) {
        cancelPreload();
    }
    
    m_isPreloading = true;
    m_preloadCurrent = 0;
    m_preloadTotal = resourcePaths.size();
    
    // 使用异步方式预加载
    m_preloadWatcher = new QFutureWatcher<void>(this);
    connect(m_preloadWatcher, &QFutureWatcher<void>::finished, this, &ResourceManager::onAsyncOperationCompleted);
    
    QFuture<void> future = QtConcurrent::run([this, resourceType, resourcePaths]() {
        for (int i = 0; i < resourcePaths.size(); ++i) {
            if (resourceType == "image") {
                loadImage(resourcePaths[i]);
            } else if (resourceType == "sound") {
                loadSound(resourcePaths[i]);
            } else if (resourceType == "font") {
                loadFont(resourcePaths[i]);
            } else if (resourceType == "animation") {
                loadAnimation(resourcePaths[i]);
            } else if (resourceType == "data") {
                loadDataFile(resourcePaths[i]);
            }
            
            m_preloadCurrent = i + 1;
            emit preloadProgress(m_preloadCurrent, m_preloadTotal);
        }
    });
    
    m_preloadWatcher->setFuture(future);
}

void ResourceManager::preloadAllResources() {
    // 预加载所有类型的资源
    QStringList imagePaths = {"images/background.png", "images/character.png", "images/ui.png"};
    QStringList soundPaths = {"sounds/music.wav", "sounds/sfx.wav"};
    QStringList fontPaths = {"fonts/main.ttf"};
    
    preloadResources("image", imagePaths);
    preloadResources("sound", soundPaths);
    preloadResources("font", fontPaths);
}

void ResourceManager::cancelPreload() {
    if (m_preloadWatcher && m_preloadWatcher->isRunning()) {
        m_preloadWatcher->cancel();
        m_preloadWatcher->waitForFinished();
    }
    m_isPreloading = false;
}

bool ResourceManager::isPreloading() const {
    return m_isPreloading;
}

// 资源缓存管理方法
void ResourceManager::enableCache(bool enable) {
    m_cacheEnabled = enable;
    
    if (!enable) {
        clearCache();
    }
}

bool ResourceManager::isCacheEnabled() const {
    return m_cacheEnabled;
}

void ResourceManager::clearCache(const QString& resourceType) {
    if (resourceType.isEmpty() || resourceType == "image") {
        clearImageCache();
    }
    if (resourceType.isEmpty() || resourceType == "sound") {
        clearSoundCache();
    }
    if (resourceType.isEmpty() || resourceType == "font") {
        clearFontCache();
    }
    if (resourceType.isEmpty() || resourceType == "animation") {
        clearAnimationCache();
    }
    if (resourceType.isEmpty() || resourceType == "data") {
        clearDataCache();
    }
}

void ResourceManager::setCacheSize(int maxSize) {
    m_maxCacheSize = maxSize;
    cleanupExpiredCache();
}

int ResourceManager::getCacheSize() const {
    return m_maxCacheSize;
}

void ResourceManager::setCachePolicy(const QString& policy) {
    m_cachePolicy = policy;
}

QString ResourceManager::getCachePolicy() const {
    return m_cachePolicy;
}

// 资源监控方法
void ResourceManager::enableResourceMonitoring(bool enable) {
    m_monitoringEnabled = enable;
}

bool ResourceManager::isResourceMonitoringEnabled() const {
    return m_monitoringEnabled;
}

QVariantMap ResourceManager::getResourceUsage() const {
    QMutexLocker locker(&m_resourceMutex);
    QVariantMap usage;
    
    for (auto it = m_resourceUsage.begin(); it != m_resourceUsage.end(); ++it) {
        usage[it.key()] = it.value();
    }
    
    return usage;
}

QVariantMap ResourceManager::getResourceStats() const {
    QMutexLocker locker(&m_resourceMutex);
    QVariantMap stats;
    
    stats["total_resources"] = m_resourceIndex.size();
    stats["cached_images"] = m_imageCache.size();
    stats["cached_sounds"] = m_soundCache.size();
    stats["cached_fonts"] = m_fontCache.size();
    stats["cached_animations"] = m_animationCache.size();
    stats["cached_data"] = m_dataCache.size();
    stats["loaded_packs"] = m_loadedPacks.size();
    
    return stats;
}

// 资源压缩方法
bool ResourceManager::compressResource(const QString& resourcePath) {
    // 这里应该实现资源压缩逻辑
    // 暂时返回true表示成功
    return true;
}

bool ResourceManager::decompressResource(const QString& resourcePath) {
    // 这里应该实现资源解压逻辑
    // 暂时返回true表示成功
    return true;
}

bool ResourceManager::isResourceCompressed(const QString& resourcePath) const {
    // 这里应该检查资源是否被压缩
    // 暂时返回false
    return false;
}

// 资源验证方法
bool ResourceManager::validateResource(const QString& resourcePath) {
    QString fullPath = findResource(resourcePath);
    if (fullPath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(fullPath);
    return fileInfo.exists() && fileInfo.isReadable();
}

QStringList ResourceManager::getResourceErrors(const QString& resourcePath) {
    QStringList errors;
    QString fullPath = findResource(resourcePath);
    
    if (fullPath.isEmpty()) {
        errors << "Resource not found";
    } else {
        QFileInfo fileInfo(fullPath);
        if (!fileInfo.exists()) {
            errors << "File does not exist";
        } else if (!fileInfo.isReadable()) {
            errors << "File is not readable";
        }
    }
    
    return errors;
}

bool ResourceManager::repairResource(const QString& resourcePath) {
    // 这里应该实现资源修复逻辑
    // 暂时返回true表示成功
    return true;
}

// 资源同步方法
void ResourceManager::enableAutoSync(bool enable) {
    m_autoSyncEnabled = enable;
    
    if (enable) {
        m_autoSyncTimer->start(m_syncInterval * 1000);
    } else {
        m_autoSyncTimer->stop();
    }
}

bool ResourceManager::isAutoSyncEnabled() const {
    return m_autoSyncEnabled;
}

void ResourceManager::syncResources() {
    // 这里应该实现资源同步逻辑
    // 暂时只是保存资源索引
    saveResourceIndex();
    
    emit syncCompleted();
}

void ResourceManager::setSyncInterval(int seconds) {
    m_syncInterval = seconds;
    
    if (m_autoSyncEnabled) {
        m_autoSyncTimer->stop();
        m_autoSyncTimer->start(m_syncInterval * 1000);
    }
}

int ResourceManager::getSyncInterval() const {
    return m_syncInterval;
}

// 资源备份方法
bool ResourceManager::backupResources(const QString& backupPath) {
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        backupDir.mkpath(".");
    }
    
    bool success = true;
    
    // 备份缓存目录
    if (QDir(m_cacheDir).exists()) {
        success &= FileUtils::copyDirectory(m_cacheDir, backupPath + "/cache");
    }
    
    // 备份资源索引
    success &= FileUtils::copyFile(m_cacheDir + "/resource_index.json", 
                                 backupPath + "/resource_index.json");
    
    emit backupCompleted(success);
    return success;
}

bool ResourceManager::restoreResources(const QString& backupPath) {
    bool success = true;
    
    // 恢复缓存目录
    QString backupCacheDir = backupPath + "/cache";
    if (QDir(backupCacheDir).exists()) {
        success &= FileUtils::copyDirectory(backupCacheDir, m_cacheDir);
    }
    
    // 恢复资源索引
    success &= FileUtils::copyFile(backupPath + "/resource_index.json", 
                                 m_cacheDir + "/resource_index.json");
    
    if (success) {
        loadResourceIndex();
    }
    
    emit restoreCompleted(success);
    return success;
}

bool ResourceManager::exportResources(const QString& exportPath, const QStringList& resourceTypes) {
    QDir exportDir(exportPath);
    if (!exportDir.exists()) {
        exportDir.mkpath(".");
    }
    
    bool success = true;
    QStringList typesToExport = resourceTypes.isEmpty() ? 
        QStringList{"image", "sound", "font", "animation", "data"} : resourceTypes;
    
    for (const QString& resourceType : typesToExport) {
        QStringList resources = getLoadedResources(resourceType);
        for (const QString& resource : resources) {
            QString fullPath = findResource(resource);
            if (!fullPath.isEmpty()) {
                QString targetPath = exportPath + "/" + resourceType + "/" + QFileInfo(resource).fileName();
                QDir().mkpath(QFileInfo(targetPath).absolutePath());
                success &= FileUtils::copyFile(fullPath, targetPath);
            }
        }
    }
    
    return success;
}

bool ResourceManager::importResources(const QString& importPath) {
    bool success = true;
    QStringList resourceTypes = {"image", "sound", "font", "animation", "data"};
    
    for (const QString& resourceType : resourceTypes) {
        QString sourceDir = importPath + "/" + resourceType;
        if (QDir(sourceDir).exists()) {
            QStringList files = QDir(sourceDir).entryList(QDir::Files);
            for (const QString& file : files) {
                QString sourcePath = sourceDir + "/" + file;
                QString targetPath = m_cacheDir + "/" + resourceType + "/" + file;
                QDir().mkpath(QFileInfo(targetPath).absolutePath());
                success &= FileUtils::copyFile(sourcePath, targetPath);
            }
        }
    }
    
    return success;
}

// 资源清理方法
void ResourceManager::clearUnusedResources() {
    QMutexLocker locker(&m_resourceMutex);
    
    // 清理长时间未使用的资源
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-7);
    
    QStringList keysToRemove;
    for (auto it = m_cacheTimestamps.begin(); it != m_cacheTimestamps.end(); ++it) {
        if (it.value() < cutoffTime) {
            keysToRemove.append(it.key());
        }
    }
    
    for (const QString& key : keysToRemove) {
        unloadResource(key);
    }
}

void ResourceManager::clearOldResources(int daysOld) {
    QDateTime cutoffTime = QDateTime::currentDateTime().addDays(-daysOld);
    
    QStringList keysToRemove;
    for (auto it = m_cacheTimestamps.begin(); it != m_cacheTimestamps.end(); ++it) {
        if (it.value() < cutoffTime) {
            keysToRemove.append(it.key());
        }
    }
    
    for (const QString& key : keysToRemove) {
        unloadResource(key);
    }
}

void ResourceManager::optimizeResources() {
    // 这里应该实现资源优化逻辑
    // 暂时只是清理缓存
    cleanupExpiredCache();
}

// 异步资源操作方法
QFuture<bool> ResourceManager::loadResourceAsync(const QString& resourcePath, const QString& resourceType) {
    return QtConcurrent::run([this, resourcePath, resourceType]() {
        return loadResource(resourcePath, resourceType);
    });
}

QFuture<bool> ResourceManager::preloadResourcesAsync(const QString& resourceType, const QStringList& resourcePaths) {
    return QtConcurrent::run([this, resourceType, resourcePaths]() {
        for (const QString& path : resourcePaths) {
            loadResource(path, resourceType);
        }
        return true;
    });
}

QFuture<bool> ResourceManager::compressResourceAsync(const QString& resourcePath) {
    return QtConcurrent::run([this, resourcePath]() {
        return compressResource(resourcePath);
    });
}

QFuture<bool> ResourceManager::validateResourceAsync(const QString& resourcePath) {
    return QtConcurrent::run([this, resourcePath]() {
        return validateResource(resourcePath);
    });
}

// 资源路径管理方法
void ResourceManager::addResourcePath(const QString& path) {
    QMutexLocker locker(&m_resourceMutex);
    if (!m_resourcePaths.contains(path)) {
        m_resourcePaths.append(path);
    }
}

void ResourceManager::removeResourcePath(const QString& path) {
    QMutexLocker locker(&m_resourceMutex);
    m_resourcePaths.removeOne(path);
}

QStringList ResourceManager::getResourcePaths() const {
    QMutexLocker locker(&m_resourceMutex);
    return m_resourcePaths;
}

QString ResourceManager::findResource(const QString& resourceName) const {
    QMutexLocker locker(&m_resourceMutex);
    
    for (const QString& path : m_resourcePaths) {
        QString fullPath = path + "/" + resourceName;
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }
    
    return QString();
}

// 资源信息方法
QVariantMap ResourceManager::getResourceInfo(const QString& resourcePath) const {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_resourceIndex.contains(resourcePath)) {
        return m_resourceIndex[resourcePath].toMap();
    }
    
    return QVariantMap();
}

qint64 ResourceManager::getResourceSize(const QString& resourcePath) const {
    QFileInfo fileInfo(resourcePath);
    return fileInfo.size();
}

QString ResourceManager::getResourceType(const QString& resourcePath) const {
    QMutexLocker locker(&m_resourceMutex);
    
    if (m_resourceIndex.contains(resourcePath)) {
        return m_resourceIndex[resourcePath].toMap()["type"].toString();
    }
    
    // 根据文件扩展名推断类型
    QString extension = QFileInfo(resourcePath).suffix().toLower();
    if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "bmp") {
        return "image";
    } else if (extension == "wav" || extension == "mp3" || extension == "ogg") {
        return "sound";
    } else if (extension == "ttf" || extension == "otf") {
        return "font";
    } else if (extension == "gif") {
        return "animation";
    } else {
        return "data";
    }
}

QDateTime ResourceManager::getResourceModificationTime(const QString& resourcePath) const {
    QFileInfo fileInfo(resourcePath);
    return fileInfo.lastModified();
}

// 私有方法
void ResourceManager::onAutoSyncTimer() {
    if (m_autoSyncEnabled) {
        syncResources();
    }
}

void ResourceManager::onAsyncOperationCompleted() {
    // 清理完成的异步操作
    auto it = m_asyncWatchers.begin();
    while (it != m_asyncWatchers.end()) {
        if ((*it)->isFinished()) {
            delete *it;
            it = m_asyncWatchers.erase(it);
        } else {
            ++it;
        }
    }
    
    // 检查预加载是否完成
    if (m_preloadWatcher && m_preloadWatcher->isFinished()) {
        m_isPreloading = false;
        emit preloadCompleted();
    }
}

void ResourceManager::onPreloadProgress(int current, int total) {
    m_preloadCurrent = current;
    m_preloadTotal = total;
    emit preloadProgress(current, total);
}

QString ResourceManager::getResourceCachePath(const QString& resourcePath) const {
    QString resourceType = getResourceType(resourcePath);
    return QString("%1/%2/%3").arg(m_cacheDir, resourceType, QFileInfo(resourcePath).fileName());
}

bool ResourceManager::ensureCacheDirectory(const QString& resourceType) {
    QString cacheTypeDir = m_cacheDir + "/" + resourceType;
    QDir dir(cacheTypeDir);
    
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    
    return true;
}

void ResourceManager::updateResourceUsage(const QString& resourcePath, qint64 size) {
    if (m_monitoringEnabled) {
        m_resourceUsage[resourcePath] = size;
        m_resourceAccessCount[resourcePath]++;
    }
    
    m_cacheSizes[resourcePath] = size;
    m_cacheTimestamps[resourcePath] = QDateTime::currentDateTime();
}

void ResourceManager::cleanupExpiredCache() {
    if (m_cachePolicy == "lru") {
        // 最近最少使用策略
        QList<QPair<QString, QDateTime>> sortedCache;
        for (auto it = m_cacheTimestamps.begin(); it != m_cacheTimestamps.end(); ++it) {
            sortedCache.append(qMakePair(it.key(), it.value()));
        }
        
        std::sort(sortedCache.begin(), sortedCache.end(), 
                 [](const QPair<QString, QDateTime>& a, const QPair<QString, QDateTime>& b) {
                     return a.second < b.second;
                 });
        
        // 移除最旧的数据直到缓存大小符合要求
        while (m_cacheSizes.size() > m_maxCacheSize && !sortedCache.isEmpty()) {
            QString key = sortedCache.takeFirst().first;
            unloadResource(key);
        }
    }
} 