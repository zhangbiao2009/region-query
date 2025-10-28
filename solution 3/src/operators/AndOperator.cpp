#include "AndOperator.h"
#include <iostream>
#include <sstream>

void AndOperator::addOperand(std::unique_ptr<QueryOperator> operand) {
    operands.push_back(std::move(operand));
}

std::vector<Point> AndOperator::execute(
    const Rectangle& valid_region,
    DatabaseManager& db_manager
) {
    if (operands.empty()) {
        std::cout << "Warning: AndOperator has no operands, returning empty result" << std::endl;
        return {};
    }
    
    std::cout << "Executing AndOperator with " << operands.size() << " operands" << std::endl;
    
    // Execute all operands and collect results
    std::vector<std::vector<Point>> results;
    results.reserve(operands.size());
    
    for (size_t i = 0; i < operands.size(); ++i) {
        std::cout << "  Executing operand " << (i + 1) << ": " << operands[i]->getDescription() << std::endl;
        
        std::vector<Point> operand_result = operands[i]->execute(valid_region, db_manager);
        
        std::cout << "    Result: " << operand_result.size() << " points" << std::endl;
        
        results.push_back(std::move(operand_result));
        
        // Early termination: if any operand returns empty, intersection is empty
        if (results.back().empty()) {
            std::cout << "  Early termination: operand returned empty result" << std::endl;
            return {};
        }
    }
    
    // Compute intersection
    std::vector<Point> intersection = PointSetUtils::intersectPoints(results);
    
    std::cout << "AndOperator result: " << intersection.size() << " points" << std::endl;
    
    return intersection;
}

std::string AndOperator::getDescription() const {
    std::ostringstream desc;
    desc << "AndOperator{operands=[";
    
    for (size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) desc << ", ";
        desc << operands[i]->getDescription();
    }
    
    desc << "]}";
    return desc.str();
}