#pragma once

#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "../geometry/Point.h"
#include "../geometry/Rectangle.h"

/**
 * Database manager for spatial queries on inspection regions
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
     * @return true if connection is successful
     */
    bool testConnection();
    
    /**
     * Execute a spatial crop query with all filters
     * @param crop_region Rectangle to crop points from
     * @param valid_region Rectangle defining valid bounds for proper groups
     * @param category_filter Optional category ID filter (empty if no filter)
     * @param group_filter Optional list of group IDs to include (empty if no filter)
     * @param proper_only If true, only include points from groups entirely within valid_region
     * @return Vector of points matching all criteria, sorted by (y, x)
     */
    std::vector<Point> executeCropQuery(
        const Rectangle& crop_region,
        const Rectangle& valid_region,
        const std::vector<int>& category_filter = {},
        const std::vector<long long>& group_filter = {},
        bool proper_only = false
    );
    
    /**
     * Get all groups that are entirely within the valid region
     * @param valid_region Rectangle defining valid bounds
     * @return Vector of group IDs that have all points within valid_region
     */
    std::vector<long long> getProperGroups(const Rectangle& valid_region);
    
    /**
     * Get count of records in a table for validation
     * @param table_name Name of the table
     * @return Number of records
     */
    size_t getTableCount(const std::string& table_name);
    
    /**
     * Load all points from database for testing purposes
     * @return Vector of all points in the database
     */
    std::vector<Point> getAllPoints();
    
private:
    /**
     * Build SQL query for crop operation
     */
    std::string buildCropQuery(
        const Rectangle& crop_region,
        const std::vector<int>& category_filter,
        const std::vector<long long>& group_filter,
        const std::vector<long long>& proper_groups = {}
    );
    
    /**
     * Convert pqxx result row to Point object
     */
    Point resultToPoint(const pqxx::row& row);
};