/**
 * @file main.cpp
 * @brief 幻境传说客户端主程序入口
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStyleFactory>
#include "ui/windows/MainWindow.h"
#include "utils/Logger.h"

using namespace Fantasy;

int main(int argc, char *argv[])
{
    // 创建Qt应用程序
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Fantasy Legend");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Fantasy Legend");
    app.setOrganizationDomain("fantasylegend.com");
    
    // 初始化日志系统
    Logger::initialize();
    Logger::setLogLevel(LogLevel::DEBUG);
    
    CLIENT_LOG_INFO("Starting Fantasy Legend Client v1.0.0");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("幻境传说 - 一个充满魔法的冒险世界");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加自定义选项
    QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen",
                                       "以全屏模式启动");
    parser.addOption(fullscreenOption);
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                  "启用调试模式");
    parser.addOption(debugOption);
    
    QCommandLineOption logLevelOption(QStringList() << "l" << "log-level",
                                     "设置日志级别 (debug, info, warn, error, fatal)",
                                     "info");
    parser.addOption(logLevelOption);
    
    // 解析命令行参数
    parser.process(app);
    
    // 设置日志级别
    QString logLevel = parser.value(logLevelOption);
    if (logLevel == "debug") {
        Logger::setLogLevel(LogLevel::DEBUG);
    } else if (logLevel == "info") {
        Logger::setLogLevel(LogLevel::INFO);
    } else if (logLevel == "warn") {
        Logger::setLogLevel(LogLevel::WARN);
    } else if (logLevel == "error") {
        Logger::setLogLevel(LogLevel::ERROR);
    } else if (logLevel == "fatal") {
        Logger::setLogLevel(LogLevel::FATAL);
    }
    
    // 检查调试模式
    if (parser.isSet(debugOption)) {
        CLIENT_LOG_INFO("Debug mode enabled");
    }
    
    try {
        // 设置应用程序样式
        app.setStyle(QStyleFactory::create("Fusion"));
        
        // 创建主窗口
        MainWindow* mainWindow = new MainWindow();
        
        // 检查全屏选项
        if (parser.isSet(fullscreenOption)) {
            mainWindow->setFullscreen(true);
        }
        
        // 显示主窗口
        mainWindow->show();
        
        CLIENT_LOG_INFO("Main window displayed successfully");
        
        // 运行应用程序
        int result = app.exec();
        
        CLIENT_LOG_INFO("Application exited with code: %d", result);
        
        // 清理资源
        delete mainWindow;
        
        return result;
        
    } catch (const std::exception& e) {
        CLIENT_LOG_FATAL("Fatal error: %s", e.what());
        
        QMessageBox::critical(nullptr, "错误",
                             QString("程序发生致命错误：\n%1").arg(e.what()));
        
        return -1;
        
    } catch (...) {
        CLIENT_LOG_FATAL("Unknown fatal error occurred");
        
        QMessageBox::critical(nullptr, "错误",
                             "程序发生未知错误");
        
        return -1;
    }
}
