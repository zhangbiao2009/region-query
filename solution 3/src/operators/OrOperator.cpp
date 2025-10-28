#include "OrOperator.h"
#include <iostream>
#include <sstream>

void OrOperator::addOperand(std::unique_ptr<QueryOperator> operand) {
    operands.push_back(std::move(operand));
}

std::vector<Point> OrOperator::execute(
    const Rectangle& valid_region,
    DatabaseManager& db_manager
) {
    if (operands.empty()) {
        std::cout << "Warning: OrOperator has no operands, returning empty result" << std::endl;
        return {};
    }
    
    std::cout << "Executing OrOperator with " << operands.size() << " operands" << std::endl;
    
    // Execute all operands and collect results
    std::vector<std::vector<Point>> results;
    results.reserve(operands.size());
    
    for (size_t i = 0; i < operands.size(); ++i) {
        std::cout << "  Executing operand " << (i + 1) << ": " << operands[i]->getDescription() << std::endl;
        
        std::vector<Point> operand_result = operands[i]->execute(valid_region, db_manager);
        
        std::cout << "    Result: " << operand_result.size() << " points" << std::endl;
        
        results.push_back(std::move(operand_result));
    }
    
    // Compute union
    std::vector<Point> union_result = PointSetUtils::unionPoints(results);
    
    std::cout << "OrOperator result: " << union_result.size() << " points" << std::endl;
    
    return union_result;
}

std::string OrOperator::getDescription() const {
    std::ostringstream desc;
    desc << "OrOperator{operands=[";
    
    for (size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) desc << ", ";
        desc << operands[i]->getDescription();
    }
    
    desc << "]}";
    return desc.str();
}