#include "ExtendedJsonParser.h"
#include <iostream>
#include <stdexcept>

std::pair<Rectangle, std::unique_ptr<QueryOperator>> ExtendedJsonParser::parseQuery(const std::string& json_str) {
    try {
        nlohmann::json json = nlohmann::json::parse(json_str);
        return parseQuery(json);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }
}

std::pair<Rectangle, std::unique_ptr<QueryOperator>> ExtendedJsonParser::parseQuery(const nlohmann::json& json) {
    // Parse valid_region
    if (!json.contains("valid_region")) {
        throw std::runtime_error("Query must contain 'valid_region'");
    }
    Rectangle valid_region = parseValidRegion(json["valid_region"]);
    
    // Parse query operator
    if (!json.contains("query")) {
        throw std::runtime_error("Query must contain 'query'");
    }
    auto root_operator = parseOperator(json["query"]);
    
    return std::make_pair(valid_region, std::move(root_operator));
}

std::unique_ptr<QueryOperator> ExtendedJsonParser::parseOperator(const nlohmann::json& operator_json) {
    // Check which operator type this is
    if (operator_json.contains("operator_crop")) {
        std::cout << "Parsing operator_crop" << std::endl;
        return CropOperator::fromJson(operator_json["operator_crop"]);
    }
    else if (operator_json.contains("operator_and")) {
        std::cout << "Parsing operator_and" << std::endl;
        return parseAndOperator(operator_json["operator_and"]);
    }
    else if (operator_json.contains("operator_or")) {
        std::cout << "Parsing operator_or" << std::endl;
        return parseOrOperator(operator_json["operator_or"]);
    }
    else {
        throw std::runtime_error("Unknown operator type in JSON");
    }
}

Rectangle ExtendedJsonParser::parseValidRegion(const nlohmann::json& valid_region_json) {
    if (!valid_region_json.contains("p_min") || !valid_region_json.contains("p_max")) {
        throw std::runtime_error("valid_region must contain 'p_min' and 'p_max'");
    }
    
    const auto& p_min = valid_region_json["p_min"];
    const auto& p_max = valid_region_json["p_max"];
    
    if (!p_min.contains("x") || !p_min.contains("y") ||
        !p_max.contains("x") || !p_max.contains("y")) {
        throw std::runtime_error("p_min and p_max must contain 'x' and 'y'");
    }
    
    return Rectangle(
        p_min["x"].get<double>(),
        p_min["y"].get<double>(),
        p_max["x"].get<double>(),
        p_max["y"].get<double>()
    );
}

std::unique_ptr<AndOperator> ExtendedJsonParser::parseAndOperator(const nlohmann::json& and_json) {
    if (!and_json.is_array()) {
        throw std::runtime_error("operator_and must be an array of operands");
    }
    
    auto and_op = std::make_unique<AndOperator>();
    
    for (const auto& operand_json : and_json) {
        auto operand = parseOperator(operand_json);  // Recursive call!
        and_op->addOperand(std::move(operand));
    }
    
    if (and_op->getOperandCount() == 0) {
        throw std::runtime_error("operator_and must have at least one operand");
    }
    
    std::cout << "Parsed AndOperator with " << and_op->getOperandCount() << " operands" << std::endl;
    
    return and_op;
}

std::unique_ptr<OrOperator> ExtendedJsonParser::parseOrOperator(const nlohmann::json& or_json) {
    if (!or_json.is_array()) {
        throw std::runtime_error("operator_or must be an array of operands");
    }
    
    auto or_op = std::make_unique<OrOperator>();
    
    for (const auto& operand_json : or_json) {
        auto operand = parseOperator(operand_json);  // Recursive call!
        or_op->addOperand(std::move(operand));
    }
    
    if (or_op->getOperandCount() == 0) {
        throw std::runtime_error("operator_or must have at least one operand");
    }
    
    std::cout << "Parsed OrOperator with " << or_op->getOperandCount() << " operands" << std::endl;
    
    return or_op;
}