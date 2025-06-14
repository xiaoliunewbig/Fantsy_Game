#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QImage>
#include <QPixmap>
#include <QSound>
#include <QFont>
#include <QMovie>

class ResourceManager : public QObject {
    Q_OBJECT
    
public:
    static ResourceManager* instance();
    
    // 资源管理
    bool loadResource(const QString& resourcePath, const QString& resourceType);
    bool unloadResource(const QString& resourcePath);
    bool isResourceLoaded(const QString& resourcePath) const;
    QStringList getLoadedResources(const QString& resourceType = QString()) const;
    
    // 图片资源管理
    bool loadImage(const QString& imagePath);
    QPixmap getImage(const QString& imagePath);
    QImage getImageData(const QString& imagePath);
    bool preloadImages(const QStringList& imagePaths);
    void clearImageCache();
    
    // 音频资源管理
    bool loadSound(const QString& soundPath);
    QSound* getSound(const QString& soundPath);
    bool preloadSounds(const QStringList& soundPaths);
    void clearSoundCache();
    
    // 字体资源管理
    bool loadFont(const QString& fontPath);
    QFont getFont(const QString& fontPath, int size = 12);
    bool preloadFonts(const QStringList& fontPaths);
    void clearFontCache();
    
    // 动画资源管理
    bool loadAnimation(const QString& animationPath);
    QMovie* getAnimation(const QString& animationPath);
    bool preloadAnimations(const QStringList& animationPaths);
    void clearAnimationCache();
    
    // 数据资源管理
    bool loadDataFile(const QString& dataPath);
    QByteArray getDataFile(const QString& dataPath);
    bool preloadDataFiles(const QStringList& dataPaths);
    void clearDataCache();
    
    // 资源包管理
    bool loadResourcePack(const QString& packPath);
    bool unloadResourcePack(const QString& packPath);
    QStringList getLoadedResourcePacks() const;
    bool isResourcePackLoaded(const QString& packPath) const;
    
    // 资源预加载
    void preloadResources(const QString& resourceType, const QStringList& resourcePaths);
    void preloadAllResources();
    void cancelPreload();
    bool isPreloading() const;
    
    // 资源缓存管理
    void enableCache(bool enable);
    bool isCacheEnabled() const;
    void clearCache(const QString& resourceType = QString());
    void setCacheSize(int maxSize);
    int getCacheSize() const;
    void setCachePolicy(const QString& policy);
    QString getCachePolicy() const;
    
    // 资源监控
    void enableResourceMonitoring(bool enable);
    bool isResourceMonitoringEnabled() const;
    QVariantMap getResourceUsage() const;
    QVariantMap getResourceStats() const;
    
    // 资源压缩
    bool compressResource(const QString& resourcePath);
    bool decompressResource(const QString& resourcePath);
    bool isResourceCompressed(const QString& resourcePath) const;
    
    // 资源验证
    bool validateResource(const QString& resourcePath);
    QStringList getResourceErrors(const QString& resourcePath);
    bool repairResource(const QString& resourcePath);
    
    // 资源同步
    void enableAutoSync(bool enable);
    bool isAutoSyncEnabled() const;
    void syncResources();
    void setSyncInterval(int seconds);
    int getSyncInterval() const;
    
    // 资源备份
    bool backupResources(const QString& backupPath);
    bool restoreResources(const QString& backupPath);
    bool exportResources(const QString& exportPath, const QStringList& resourceTypes = QStringList());
    bool importResources(const QString& importPath);
    
    // 资源清理
    void clearUnusedResources();
    void clearOldResources(int daysOld);
    void optimizeResources();
    
    // 异步资源操作
    QFuture<bool> loadResourceAsync(const QString& resourcePath, const QString& resourceType);
    QFuture<bool> preloadResourcesAsync(const QString& resourceType, const QStringList& resourcePaths);
    QFuture<bool> compressResourceAsync(const QString& resourcePath);
    QFuture<bool> validateResourceAsync(const QString& resourcePath);
    
    // 资源路径管理
    void addResourcePath(const QString& path);
    void removeResourcePath(const QString& path);
    QStringList getResourcePaths() const;
    QString findResource(const QString& resourceName) const;
    
    // 资源信息
    QVariantMap getResourceInfo(const QString& resourcePath) const;
    qint64 getResourceSize(const QString& resourcePath) const;
    QString getResourceType(const QString& resourcePath) const;
    QDateTime getResourceModificationTime(const QString& resourcePath) const;
    
signals:
    void resourceLoaded(const QString& resourcePath, const QString& resourceType);
    void resourceUnloaded(const QString& resourcePath);
    void resourceError(const QString& resourcePath, const QString& error);
    void preloadProgress(int current, int total);
    void preloadCompleted();
    void cacheCleared(const QString& resourceType);
    void resourcePackLoaded(const QString& packPath);
    void resourcePackUnloaded(const QString& packPath);
    void syncCompleted();
    void backupCompleted(bool success);
    void restoreCompleted(bool success);
    
private slots:
    void onAutoSyncTimer();
    void onAsyncOperationCompleted();
    void onPreloadProgress(int current, int total);
    
private:
    ResourceManager();
    ~ResourceManager();
    
    void initializeResourcePaths();
    void setupAutoSync();
    void loadResourceIndex();
    void saveResourceIndex();
    QString getResourceCachePath(const QString& resourcePath) const;
    bool ensureCacheDirectory(const QString& resourceType);
    void updateResourceUsage(const QString& resourcePath, qint64 size);
    void cleanupExpiredCache();
    
    // 资源存储
    QMap<QString, QPixmap> m_imageCache;
    QMap<QString, QSound*> m_soundCache;
    QMap<QString, QFont> m_fontCache;
    QMap<QString, QMovie*> m_animationCache;
    QMap<QString, QByteArray> m_dataCache;
    
    // 资源包
    QMap<QString, QStringList> m_resourcePacks;
    QStringList m_loadedPacks;
    
    // 资源路径
    QStringList m_resourcePaths;
    QString m_cacheDir;
    QString m_backupDir;
    
    // 缓存管理
    bool m_cacheEnabled;
    int m_maxCacheSize;
    QString m_cachePolicy;
    QMap<QString, QDateTime> m_cacheTimestamps;
    QMap<QString, qint64> m_cacheSizes;
    
    // 自动同步
    QTimer* m_autoSyncTimer;
    bool m_autoSyncEnabled;
    int m_syncInterval;
    
    // 资源监控
    bool m_monitoringEnabled;
    QMap<QString, qint64> m_resourceUsage;
    QMap<QString, int> m_resourceAccessCount;
    
    // 预加载
    QFutureWatcher<void>* m_preloadWatcher;
    bool m_isPreloading;
    int m_preloadCurrent;
    int m_preloadTotal;
    
    // 异步操作
    QList<QFutureWatcher<void>*> m_asyncWatchers;
    
    // 资源索引
    QVariantMap m_resourceIndex;
    
    // 线程安全
    mutable QMutex m_resourceMutex;
    
    static ResourceManager* s_instance;
}; 