/**
 * @file Database.h
 * @brief 数据库接口定义 - 支持多种数据库后端
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 支持多种数据库类型 (SQLite, MySQL, PostgreSQL)
 * - 连接池管理
 * - 事务支持
 * - 预处理语句
 * - 异步查询
 * - 连接状态监控
 * - 错误处理和重试机制
 * - 性能监控和统计
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <future>
#include <chrono>
#include <variant>
#include <optional>
#include <mutex>
#include <atomic>

namespace Fantasy {

// 数据库类型枚举
enum class DatabaseType {
    SQLITE,
    MYSQL,
    POSTGRESQL,
    UNKNOWN
};

// 数据库值类型
using DatabaseValue = std::variant<std::string, int, double, bool, std::vector<uint8_t>, std::nullptr_t>;

// 数据库行类型
using DatabaseRow = std::vector<DatabaseValue>;

// 数据库结果集
struct DatabaseResult {
    std::vector<DatabaseRow> rows;
    std::vector<std::string> columnNames;
    size_t affectedRows;
    uint64_t lastInsertId;
    bool success;
    std::string errorMessage;
};

// 数据库连接配置
struct DatabaseConfig {
    DatabaseType type = DatabaseType::SQLITE;
    std::string host = "localhost";
    uint16_t port = 3306;
    std::string database;
    std::string username;
    std::string password;
    std::string charset = "utf8mb4";
    size_t maxConnections = 10;
    size_t minConnections = 2;
    std::chrono::milliseconds connectionTimeout{5000};
    std::chrono::milliseconds queryTimeout{30000};
    bool enableSSL = false;
    std::string sslCert;
    std::string sslKey;
    std::string sslCA;
    bool enableCompression = true;
    size_t maxQuerySize = 1024 * 1024; // 1MB
    bool enableLogging = true;
    bool enablePerformanceMonitoring = true;
};

// 数据库连接状态
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR,
    CLOSING
};

// 数据库统计信息
struct DatabaseStats {
    size_t totalQueries;
    size_t successfulQueries;
    size_t failedQueries;
    size_t activeConnections;
    size_t idleConnections;
    std::chrono::milliseconds averageQueryTime;
    std::chrono::milliseconds totalQueryTime;
    size_t totalBytesTransferred;
    std::chrono::system_clock::time_point lastQueryTime;
};

// 查询回调函数类型
using QueryCallback = std::function<void(const DatabaseResult&)>;

// 连接状态回调函数类型
using ConnectionStateCallback = std::function<void(ConnectionState)>;

// 数据库异常类
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message) : std::runtime_error(message) {}
    explicit DatabaseException(const char* message) : std::runtime_error(message) {}
};

/**
 * @brief 数据库连接接口
 */
class DatabaseConnection {
public:
    virtual ~DatabaseConnection() = default;
    
    // 连接管理
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual ConnectionState getState() const = 0;
    
    // 查询执行
    virtual DatabaseResult executeQuery(const std::string& query) = 0;
    virtual DatabaseResult executePreparedQuery(const std::string& query, const std::vector<DatabaseValue>& params) = 0;
    
    // 异步查询
    virtual std::future<DatabaseResult> executeQueryAsync(const std::string& query) = 0;
    virtual std::future<DatabaseResult> executePreparedQueryAsync(const std::string& query, const std::vector<DatabaseValue>& params) = 0;
    
    // 事务管理
    virtual bool beginTransaction() = 0;
    virtual bool commitTransaction() = 0;
    virtual bool rollbackTransaction() = 0;
    virtual bool isInTransaction() const = 0;
    
    // 预处理语句
    virtual bool prepareStatement(const std::string& name, const std::string& query) = 0;
    virtual bool executePreparedStatement(const std::string& name, const std::vector<DatabaseValue>& params) = 0;
    virtual void closePreparedStatement(const std::string& name) = 0;
    
    // 连接信息
    virtual std::string getLastError() const = 0;
    virtual uint64_t getLastInsertId() const = 0;
    virtual size_t getAffectedRows() const = 0;
    
    // 性能监控
    virtual DatabaseStats getStats() const = 0;
    virtual void resetStats() = 0;
    
    // 连接配置
    virtual DatabaseConfig getConfig() const = 0;
    virtual void setConfig(const DatabaseConfig& config) = 0;
};

/**
 * @brief 数据库连接池
 */
class DatabaseConnectionPool {
public:
    virtual ~DatabaseConnectionPool() = default;
    
    // 连接池管理
    virtual bool initialize(const DatabaseConfig& config) = 0;
    virtual void shutdown() = 0;
    
    // 连接获取和释放
    virtual std::shared_ptr<DatabaseConnection> getConnection() = 0;
    virtual void releaseConnection(std::shared_ptr<DatabaseConnection> connection) = 0;
    
    // 连接池状态
    virtual size_t getActiveConnections() const = 0;
    virtual size_t getIdleConnections() const = 0;
    virtual size_t getTotalConnections() const = 0;
    virtual bool isHealthy() const = 0;
    
    // 连接池配置
    virtual DatabaseConfig getConfig() const = 0;
    virtual void setConfig(const DatabaseConfig& config) = 0;
    
    // 连接池统计
    virtual DatabaseStats getStats() const = 0;
    virtual void resetStats() = 0;
};

/**
 * @brief 数据库主接口
 */
class Database {
public:
    virtual ~Database() = default;
    
    // 数据库管理
    virtual bool initialize(const DatabaseConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
    
    // 连接管理
    virtual std::shared_ptr<DatabaseConnection> getConnection() = 0;
    virtual void releaseConnection(std::shared_ptr<DatabaseConnection> connection) = 0;
    
    // 便捷查询方法
    virtual DatabaseResult query(const std::string& sql) = 0;
    virtual DatabaseResult query(const std::string& sql, const std::vector<DatabaseValue>& params) = 0;
    virtual std::future<DatabaseResult> queryAsync(const std::string& sql) = 0;
    virtual std::future<DatabaseResult> queryAsync(const std::string& sql, const std::vector<DatabaseValue>& params) = 0;
    
    // 事务管理
    virtual bool beginTransaction() = 0;
    virtual bool commitTransaction() = 0;
    virtual bool rollbackTransaction() = 0;
    virtual bool isInTransaction() const = 0;
    
    // 数据库操作
    virtual bool createTable(const std::string& tableName, const std::string& schema) = 0;
    virtual bool dropTable(const std::string& tableName) = 0;
    virtual bool tableExists(const std::string& tableName) = 0;
    virtual std::vector<std::string> getTableNames() = 0;
    virtual std::string getTableSchema(const std::string& tableName) = 0;
    
    // 数据操作
    virtual uint64_t insert(const std::string& tableName, const std::unordered_map<std::string, DatabaseValue>& data) = 0;
    virtual bool update(const std::string& tableName, const std::unordered_map<std::string, DatabaseValue>& data, const std::string& whereClause) = 0;
    virtual bool delete_(const std::string& tableName, const std::string& whereClause) = 0;
    virtual DatabaseResult select(const std::string& tableName, const std::vector<std::string>& columns = {}, const std::string& whereClause = "") = 0;
    
    // 批量操作
    virtual bool insertBatch(const std::string& tableName, const std::vector<std::unordered_map<std::string, DatabaseValue>>& data) = 0;
    virtual bool updateBatch(const std::string& tableName, const std::vector<std::pair<std::unordered_map<std::string, DatabaseValue>, std::string>>& data) = 0;
    
    // 数据库信息
    virtual DatabaseConfig getConfig() const = 0;
    virtual DatabaseStats getStats() const = 0;
    virtual std::string getLastError() const = 0;
    virtual bool ping() = 0;
    
    // 备份和恢复
    virtual bool backup(const std::string& backupPath) = 0;
    virtual bool restore(const std::string& backupPath) = 0;
    
    // 性能优化
    virtual bool optimize() = 0;
    virtual bool vacuum() = 0;
    virtual bool analyze() = 0;
    
    // 事件回调
    virtual void setConnectionStateCallback(ConnectionStateCallback callback) = 0;
    virtual void setQueryCallback(QueryCallback callback) = 0;
};

// 数据库工厂类
class DatabaseFactory {
public:
    static std::unique_ptr<Database> createDatabase(DatabaseType type);
    static std::unique_ptr<DatabaseConnection> createConnection(const DatabaseConfig& config);
    static std::unique_ptr<DatabaseConnectionPool> createConnectionPool(const DatabaseConfig& config);
    
    // 数据库类型检测
    static DatabaseType detectType(const std::string& connectionString);
    static std::string getTypeString(DatabaseType type);
    
    // 配置验证
    static bool validateConfig(const DatabaseConfig& config);
    static std::string getConfigError(const DatabaseConfig& config);
};

// 数据库工具类
class DatabaseUtils {
public:
    // 值转换
    static std::string toString(const DatabaseValue& value);
    static int toInt(const DatabaseValue& value, int defaultValue = 0);
    static double toDouble(const DatabaseValue& value, double defaultValue = 0.0);
    static bool toBool(const DatabaseValue& value, bool defaultValue = false);
    static std::vector<uint8_t> toBlob(const DatabaseValue& value);
    
    // 值创建
    static DatabaseValue createValue(const std::string& value);
    static DatabaseValue createValue(int value);
    static DatabaseValue createValue(double value);
    static DatabaseValue createValue(bool value);
    static DatabaseValue createValue(const std::vector<uint8_t>& value);
    static DatabaseValue createNull();
    
    // SQL工具
    static std::string escapeString(const std::string& str);
    static std::string buildWhereClause(const std::unordered_map<std::string, DatabaseValue>& conditions);
    static std::string buildInsertQuery(const std::string& tableName, const std::vector<std::string>& columns);
    static std::string buildUpdateQuery(const std::string& tableName, const std::vector<std::string>& columns);
    static std::string buildSelectQuery(const std::string& tableName, const std::vector<std::string>& columns, const std::string& whereClause = "");
    
    // 结果处理
    static std::optional<DatabaseValue> getValue(const DatabaseRow& row, size_t index);
    static std::optional<DatabaseValue> getValue(const DatabaseRow& row, const std::string& columnName, const std::vector<std::string>& columnNames);
    static std::unordered_map<std::string, DatabaseValue> rowToMap(const DatabaseRow& row, const std::vector<std::string>& columnNames);
    
    // 验证
    static bool isValidTableName(const std::string& tableName);
    static bool isValidColumnName(const std::string& columnName);
    static bool isValidSQL(const std::string& sql);
};

// 数据库日志宏
#define DB_LOG_TRACE(msg, ...) \
    FANTASY_LOG_TRACE("[Database] " msg, ##__VA_ARGS__)

#define DB_LOG_DEBUG(msg, ...) \
    FANTASY_LOG_DEBUG("[Database] " msg, ##__VA_ARGS__)

#define DB_LOG_INFO(msg, ...) \
    FANTASY_LOG_INFO("[Database] " msg, ##__VA_ARGS__)

#define DB_LOG_WARN(msg, ...) \
    FANTASY_LOG_WARN("[Database] " msg, ##__VA_ARGS__)

#define DB_LOG_ERROR(msg, ...) \
    FANTASY_LOG_ERROR("[Database] " msg, ##__VA_ARGS__)

} // namespace Fantasy
