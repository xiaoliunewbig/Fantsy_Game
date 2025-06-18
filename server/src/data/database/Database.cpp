/**
 * @file Database.cpp
 * @brief 数据库实现 - SQLite后端
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/data/database/Database.h"
#include "include/utils/resources/ResourceLogger.h"
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <numeric>

namespace Fantasy {

// SQLite数据库连接实现
class SQLiteConnection : public DatabaseConnection {
public:
    SQLiteConnection(const DatabaseConfig& config) : config_(config), db_(nullptr), inTransaction_(false) {
        stats_ = DatabaseStats{};
    }
    
    ~SQLiteConnection() {
        disconnect();
    }
    
    bool connect() override {
        if (isConnected()) {
            return true;
        }
        
        state_ = ConnectionState::CONNECTING;
        
        int result = sqlite3_open_v2(config_.database.c_str(), &db_,
                                   SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                   nullptr);
        
        if (result != SQLITE_OK) {
            state_ = ConnectionState::ERROR;
            lastError_ = sqlite3_errmsg(db_);
            DB_LOG_ERROR("Failed to connect to SQLite database: {}", lastError_);
            return false;
        }
        
        // 设置配置
        sqlite3_busy_timeout(db_, static_cast<int>(config_.queryTimeout.count()));
        
        // 启用外键约束
        sqlite3_exec(db_, "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);
        
        // 设置WAL模式以提高并发性能
        sqlite3_exec(db_, "PRAGMA journal_mode = WAL", nullptr, nullptr, nullptr);
        
        // 设置缓存大小
        sqlite3_exec(db_, "PRAGMA cache_size = 10000", nullptr, nullptr, nullptr);
        
        state_ = ConnectionState::CONNECTED;
        DB_LOG_INFO("Connected to SQLite database: {}", config_.database);
        return true;
    }
    
    void disconnect() override {
        if (db_) {
            if (inTransaction_) {
                rollbackTransaction();
            }
            sqlite3_close(db_);
            db_ = nullptr;
        }
        state_ = ConnectionState::DISCONNECTED;
        DB_LOG_INFO("Disconnected from SQLite database");
    }
    
    bool isConnected() const override {
        return db_ != nullptr && state_ == ConnectionState::CONNECTED;
    }
    
    ConnectionState getState() const override {
        return state_;
    }
    
    DatabaseResult executeQuery(const std::string& query) override {
        if (!isConnected()) {
            return DatabaseResult{{}, {}, 0, 0, false, "Not connected"};
        }
        
        auto startTime = std::chrono::steady_clock::now();
        
        sqlite3_stmt* stmt = nullptr;
        int result = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
        
        if (result != SQLITE_OK) {
            std::string error = sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            updateStats(false, startTime);
            return DatabaseResult{{}, {}, 0, 0, false, error};
        }
        
        DatabaseResult dbResult;
        dbResult.success = true;
        
        // 获取列名
        int columnCount = sqlite3_column_count(stmt);
        for (int i = 0; i < columnCount; ++i) {
            dbResult.columnNames.push_back(sqlite3_column_name(stmt, i));
        }
        
        // 获取数据
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            DatabaseRow row;
            for (int i = 0; i < columnCount; ++i) {
                row.push_back(getColumnValue(stmt, i));
            }
            dbResult.rows.push_back(row);
        }
        
        dbResult.affectedRows = sqlite3_changes(db_);
        dbResult.lastInsertId = sqlite3_last_insert_rowid(db_);
        
        sqlite3_finalize(stmt);
        updateStats(true, startTime);
        
        return dbResult;
    }
    
    DatabaseResult executePreparedQuery(const std::string& query, const std::vector<DatabaseValue>& params) override {
        if (!isConnected()) {
            return DatabaseResult{{}, {}, 0, 0, false, "Not connected"};
        }
        
        auto startTime = std::chrono::steady_clock::now();
        
        sqlite3_stmt* stmt = nullptr;
        int result = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
        
        if (result != SQLITE_OK) {
            std::string error = sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            updateStats(false, startTime);
            return DatabaseResult{{}, {}, 0, 0, false, error};
        }
        
        // 绑定参数
        for (size_t i = 0; i < params.size(); ++i) {
            bindParameter(stmt, static_cast<int>(i + 1), params[i]);
        }
        
        DatabaseResult dbResult;
        dbResult.success = true;
        
        // 获取列名
        int columnCount = sqlite3_column_count(stmt);
        for (int i = 0; i < columnCount; ++i) {
            dbResult.columnNames.push_back(sqlite3_column_name(stmt, i));
        }
        
        // 获取数据
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            DatabaseRow row;
            for (int i = 0; i < columnCount; ++i) {
                row.push_back(getColumnValue(stmt, i));
            }
            dbResult.rows.push_back(row);
        }
        
        dbResult.affectedRows = sqlite3_changes(db_);
        dbResult.lastInsertId = sqlite3_last_insert_rowid(db_);
        
        sqlite3_finalize(stmt);
        updateStats(true, startTime);
        
        return dbResult;
    }
    
    std::future<DatabaseResult> executeQueryAsync(const std::string& query) override {
        return std::async(std::launch::async, [this, query]() {
            return executeQuery(query);
        });
    }
    
    std::future<DatabaseResult> executePreparedQueryAsync(const std::string& query, const std::vector<DatabaseValue>& params) override {
        return std::async(std::launch::async, [this, query, params]() {
            return executePreparedQuery(query, params);
        });
    }
    
    bool beginTransaction() override {
        if (!isConnected() || inTransaction_) {
            return false;
        }
        
        auto result = executeQuery("BEGIN TRANSACTION");
        if (result.success) {
            inTransaction_ = true;
            DB_LOG_DEBUG("Transaction started");
        }
        return result.success;
    }
    
    bool commitTransaction() override {
        if (!isConnected() || !inTransaction_) {
            return false;
        }
        
        auto result = executeQuery("COMMIT");
        if (result.success) {
            inTransaction_ = false;
            DB_LOG_DEBUG("Transaction committed");
        }
        return result.success;
    }
    
    bool rollbackTransaction() override {
        if (!isConnected() || !inTransaction_) {
            return false;
        }
        
        auto result = executeQuery("ROLLBACK");
        if (result.success) {
            inTransaction_ = false;
            DB_LOG_DEBUG("Transaction rolled back");
        }
        return result.success;
    }
    
    bool isInTransaction() const override {
        return inTransaction_;
    }
    
    bool prepareStatement(const std::string& name, const std::string& query) override {
        if (!isConnected()) {
            return false;
        }
        
        sqlite3_stmt* stmt = nullptr;
        int result = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
        
        if (result != SQLITE_OK) {
            lastError_ = sqlite3_errmsg(db_);
            return false;
        }
        
        preparedStatements_[name] = stmt;
        return true;
    }
    
    bool executePreparedStatement(const std::string& name, const std::vector<DatabaseValue>& params) override {
        auto it = preparedStatements_.find(name);
        if (it == preparedStatements_.end()) {
            return false;
        }
        
        sqlite3_stmt* stmt = it->second;
        sqlite3_reset(stmt);
        
        // 绑定参数
        for (size_t i = 0; i < params.size(); ++i) {
            bindParameter(stmt, static_cast<int>(i + 1), params[i]);
        }
        
        return sqlite3_step(stmt) == SQLITE_DONE;
    }
    
    void closePreparedStatement(const std::string& name) override {
        auto it = preparedStatements_.find(name);
        if (it != preparedStatements_.end()) {
            sqlite3_finalize(it->second);
            preparedStatements_.erase(it);
        }
    }
    
    std::string getLastError() const override {
        return lastError_;
    }
    
    uint64_t getLastInsertId() const override {
        return db_ ? sqlite3_last_insert_rowid(db_) : 0;
    }
    
    size_t getAffectedRows() const override {
        return db_ ? sqlite3_changes(db_) : 0;
    }
    
    DatabaseStats getStats() const override {
        return stats_;
    }
    
    void resetStats() override {
        stats_ = DatabaseStats{};
    }
    
    DatabaseConfig getConfig() const override {
        return config_;
    }
    
    void setConfig(const DatabaseConfig& config) override {
        config_ = config;
    }

private:
    DatabaseConfig config_;
    sqlite3* db_;
    ConnectionState state_;
    std::string lastError_;
    bool inTransaction_;
    DatabaseStats stats_;
    std::unordered_map<std::string, sqlite3_stmt*> preparedStatements_;
    
    DatabaseValue getColumnValue(sqlite3_stmt* stmt, int column) {
        int type = sqlite3_column_type(stmt, column);
        
        switch (type) {
            case SQLITE_INTEGER:
                return static_cast<int>(sqlite3_column_int64(stmt, column));
            case SQLITE_FLOAT:
                return sqlite3_column_double(stmt, column);
            case SQLITE_TEXT:
                return std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column)));
            case SQLITE_BLOB: {
                const void* data = sqlite3_column_blob(stmt, column);
                int size = sqlite3_column_bytes(stmt, column);
                std::vector<uint8_t> blob(static_cast<const uint8_t*>(data), 
                                        static_cast<const uint8_t*>(data) + size);
                return blob;
            }
            case SQLITE_NULL:
            default:
                return std::nullptr_t{};
        }
    }
    
    void bindParameter(sqlite3_stmt* stmt, int index, const DatabaseValue& value) {
        if (std::holds_alternative<std::nullptr_t>(value)) {
            sqlite3_bind_null(stmt, index);
        } else if (std::holds_alternative<int>(value)) {
            sqlite3_bind_int64(stmt, index, std::get<int>(value));
        } else if (std::holds_alternative<double>(value)) {
            sqlite3_bind_double(stmt, index, std::get<double>(value));
        } else if (std::holds_alternative<std::string>(value)) {
            const std::string& str = std::get<std::string>(value);
            sqlite3_bind_text(stmt, index, str.c_str(), -1, SQLITE_TRANSIENT);
        } else if (std::holds_alternative<bool>(value)) {
            sqlite3_bind_int(stmt, index, std::get<bool>(value) ? 1 : 0);
        } else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
            const std::vector<uint8_t>& blob = std::get<std::vector<uint8_t>>(value);
            sqlite3_bind_blob(stmt, index, blob.data(), static_cast<int>(blob.size()), SQLITE_TRANSIENT);
        }
    }
    
    void updateStats(bool success, std::chrono::steady_clock::time_point startTime) {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        stats_.totalQueries++;
    if (success) {
            stats_.successfulQueries++;
        } else {
            stats_.failedQueries++;
        }
        
        stats_.totalQueryTime += duration;
        stats_.averageQueryTime = std::chrono::milliseconds(
            stats_.totalQueryTime.count() / stats_.totalQueries);
        stats_.lastQueryTime = std::chrono::system_clock::now();
    }
};

// SQLite连接池实现
class SQLiteConnectionPool : public DatabaseConnectionPool {
public:
    SQLiteConnectionPool(const DatabaseConfig& config) : config_(config), initialized_(false) {}
    
    bool initialize(const DatabaseConfig& config) override {
        config_ = config;
        initialized_ = true;
        return true;
    }
    
    void shutdown() override {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.clear();
        initialized_ = false;
    }
    
    std::shared_ptr<DatabaseConnection> getConnection() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 查找空闲连接
        for (auto& conn : connections_) {
            if (!conn->isConnected()) {
                if (conn->connect()) {
                    return conn;
                }
            }
        }
        
        // 创建新连接
        if (connections_.size() < config_.maxConnections) {
            auto conn = std::make_shared<SQLiteConnection>(config_);
            if (conn->connect()) {
                connections_.push_back(conn);
                return conn;
            }
        }
        
        return nullptr;
    }
    
    void releaseConnection(std::shared_ptr<DatabaseConnection> connection) override {
        // 连接池会自动管理连接
    }
    
    size_t getActiveConnections() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& conn : connections_) {
            if (conn->isConnected()) {
                count++;
            }
        }
        return count;
    }
    
    size_t getIdleConnections() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& conn : connections_) {
            if (!conn->isConnected()) {
                count++;
            }
        }
        return count;
    }
    
    size_t getTotalConnections() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.size();
    }
    
    bool isHealthy() const override {
        return initialized_ && getActiveConnections() > 0;
    }
    
    DatabaseConfig getConfig() const override {
        return config_;
    }
    
    void setConfig(const DatabaseConfig& config) override {
        config_ = config;
    }
    
    DatabaseStats getStats() const override {
        return stats_;
    }
    
    void resetStats() override {
        stats_ = DatabaseStats{};
    }

private:
    DatabaseConfig config_;
    bool initialized_;
    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<SQLiteConnection>> connections_;
    DatabaseStats stats_;
};

// SQLite数据库实现
class SQLiteDatabase : public Database {
public:
    SQLiteDatabase(const DatabaseConfig& config) : config_(config), initialized_(false) {
        // 延迟初始化连接池
    }
    
    ~SQLiteDatabase() {
        shutdown();
    }
    
    bool initialize(const DatabaseConfig& config) override {
        config_ = config;
        
        // 创建连接池
        connectionPool_ = std::make_unique<SQLiteConnectionPool>(config);
        
        if (!connectionPool_->initialize(config)) {
            DB_LOG_ERROR("Failed to initialize SQLite connection pool");
            return false;
        }
        
        initialized_ = true;
        DB_LOG_INFO("SQLite database initialized: {}", config.database);
        return true;
    }
    
    void shutdown() override {
        if (connectionPool_) {
            connectionPool_->shutdown();
        }
        initialized_ = false;
        DB_LOG_INFO("SQLite database shutdown");
    }
    
    bool isInitialized() const override {
        return initialized_;
    }
    
    std::shared_ptr<DatabaseConnection> getConnection() override {
        return connectionPool_->getConnection();
    }
    
    void releaseConnection(std::shared_ptr<DatabaseConnection> connection) override {
        connectionPool_->releaseConnection(connection);
    }
    
    DatabaseResult query(const std::string& sql) override {
        auto connection = getConnection();
        if (!connection) {
            return DatabaseResult{{}, {}, 0, 0, false, "No available connection"};
        }
        
        auto result = connection->executeQuery(sql);
        releaseConnection(connection);
        return result;
    }
    
    DatabaseResult query(const std::string& sql, const std::vector<DatabaseValue>& params) override {
        auto connection = getConnection();
        if (!connection) {
            return DatabaseResult{{}, {}, 0, 0, false, "No available connection"};
        }
        
        auto result = connection->executePreparedQuery(sql, params);
        releaseConnection(connection);
        return result;
    }
    
    std::future<DatabaseResult> queryAsync(const std::string& sql) override {
        auto connection = getConnection();
        if (!connection) {
            return std::async(std::launch::async, []() {
                return DatabaseResult{{}, {}, 0, 0, false, "No available connection"};
            });
        }
        
        return std::async(std::launch::async, [this, connection, sql]() {
            auto result = connection->executeQuery(sql);
            releaseConnection(connection);
            return result;
        });
    }
    
    std::future<DatabaseResult> queryAsync(const std::string& sql, const std::vector<DatabaseValue>& params) override {
        auto connection = getConnection();
        if (!connection) {
            return std::async(std::launch::async, []() {
                return DatabaseResult{{}, {}, 0, 0, false, "No available connection"};
            });
        }
        
        return std::async(std::launch::async, [this, connection, sql, params]() {
            auto result = connection->executePreparedQuery(sql, params);
            releaseConnection(connection);
            return result;
        });
    }
    
    bool beginTransaction() override {
        auto connection = getConnection();
        if (!connection) {
            return false;
        }
        
        bool result = connection->beginTransaction();
        if (result) {
            currentTransaction_ = connection;
    } else {
            releaseConnection(connection);
        }
        return result;
    }
    
    bool commitTransaction() override {
        if (!currentTransaction_) {
            return false;
        }
        
        bool result = currentTransaction_->commitTransaction();
        releaseConnection(currentTransaction_);
        currentTransaction_.reset();
        return result;
    }
    
    bool rollbackTransaction() override {
        if (!currentTransaction_) {
            return false;
        }
        
        bool result = currentTransaction_->rollbackTransaction();
        releaseConnection(currentTransaction_);
        currentTransaction_.reset();
        return result;
    }
    
    bool isInTransaction() const override {
        return currentTransaction_ != nullptr;
    }
    
    bool createTable(const std::string& tableName, const std::string& schema) override {
        std::string sql = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + schema + ")";
        return query(sql).success;
    }
    
    bool dropTable(const std::string& tableName) override {
        std::string sql = "DROP TABLE IF EXISTS " + tableName;
        return query(sql).success;
    }
    
    bool tableExists(const std::string& tableName) override {
        std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";
        auto result = query(sql, {tableName});
        return !result.rows.empty();
    }
    
    std::vector<std::string> getTableNames() override {
        std::string sql = "SELECT name FROM sqlite_master WHERE type='table'";
        auto result = query(sql);
        std::vector<std::string> tables;
        for (const auto& row : result.rows) {
            if (!row.empty() && std::holds_alternative<std::string>(row[0])) {
                tables.push_back(std::get<std::string>(row[0]));
            }
        }
        return tables;
    }
    
    std::string getTableSchema(const std::string& tableName) override {
        std::string sql = "SELECT sql FROM sqlite_master WHERE type='table' AND name=?";
        auto result = query(sql, {tableName});
        if (!result.rows.empty() && !result.rows[0].empty() && 
            std::holds_alternative<std::string>(result.rows[0][0])) {
            return std::get<std::string>(result.rows[0][0]);
        }
        return "";
    }
    
    uint64_t insert(const std::string& tableName, const std::unordered_map<std::string, DatabaseValue>& data) override {
        if (data.empty()) {
            return 0;
        }
        
        std::vector<std::string> columns;
        std::vector<std::string> placeholders;
        std::vector<DatabaseValue> values;
        
        for (const auto& [key, value] : data) {
            columns.push_back(key);
            placeholders.push_back("?");
            values.push_back(value);
        }
        
        std::string sql = "INSERT INTO " + tableName + " (" + 
                         DatabaseUtils::buildInsertQuery(tableName, columns) + ") VALUES (" +
                         std::accumulate(placeholders.begin(), placeholders.end(), std::string(),
                                       [](const std::string& a, const std::string& b) {
                                           return a.empty() ? b : a + ", " + b;
                                       }) + ")";
        
        auto result = query(sql, values);
        return result.success ? result.lastInsertId : 0;
    }
    
    bool update(const std::string& tableName, const std::unordered_map<std::string, DatabaseValue>& data, 
                const std::string& whereClause) override {
        if (data.empty()) {
        return false;
    }
    
        std::vector<std::string> setClauses;
        std::vector<DatabaseValue> values;
        
        for (const auto& [key, value] : data) {
            setClauses.push_back(key + " = ?");
            values.push_back(value);
        }
        
        std::string sql = "UPDATE " + tableName + " SET " +
                         std::accumulate(setClauses.begin(), setClauses.end(), std::string(),
                                       [](const std::string& a, const std::string& b) {
                                           return a.empty() ? b : a + ", " + b;
                                       });
        
        if (!whereClause.empty()) {
            sql += " WHERE " + whereClause;
        }
        
        return query(sql, values).success;
    }
    
    bool delete_(const std::string& tableName, const std::string& whereClause) override {
        std::string sql = "DELETE FROM " + tableName;
        if (!whereClause.empty()) {
            sql += " WHERE " + whereClause;
        }
        return query(sql).success;
    }
    
    DatabaseResult select(const std::string& tableName, const std::vector<std::string>& columns, 
                         const std::string& whereClause) override {
        std::string sql = DatabaseUtils::buildSelectQuery(tableName, columns, whereClause);
        return query(sql);
    }
    
    bool insertBatch(const std::string& tableName, 
                    const std::vector<std::unordered_map<std::string, DatabaseValue>>& data) override {
        if (data.empty()) {
            return true;
        }
        
        auto connection = getConnection();
        if (!connection) {
            return false;
        }
        
        if (!connection->beginTransaction()) {
            releaseConnection(connection);
        return false;
    }
    
        bool success = true;
        for (const auto& row : data) {
            if (insert(tableName, row) == 0) {
                success = false;
                break;
            }
        }
        
    if (success) {
            connection->commitTransaction();
        } else {
            connection->rollbackTransaction();
    }
        
        releaseConnection(connection);
    return success;
}

    bool updateBatch(const std::string& tableName, 
                    const std::vector<std::pair<std::unordered_map<std::string, DatabaseValue>, std::string>>& data) override {
        if (data.empty()) {
            return true;
        }
        
        auto connection = getConnection();
        if (!connection) {
        return false;
    }
    
        if (!connection->beginTransaction()) {
            releaseConnection(connection);
            return false;
        }
        
        bool success = true;
        for (const auto& [rowData, whereClause] : data) {
            if (!update(tableName, rowData, whereClause)) {
                success = false;
                break;
            }
        }
        
    if (success) {
            connection->commitTransaction();
        } else {
            connection->rollbackTransaction();
    }
        
        releaseConnection(connection);
    return success;
}

    DatabaseConfig getConfig() const override {
        return config_;
    }
    
    DatabaseStats getStats() const override {
        return connectionPool_->getStats();
    }
    
    std::string getLastError() const override {
        return lastError_;
    }
    
    bool ping() override {
        return query("SELECT 1").success;
    }
    
    bool backup(const std::string& backupPath) override {
        auto connection = getConnection();
        if (!connection) {
        return false;
    }
    
        // SQLite备份需要特殊处理
        // 这里简化实现，实际应该使用sqlite3_backup_* API
        return true;
    }
    
    bool restore(const std::string& backupPath) override {
        // 简化实现
        return true;
    }
    
    bool optimize() override {
        return query("VACUUM").success;
    }
    
    bool vacuum() override {
        return query("VACUUM").success;
    }
    
    bool analyze() override {
        return query("ANALYZE").success;
    }
    
    void setConnectionStateCallback(ConnectionStateCallback callback) override {
        connectionStateCallback_ = callback;
    }
    
    void setQueryCallback(QueryCallback callback) override {
        queryCallback_ = callback;
    }

private:
    DatabaseConfig config_;
    bool initialized_;
    std::string lastError_;
    std::shared_ptr<DatabaseConnection> currentTransaction_;
    std::unique_ptr<SQLiteConnectionPool> connectionPool_;
    ConnectionStateCallback connectionStateCallback_;
    QueryCallback queryCallback_;
};

// 数据库工厂实现
std::unique_ptr<Database> DatabaseFactory::createDatabase(DatabaseType type) {
    switch (type) {
        case DatabaseType::SQLITE:
            return std::make_unique<SQLiteDatabase>(DatabaseConfig{});
        case DatabaseType::MYSQL:
        case DatabaseType::POSTGRESQL:
        default:
            DB_LOG_ERROR("Database type not supported yet: {}", static_cast<int>(type));
            return nullptr;
    }
}

std::unique_ptr<DatabaseConnection> DatabaseFactory::createConnection(const DatabaseConfig& config) {
    switch (config.type) {
        case DatabaseType::SQLITE:
            return std::make_unique<SQLiteConnection>(config);
        case DatabaseType::MYSQL:
        case DatabaseType::POSTGRESQL:
        default:
            DB_LOG_ERROR("Database type not supported yet: {}", static_cast<int>(config.type));
            return nullptr;
    }
}

std::unique_ptr<DatabaseConnectionPool> DatabaseFactory::createConnectionPool(const DatabaseConfig& config) {
    // 简化实现，返回nullptr
    return nullptr;
}

DatabaseType DatabaseFactory::detectType(const std::string& connectionString) {
    if (connectionString.find("sqlite") != std::string::npos) {
        return DatabaseType::SQLITE;
    } else if (connectionString.find("mysql") != std::string::npos) {
        return DatabaseType::MYSQL;
    } else if (connectionString.find("postgresql") != std::string::npos) {
        return DatabaseType::POSTGRESQL;
    }
    return DatabaseType::UNKNOWN;
}

std::string DatabaseFactory::getTypeString(DatabaseType type) {
    switch (type) {
        case DatabaseType::SQLITE: return "SQLite";
        case DatabaseType::MYSQL: return "MySQL";
        case DatabaseType::POSTGRESQL: return "PostgreSQL";
        default: return "Unknown";
    }
}

bool DatabaseFactory::validateConfig(const DatabaseConfig& config) {
    if (config.database.empty()) {
        return false;
    }
    
    switch (config.type) {
        case DatabaseType::SQLITE:
            return true; // SQLite只需要数据库文件路径
        case DatabaseType::MYSQL:
        case DatabaseType::POSTGRESQL:
            return !config.host.empty() && !config.username.empty();
        default:
            return false;
    }
}

std::string DatabaseFactory::getConfigError(const DatabaseConfig& config) {
    if (config.database.empty()) {
        return "Database path is required";
    }
    
    switch (config.type) {
        case DatabaseType::SQLITE:
            return "";
        case DatabaseType::MYSQL:
        case DatabaseType::POSTGRESQL:
            if (config.host.empty()) return "Host is required";
            if (config.username.empty()) return "Username is required";
            break;
        default:
            return "Unsupported database type";
    }
    
    return "";
}

// 数据库工具类实现
std::string DatabaseUtils::toString(const DatabaseValue& value) {
    if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
    if (std::holds_alternative<int>(value)) return std::to_string(std::get<int>(value));
    if (std::holds_alternative<double>(value)) return std::to_string(std::get<double>(value));
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    if (std::holds_alternative<std::vector<uint8_t>>(value)) return "[BLOB]";
    return "NULL";
}

int DatabaseUtils::toInt(const DatabaseValue& value, int defaultValue) {
    if (std::holds_alternative<int>(value)) return std::get<int>(value);
    if (std::holds_alternative<std::string>(value)) {
        try {
            return std::stoi(std::get<std::string>(value));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

double DatabaseUtils::toDouble(const DatabaseValue& value, double defaultValue) {
    if (std::holds_alternative<double>(value)) return std::get<double>(value);
    if (std::holds_alternative<int>(value)) return static_cast<double>(std::get<int>(value));
    if (std::holds_alternative<std::string>(value)) {
        try {
            return std::stod(std::get<std::string>(value));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool DatabaseUtils::toBool(const DatabaseValue& value, bool defaultValue) {
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<int>(value)) return std::get<int>(value) != 0;
    if (std::holds_alternative<std::string>(value)) {
        const std::string& str = std::get<std::string>(value);
        return str == "true" || str == "1" || str == "yes";
    }
    return defaultValue;
}

std::vector<uint8_t> DatabaseUtils::toBlob(const DatabaseValue& value) {
    if (std::holds_alternative<std::vector<uint8_t>>(value)) {
        return std::get<std::vector<uint8_t>>(value);
    }
    return {};
}

DatabaseValue DatabaseUtils::createValue(const std::string& value) { return value; }
DatabaseValue DatabaseUtils::createValue(int value) { return value; }
DatabaseValue DatabaseUtils::createValue(double value) { return value; }
DatabaseValue DatabaseUtils::createValue(bool value) { return value; }
DatabaseValue DatabaseUtils::createValue(const std::vector<uint8_t>& value) { return value; }
DatabaseValue DatabaseUtils::createNull() { return std::nullptr_t{}; }

std::string DatabaseUtils::escapeString(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    
    return result;
}

std::string DatabaseUtils::buildWhereClause(const std::unordered_map<std::string, DatabaseValue>& conditions) {
    if (conditions.empty()) {
        return "";
    }
    
    std::vector<std::string> clauses;
    for (const auto& [key, _] : conditions) {
        clauses.push_back(key + " = ?");
    }
    
    return std::accumulate(clauses.begin(), clauses.end(), std::string(),
                          [](const std::string& a, const std::string& b) {
                              return a.empty() ? b : a + " AND " + b;
                          });
}

std::string DatabaseUtils::buildInsertQuery(const std::string& tableName, const std::vector<std::string>& columns) {
    return std::accumulate(columns.begin(), columns.end(), std::string(),
                          [](const std::string& a, const std::string& b) {
                              return a.empty() ? b : a + ", " + b;
                          });
}

std::string DatabaseUtils::buildUpdateQuery(const std::string& tableName, const std::vector<std::string>& columns) {
    std::vector<std::string> setClauses;
    for (const auto& column : columns) {
        setClauses.push_back(column + " = ?");
    }
    
    return std::accumulate(setClauses.begin(), setClauses.end(), std::string(),
                          [](const std::string& a, const std::string& b) {
                              return a.empty() ? b : a + ", " + b;
                          });
}

std::string DatabaseUtils::buildSelectQuery(const std::string& tableName, const std::vector<std::string>& columns, 
                                           const std::string& whereClause) {
    std::string sql = "SELECT ";
    
    if (columns.empty()) {
        sql += "*";
    } else {
        sql += std::accumulate(columns.begin(), columns.end(), std::string(),
                              [](const std::string& a, const std::string& b) {
                                  return a.empty() ? b : a + ", " + b;
                              });
    }
    
    sql += " FROM " + tableName;
    
    if (!whereClause.empty()) {
        sql += " WHERE " + whereClause;
    }
    
    return sql;
}

std::optional<DatabaseValue> DatabaseUtils::getValue(const DatabaseRow& row, size_t index) {
    if (index >= row.size()) {
        return std::nullopt;
    }
    return row[index];
}

std::optional<DatabaseValue> DatabaseUtils::getValue(const DatabaseRow& row, const std::string& columnName, 
                                                    const std::vector<std::string>& columnNames) {
    auto it = std::find(columnNames.begin(), columnNames.end(), columnName);
    if (it == columnNames.end()) {
        return std::nullopt;
    }
    
    size_t index = std::distance(columnNames.begin(), it);
    return getValue(row, index);
}

std::unordered_map<std::string, DatabaseValue> DatabaseUtils::rowToMap(const DatabaseRow& row, 
                                                                       const std::vector<std::string>& columnNames) {
    std::unordered_map<std::string, DatabaseValue> result;
    
    for (size_t i = 0; i < row.size() && i < columnNames.size(); ++i) {
        result[columnNames[i]] = row[i];
    }
    
    return result;
}

bool DatabaseUtils::isValidTableName(const std::string& tableName) {
    if (tableName.empty()) return false;
    
    // 检查是否包含SQL关键字
    std::vector<std::string> keywords = {"SELECT", "INSERT", "UPDATE", "DELETE", "DROP", "CREATE", "ALTER"};
    std::string upperName = tableName;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    
    for (const auto& keyword : keywords) {
        if (upperName == keyword) return false;
    }
    
    return true;
}

bool DatabaseUtils::isValidColumnName(const std::string& columnName) {
    return !columnName.empty() && columnName.find(' ') == std::string::npos;
}

bool DatabaseUtils::isValidSQL(const std::string& sql) {
    return !sql.empty() && sql.length() < 1024 * 1024; // 1MB限制
}

} // namespace Fantasy

