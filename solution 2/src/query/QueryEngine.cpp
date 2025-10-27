#include "QueryEngine.h"
#include <iostream>
#include <chrono>

QueryEngine::QueryEngine(const std::string& connection_string) {
    db_manager = std::make_unique<DatabaseManager>(connection_string);
}

QueryEngine::~QueryEngine() = default;

QueryResult QueryEngine::executeQueryFile(const std::string& query_file) {
    QuerySpec query_spec = JsonParser::parseQueryFile(query_file);
    return executeQuery(query_spec);
}

QueryResult QueryEngine::executeQueryString(const std::string& json_content) {
    QuerySpec query_spec = JsonParser::parseQueryString(json_content);
    return executeQuery(query_spec);
}

QueryResult QueryEngine::executeQuery(const QuerySpec& query_spec) {
    validateQuery(query_spec);
    
    std::cout << "\n=== Executing Crop Query ===" << std::endl;
    std::cout << "Valid region: " << query_spec.valid_region.toString() << std::endl;
    std::cout << "Crop region: " << query_spec.crop_query.region.toString() << std::endl;
    
    if (!query_spec.crop_query.category_filter.empty()) {
        std::cout << "Category filter: ";
        for (size_t i = 0; i < query_spec.crop_query.category_filter.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << query_spec.crop_query.category_filter[i];
        }
        std::cout << std::endl;
    }
    
    if (!query_spec.crop_query.group_filter.empty()) {
        std::cout << "Group filter: ";
        for (size_t i = 0; i < query_spec.crop_query.group_filter.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << query_spec.crop_query.group_filter[i];
        }
        std::cout << std::endl;
    }
    
    if (query_spec.crop_query.proper_only) {
        std::cout << "Proper groups only: true" << std::endl;
    }
    
    // Record start time for query execution
    auto query_start = std::chrono::high_resolution_clock::now();
    
    // Execute the database query
    std::vector<Point> result_points = db_manager->executeCropQuery(
        query_spec.crop_query.region,
        query_spec.valid_region,
        query_spec.crop_query.category_filter,
        query_spec.crop_query.group_filter,
        query_spec.crop_query.proper_only
    );
    
    // Calculate query execution time
    auto query_end = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::milliseconds>(query_end - query_start);
    
    std::cout << "Query executed successfully in " << query_duration.count() << " ms." << std::endl;
    
    QueryResult result(result_points);
    result.setQueryDuration(query_duration.count());
    return result;
}

bool QueryEngine::testConnection() {
    return db_manager && db_manager->testConnection();
}

void QueryEngine::validateQuery(const QuerySpec& query_spec) {
    // Validate valid region
    if (!query_spec.valid_region.isValid()) {
        throw std::runtime_error("Invalid valid_region: p_min must be <= p_max in both dimensions");
    }
    
    // Validate crop region
    if (!query_spec.crop_query.region.isValid()) {
        throw std::runtime_error("Invalid crop region: p_min must be <= p_max in both dimensions");
    }
    
    // Validate that crop region is within or intersects valid region
    if (!query_spec.valid_region.intersects(query_spec.crop_query.region)) {
        std::cout << "Warning: Crop region does not intersect with valid region. Query may return no results." << std::endl;
    }
}