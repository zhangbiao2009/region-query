#pragma once

#include "QueryOperator.h"
#include <vector>
#include <memory>

/**
 * AndOperator computes the intersection of all operand results
 * Returns points that appear in ALL operand results
 */
class AndOperator : public QueryOperator {
private:
    std::vector<std::unique_ptr<QueryOperator>> operands;
    
public:
    AndOperator() = default;
    
    /**
     * Add an operand to this AND operation
     */
    void addOperand(std::unique_ptr<QueryOperator> operand);
    
    /**
     * Execute AND operation: intersection of all operand results
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