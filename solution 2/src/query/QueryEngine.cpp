#include "QueryEngine.h"
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <algorithm>

QueryEngine::QueryEngine(const std::string& connection_string, bool test_mode) : test_mode(test_mode) {
    db_manager = std::make_unique<DatabaseManager>(connection_string);
    
    if (test_mode) {
        std::cout << "Loading all points into memory for brute force testing..." << std::endl;
        cached_points = db_manager->getAllPoints();
    }
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
    
    if (query_spec.crop_query.proper.has_value()) {
        std::cout << "Proper constraint: " << (query_spec.crop_query.proper.value() ? "true (proper groups only)" : "false (improper groups only)") << std::endl;
    }
    
    // Record start time for query execution
    auto query_start = std::chrono::high_resolution_clock::now();
    
    // Execute the database query
    std::vector<Point> result_points = db_manager->executeCropQuery(
        query_spec.crop_query.region,
        query_spec.valid_region,
        query_spec.crop_query.category_filter,
        query_spec.crop_query.group_filter,
        query_spec.crop_query.proper
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

QueryResult QueryEngine::executeQueryBruteForce(const QuerySpec& query_spec) {
    if (!test_mode) {
        throw std::runtime_error("executeQueryBruteForce can only be called in test mode");
    }
    
    validateQuery(query_spec);
    
    std::cout << "\n=== Executing Brute Force Query ===" << std::endl;
    
    auto query_start = std::chrono::high_resolution_clock::now();
    
    std::vector<Point> result_points;
    
    // Step 1: Find proper groups if proper constraint is specified
    std::set<long long> proper_groups;
    if (query_spec.crop_query.proper.has_value()) {
        // Only analyze groups when proper constraint is specified
        std::map<long long, std::vector<Point>> groups_map;
        
        // Group points by group_id
        for (const auto& point : cached_points) {
            groups_map[point.group_id].push_back(point);
        }
        
        // Check which groups are entirely within valid region
        for (const auto& [group_id, group_points] : groups_map) {
            bool all_within_valid = true;
            for (const auto& point : group_points) {
                if (!query_spec.valid_region.contains(Point(point.x, point.y))) {
                    all_within_valid = false;
                    break;
                }
            }
            if (all_within_valid) {
                proper_groups.insert(group_id);
            }
        }
        
        if (query_spec.crop_query.proper.value()) {
            std::cout << "Found " << proper_groups.size() << " proper groups within valid region" << std::endl;
        } else {
            std::cout << "Found " << (groups_map.size() - proper_groups.size()) << " improper groups within valid region" << std::endl;
        }
    }
    // When proper is nullopt, proper_groups remains empty and valid_region is ignored for group logic
    
    // Step 2: Filter points
    for (const auto& point : cached_points) {
        // Check if point is within crop region
        if (!query_spec.crop_query.region.contains(Point(point.x, point.y))) {
            continue;
        }
        
        // Check category filter
        if (!query_spec.crop_query.category_filter.empty()) {
            bool category_match = false;
            for (int cat : query_spec.crop_query.category_filter) {
                if (point.category == cat) {
                    category_match = true;
                    break;
                }
            }
            if (!category_match) continue;
        }
        
        // Check group filter
        if (!query_spec.crop_query.group_filter.empty()) {
            bool group_match = false;
            for (long long grp : query_spec.crop_query.group_filter) {
                if (point.group_id == grp) {
                    group_match = true;
                    break;
                }
            }
            if (!group_match) continue;
        }
        
        // Check proper constraint
        if (query_spec.crop_query.proper.has_value()) {
            bool is_proper_group = (proper_groups.find(point.group_id) != proper_groups.end());
            
            if (query_spec.crop_query.proper.value()) {
                // proper: true - only proper groups
                if (!is_proper_group) {
                    continue;
                }
            } else {
                // proper: false - only improper groups
                if (is_proper_group) {
                    continue;
                }
            }
        }
        
        result_points.push_back(point);
    }
    
    // Step 3: Sort results by (y, x)
    std::sort(result_points.begin(), result_points.end());
    
    auto query_end = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::milliseconds>(query_end - query_start);
    
    std::cout << "Brute force query executed in " << query_duration.count() << " ms." << std::endl;
    std::cout << "Found " << result_points.size() << " matching points" << std::endl;
    
    QueryResult result(result_points);
    result.setQueryDuration(query_duration.count());
    return result;
}

DataBounds QueryEngine::getDataBounds() const {
    if (!test_mode) {
        throw std::runtime_error("getDataBounds() is only available in test mode");
    }
    
    if (cached_points.empty()) {
        throw std::runtime_error("No data loaded - cannot determine bounds");
    }
    
    DataBounds bounds;
    bounds.total_points = cached_points.size();
    
    // Initialize with first point
    const auto& first_point = cached_points[0];
    bounds.min_x = bounds.max_x = first_point.x;
    bounds.min_y = bounds.max_y = first_point.y;
    bounds.min_category = bounds.max_category = first_point.category;
    bounds.min_group_id = bounds.max_group_id = first_point.group_id;
    
    // Find bounds across all points
    for (const auto& point : cached_points) {
        bounds.min_x = std::min(bounds.min_x, point.x);
        bounds.max_x = std::max(bounds.max_x, point.x);
        bounds.min_y = std::min(bounds.min_y, point.y);
        bounds.max_y = std::max(bounds.max_y, point.y);
        bounds.min_category = std::min(bounds.min_category, point.category);
        bounds.max_category = std::max(bounds.max_category, point.category);
        bounds.min_group_id = std::min(bounds.min_group_id, point.group_id);
        bounds.max_group_id = std::max(bounds.max_group_id, point.group_id);
    }
    
    return bounds;
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