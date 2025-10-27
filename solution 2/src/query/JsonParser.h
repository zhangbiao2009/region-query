#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "../geometry/Rectangle.h"

/**
 * Represents a crop query operation with all its parameters
 */
struct CropQuery {
    Rectangle region;                    // Required crop region
    std::vector<int> category_filter;    // Optional category filter
    std::vector<long long> group_filter; // Optional one_of_groups filter
    std::optional<bool> proper;          // Optional proper flag: true=proper, false=improper, nullopt=ignore
    
    CropQuery() = default;
    CropQuery(const Rectangle& r) : region(r) {}
};

/**
 * Complete query specification including valid region and query
 */
struct QuerySpec {
    Rectangle valid_region;
    CropQuery crop_query;
    
    QuerySpec() = default;
    QuerySpec(const Rectangle& valid_r, const CropQuery& crop_q)
        : valid_region(valid_r), crop_query(crop_q) {}
};

/**
 * JSON parser for query specifications
 */
class JsonParser {
public:
    /**
     * Parse a JSON query file
     * @param filename Path to JSON file
     * @return QuerySpec object
     * @throws std::runtime_error on parsing errors
     */
    static QuerySpec parseQueryFile(const std::string& filename);
    
    /**
     * Parse JSON content from string
     * @param json_content JSON string content
     * @return QuerySpec object
     * @throws std::runtime_error on parsing errors
     */
    static QuerySpec parseQueryString(const std::string& json_content);
    
private:
    /**
     * Parse a point from JSON object
     * @param json_point JSON object with "x" and "y" fields
     * @return Point object
     */
    static Point parsePoint(const nlohmann::json& json_point);
    
    /**
     * Parse a rectangle from JSON object
     * @param json_rect JSON object with "p_min" and "p_max" fields
     * @return Rectangle object
     */
    static Rectangle parseRectangle(const nlohmann::json& json_rect);
    
    /**
     * Parse crop query from JSON object
     * @param json_crop JSON object with operator_crop fields
     * @return CropQuery object
     */
    static CropQuery parseCropQuery(const nlohmann::json& json_crop);
    
    /**
     * Validate that required fields are present
     */
    static void validateRequiredFields(const nlohmann::json& json_obj, const std::vector<std::string>& fields);
};