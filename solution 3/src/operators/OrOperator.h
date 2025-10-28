#pragma once

#include "QueryOperator.h"
#include <vector>
#include <memory>

/**
 * OrOperator computes the union of all operand results
 * Returns points that appear in ANY operand result (no duplicates)
 */
class OrOperator : public QueryOperator {
private:
    std::vector<std::unique_ptr<QueryOperator>> operands;
    
public:
    OrOperator() = default;
    
    /**
     * Add an operand to this OR operation
     */
    void addOperand(std::unique_ptr<QueryOperator> operand);
    
    /**
     * Execute OR operation: union of all operand results
     */
    std::vector<Point> execute(
        const Rectangle& valid_region,
        DatabaseManager& db_manager
    ) override;
    
    std::string getDescription() const override;
    
    /**
     * Get number of operands
     */
    size_t getOperandCount() const { return operands.size(); }
};