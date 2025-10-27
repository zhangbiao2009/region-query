#include "JsonParser.h"
#include <fstream>
#include <iostream>

QuerySpec JsonParser::parseQueryFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open query file: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    return parseQueryString(content);
}

QuerySpec JsonParser::parseQueryString(const std::string& json_content) {
    try {
        nlohmann::json root = nlohmann::json::parse(json_content);
        
        // Validate top-level structure
        validateRequiredFields(root, {"valid_region", "query"});
        
        // Parse valid region
        Rectangle valid_region = parseRectangle(root["valid_region"]);
        
        // Parse query (currently only operator_crop is supported)
        const auto& query_obj = root["query"];
        if (!query_obj.contains("operator_crop")) {
            throw std::runtime_error("Only 'operator_crop' query operator is supported in Task 2");
        }
        
        CropQuery crop_query = parseCropQuery(query_obj["operator_crop"]);
        
        return QuerySpec(valid_region, crop_query);
        
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }
}

Point JsonParser::parsePoint(const nlohmann::json& json_point) {
    validateRequiredFields(json_point, {"x", "y"});
    
    double x = json_point["x"].get<double>();
    double y = json_point["y"].get<double>();
    
    return Point(x, y);
}

Rectangle JsonParser::parseRectangle(const nlohmann::json& json_rect) {
    validateRequiredFields(json_rect, {"p_min", "p_max"});
    
    Point p_min = parsePoint(json_rect["p_min"]);
    Point p_max = parsePoint(json_rect["p_max"]);
    
    Rectangle rect(p_min, p_max);
    if (!rect.isValid()) {
        throw std::runtime_error("Invalid rectangle: p_min must be <= p_max in both dimensions");
    }
    
    return rect;
}

CropQuery JsonParser::parseCropQuery(const nlohmann::json& json_crop) {
    // region is required
    if (!json_crop.contains("region")) {
        throw std::runtime_error("operator_crop requires 'region' field");
    }
    
    CropQuery crop_query;
    
    // Parse required region
    crop_query.region = parseRectangle(json_crop["region"]);
    
    // Parse optional category filter
    if (json_crop.contains("category")) {
        int category = json_crop["category"].get<int>();
        crop_query.category_filter.push_back(category);
    }
    
    // Parse optional one_of_groups filter
    if (json_crop.contains("one_of_groups")) {
        const auto& groups_array = json_crop["one_of_groups"];
        if (!groups_array.is_array()) {
            throw std::runtime_error("one_of_groups must be an array");
        }
        
        for (const auto& group : groups_array) {
            crop_query.group_filter.push_back(group.get<long long>());
        }
    }
    
    // Parse optional proper flag
    if (json_crop.contains("proper")) {
        crop_query.proper_only = json_crop["proper"].get<bool>();
    }
    
    return crop_query;
}

void JsonParser::validateRequiredFields(const nlohmann::json& json_obj, const std::vector<std::string>& fields) {
    for (const std::string& field : fields) {
        if (!json_obj.contains(field)) {
            throw std::runtime_error("Missing required field: " + field);
        }
    }
}