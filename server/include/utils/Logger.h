#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger : public QObject {
    Q_OBJECT
    
public:
    static Logger* instance();
    
    // 日志记录方法
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    static void critical(const QString& message);
    
    // 日志级别控制
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    // 日志文件控制
    void setLogFile(const QString& filePath);
    void enableConsoleOutput(bool enable);
    void enableFileOutput(bool enable);
    
    // 日志格式化
    void setDateFormat(const QString& format);
    void setMessageFormat(const QString& format);
    
signals:
    void logMessage(LogLevel level, const QString& message);
    
private:
    Logger();
    ~Logger();
    
    void log(LogLevel level, const QString& message);
    QString formatMessage(LogLevel level, const QString& message);
    QString levelToString(LogLevel level);
    QString getCurrentTimestamp();
    
    static Logger* s_instance;
    static QMutex s_mutex;
    
    LogLevel m_logLevel;
    QString m_logFilePath;
    QFile* m_logFile;
    QTextStream* m_logStream;
    bool m_consoleOutput;
    bool m_fileOutput;
    QString m_dateFormat;
    QString m_messageFormat;
};
