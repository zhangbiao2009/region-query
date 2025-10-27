#pragma once

#include <memory>
#include "../database/DatabaseManager.h"
#include "../query/JsonParser.h"
#include "../query/QueryResult.h"

/**
 * Structure to hold data bounds for random query generation
 */
struct DataBounds {
    double min_x, max_x;
    double min_y, max_y;
    int min_category, max_category;
    long long min_group_id, max_group_id;
    size_t total_points;
};

/**
 * Main query execution engine for Task 2
 */
class QueryEngine {
private:
    std::unique_ptr<DatabaseManager> db_manager;
    bool test_mode;
    std::vector<Point> cached_points;  // For brute force testing
    
public:
    /**
     * Initialize query engine with database connection
     * @param connection_string PostgreSQL connection string
     * @param test_mode If true, loads all points into memory for brute force testing
     */
    explicit QueryEngine(const std::string& connection_string, bool test_mode = false);
    
    /**
     * Destructor
     */
    ~QueryEngine();
    
    /**
     * Execute a query from JSON file
     * @param query_file Path to JSON query file
     * @return QueryResult containing matching points
     * @throws std::runtime_error on query execution errors
     */
    QueryResult executeQueryFile(const std::string& query_file);
    
    /**
     * Execute a query from JSON string
     * @param json_content JSON query string
     * @return QueryResult containing matching points
     * @throws std::runtime_error on query execution errors
     */
    QueryResult executeQueryString(const std::string& json_content);
    
    /**
     * Execute a parsed query specification
     * @param query_spec Parsed query specification
     * @return QueryResult containing matching points
     * @throws std::runtime_error on query execution errors
     */
    QueryResult executeQuery(const QuerySpec& query_spec);
    
    /**
     * Execute query using brute force approach for testing
     * @param query_spec Parsed query specification
     * @return QueryResult containing matching points
     * @throws std::runtime_error if not in test mode or on execution errors
     */
    QueryResult executeQueryBruteForce(const QuerySpec& query_spec);
    
    /**
     * Test database connection
     * @return true if connection is working
     */
    bool testConnection();
    
    /**
     * Get data bounds from cached points (only available in test mode)
     * @return DataBounds structure containing min/max values for all fields
     * @throws std::runtime_error if not in test mode or no data loaded
     */
    DataBounds getDataBounds() const;
    
private:
    /**
     * Validate query specification before execution
     */
    void validateQuery(const QuerySpec& query_spec);
};