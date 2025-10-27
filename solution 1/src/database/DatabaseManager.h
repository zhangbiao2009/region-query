#pragma once

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "../data/Point.h"

/**
 * Manages PostgreSQL database connections and operations
 */
class DatabaseManager {
private:
    std::unique_ptr<pqxx::connection> connection;
    std::string connection_string;

public:
    /**
     * Initialize database connection
     * @param conn_str PostgreSQL connection string
     */
    explicit DatabaseManager(const std::string& conn_str);
    
    /**
     * Destructor - ensures connection is properly closed
     */
    ~DatabaseManager();
    
    /**
     * Test database connection
     * @return true if connection is working
     */
    bool testConnection();
    
    /**
     * Clear all data from tables (for testing)
     * Note: Tables and indexes should be created externally via SQL scripts
     */
    void clearTables();
    
    /**
     * Check if required tables exist
     * @return true if both inspection_group and inspection_region tables exist
     */
    bool tablesExist();
    
    /**
     * Insert a group into inspection_group table
     * @param group_id Unique group identifier
     */
    void insertGroup(int64_t group_id);
    
    /**
     * Insert multiple groups efficiently
     * @param group_ids Vector of unique group identifiers
     */
    void insertGroups(const std::vector<int64_t>& group_ids);
    
    /**
     * Insert a point into inspection_region table
     * @param point Point data to insert
     */
    void insertPoint(const Point& point);
    
    /**
     * Insert multiple points efficiently using batch operations
     * @param points Vector of points to insert
     */
    void insertPoints(const std::vector<Point>& points);
    
    /**
     * Get database connection for advanced operations
     * @return Reference to pqxx connection
     */
    pqxx::connection& getConnection();
    
    /**
     * Execute a query and return results
     * @param query SQL query string
     * @return Query results
     */
    pqxx::result executeQuery(const std::string& query);
    
    /**
     * Get count of records in a table
     * @param table_name Name of the table
     * @return Number of records
     */
    size_t getTableCount(const std::string& table_name);
    
private:
    /**
     * Prepare commonly used SQL statements for better performance
     */
    void prepareStatements();
};