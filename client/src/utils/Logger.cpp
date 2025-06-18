/**
 * @file Logger.cpp
 * @brief 日志工具实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/Logger.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>

namespace Fantasy {

// 静态成员初始化
LogLevel Logger::s_currentLevel = LogLevel::INFO;
bool Logger::s_initialized = false;

void Logger::initialize()
{
    if (s_initialized) {
        return;
    }
    
    // 创建日志目录
    QDir logDir("logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    
    s_initialized = true;
    info("Logger initialized");
}

void Logger::setLogLevel(LogLevel level)
{
    s_currentLevel = level;
}

void Logger::debug(const QString& message)
{
    if (s_currentLevel <= LogLevel::DEBUG) {
        QString formattedMessage = formatMessage(LogLevel::DEBUG, message);
        qDebug().noquote() << formattedMessage;
    }
}

void Logger::info(const QString& message)
{
    if (s_currentLevel <= LogLevel::INFO) {
        QString formattedMessage = formatMessage(LogLevel::INFO, message);
        qInfo().noquote() << formattedMessage;
    }
}

void Logger::warn(const QString& message)
{
    if (s_currentLevel <= LogLevel::WARN) {
        QString formattedMessage = formatMessage(LogLevel::WARN, message);
        qWarning().noquote() << formattedMessage;
    }
}

void Logger::error(const QString& message)
{
    if (s_currentLevel <= LogLevel::ERROR) {
        QString formattedMessage = formatMessage(LogLevel::ERROR, message);
        qCritical().noquote() << formattedMessage;
    }
}

void Logger::fatal(const QString& message)
{
    if (s_currentLevel <= LogLevel::FATAL) {
        QString formattedMessage = formatMessage(LogLevel::FATAL, message);
        qCritical().noquote() << formattedMessage;
    }
}

QString Logger::formatMessage(LogLevel level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr;
    
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            levelStr = "INFO ";
            break;
        case LogLevel::WARN:
            levelStr = "WARN ";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
        case LogLevel::FATAL:
            levelStr = "FATAL";
            break;
    }
    
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

} // namespace Fantasy
