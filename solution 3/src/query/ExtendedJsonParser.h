#pragma once

#include "operators/QueryOperator.h"
#include "operators/CropOperator.h"
#include "operators/AndOperator.h"
#include "operators/OrOperator.h"
#include <nlohmann/json.hpp>
#include <memory>

/**
 * Extended JSON parser that supports nested logical operators
 * Supports: operator_crop, operator_and, operator_or
 */
class ExtendedJsonParser {
public:
    /**
     * Parse a complete query JSON including valid_region and nested operators
     * @param json_str JSON string containing the complete query
     * @return Pair of (valid_region, root_operator)
     */
    static std::pair<Rectangle, std::unique_ptr<QueryOperator>> parseQuery(const std::string& json_str);
    
    /**
     * Parse a complete query from JSON object
     */
    static std::pair<Rectangle, std::unique_ptr<QueryOperator>> parseQuery(const nlohmann::json& json);
    
    /**
     * Parse a single operator (recursive)
     * @param operator_json JSON object containing operator definition
     * @return Unique pointer to the parsed operator
     */
    static std::unique_ptr<QueryOperator> parseOperator(const nlohmann::json& operator_json);
    
private:
    /**
     * Parse valid_region from JSON
     */
    static Rectangle parseValidRegion(const nlohmann::json& valid_region_json);
    
    /**
     * Parse operator_and from JSON
     */
    static std::unique_ptr<AndOperator> parseAndOperator(const nlohmann::json& and_json);
    
    /**
     * Parse operator_or from JSON  
     */
    static std::unique_ptr<OrOperator> parseOrOperator(const nlohmann::json& or_json);
};