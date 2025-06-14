#include "Logger.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <iostream>

Logger* Logger::s_instance = nullptr;
QMutex Logger::s_mutex;

Logger* Logger::instance() {
    if (!s_instance) {
        QMutexLocker locker(&s_mutex);
        if (!s_instance) {
            s_instance = new Logger();
        }
    }
    return s_instance;
}

Logger::Logger()
    : m_logLevel(LogLevel::INFO)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_consoleOutput(true)
    , m_fileOutput(true)
    , m_dateFormat("yyyy-MM-dd hh:mm:ss.zzz")
    , m_messageFormat("[%timestamp%] [%level%] %message%") {
    
    // 设置默认日志文件路径
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    m_logFilePath = logDir + "/fantasy_legend.log";
    
    // 初始化日志文件
    if (m_fileOutput) {
        setLogFile(m_logFilePath);
    }
}

Logger::~Logger() {
    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

void Logger::debug(const QString& message) {
    instance()->log(LogLevel::DEBUG, message);
}

void Logger::info(const QString& message) {
    instance()->log(LogLevel::INFO, message);
}

void Logger::warning(const QString& message) {
    instance()->log(LogLevel::WARNING, message);
}

void Logger::error(const QString& message) {
    instance()->log(LogLevel::ERROR, message);
}

void Logger::critical(const QString& message) {
    instance()->log(LogLevel::CRITICAL, message);
}

void Logger::setLogLevel(LogLevel level) {
    m_logLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return m_logLevel;
}

void Logger::setLogFile(const QString& filePath) {
    QMutexLocker locker(&s_mutex);
    
    // 关闭现有文件
    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    
    // 打开新文件
    m_logFilePath = filePath;
    m_logFile = new QFile(m_logFilePath);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setCodec("UTF-8");
    } else {
        qWarning() << "Failed to open log file:" << filePath;
        delete m_logFile;
        m_logFile = nullptr;
    }
}

void Logger::enableConsoleOutput(bool enable) {
    m_consoleOutput = enable;
}

void Logger::enableFileOutput(bool enable) {
    m_fileOutput = enable;
    if (enable && !m_logFile) {
        setLogFile(m_logFilePath);
    }
}

void Logger::setDateFormat(const QString& format) {
    m_dateFormat = format;
}

void Logger::setMessageFormat(const QString& format) {
    m_messageFormat = format;
}

void Logger::log(LogLevel level, const QString& message) {
    if (level < m_logLevel) {
        return;
    }
    
    QString formattedMessage = formatMessage(level, message);
    
    // 控制台输出
    if (m_consoleOutput) {
        switch (level) {
            case LogLevel::DEBUG:
                qDebug().noquote() << formattedMessage;
                break;
            case LogLevel::INFO:
                qInfo().noquote() << formattedMessage;
                break;
            case LogLevel::WARNING:
                qWarning().noquote() << formattedMessage;
                break;
            case LogLevel::ERROR:
                qCritical().noquote() << formattedMessage;
                break;
            case LogLevel::CRITICAL:
                std::cerr << formattedMessage.toStdString() << std::endl;
                break;
        }
    }
    
    // 文件输出
    if (m_fileOutput && m_logStream) {
        QMutexLocker locker(&s_mutex);
        *m_logStream << formattedMessage << Qt::endl;
        m_logStream->flush();
    }
    
    // 发送信号
    emit logMessage(level, message);
}

QString Logger::formatMessage(LogLevel level, const QString& message) {
    QString formatted = m_messageFormat;
    formatted.replace("%timestamp%", getCurrentTimestamp());
    formatted.replace("%level%", levelToString(level));
    formatted.replace("%message%", message);
    return formatted;
}

QString Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

QString Logger::getCurrentTimestamp() {
    return QDateTime::currentDateTime().toString(m_dateFormat);
}
