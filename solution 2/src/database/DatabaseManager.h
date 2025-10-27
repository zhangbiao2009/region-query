#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
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
     * @param proper_constraint Optional proper flag: true=proper groups, false=improper groups, nullopt=ignore
     * @return Vector of points matching all criteria, sorted by (y, x)
     */
    std::vector<Point> executeCropQuery(
        const Rectangle& crop_region,
        const Rectangle& valid_region,
        const std::vector<int>& category_filter = {},
        const std::vector<long long>& group_filter = {},
        const std::optional<bool>& proper_constraint = std::nullopt
    );
    
    /**
     * Get all groups that are entirely within the valid region
     * @param valid_region Rectangle defining valid bounds
     * @return Vector of group IDs that are proper (all points in valid region)
     */
    std::vector<long long> getProperGroups(const Rectangle& valid_region);
    
    /**
     * Get all groups that have at least one point outside the valid region
     * @param valid_region Rectangle defining valid bounds
     * @param proper_groups Vector of proper group IDs (optimization to avoid recomputation)
     * @return Vector of group IDs that are improper (at least one point outside valid region)
     */
    std::vector<long long> getImproperGroups(const Rectangle& valid_region, const std::vector<long long>& proper_groups);
    
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