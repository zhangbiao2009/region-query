#pragma once

#include "geometry/Point.h"
#include "geometry/Rectangle.h"
#include "database/DatabaseManager.h"
#include <vector>
#include <memory>

/**
 * Abstract base class for all query operators
 * Supports the composite pattern for nested operations
 */
class QueryOperator {
public:
    virtual ~QueryOperator() = default;
    
    /**
     * Execute the operator and return matching points
     * @param valid_region The valid region from the top-level query
     * @param db_manager Database manager for executing queries
     * @return Vector of points matching the operator criteria, sorted by (y,x)
     */
    virtual std::vector<Point> execute(
        const Rectangle& valid_region,
        DatabaseManager& db_manager
    ) = 0;
    
    /**
     * Get a human-readable description of this operator
     */
    virtual std::string getDescription() const = 0;
};

/**
 * Utility functions for set operations on Point vectors
 */
namespace PointSetUtils {
    /**
     * Compute intersection of multiple point sets
     * Returns points that appear in ALL sets
     */
    std::vector<Point> intersectPoints(const std::vector<std::vector<Point>>& pointSets);
    
    /**
     * Compute union of multiple point sets  
     * Returns points that appear in ANY set (no duplicates)
     */
    std::vector<Point> unionPoints(const std::vector<std::vector<Point>>& pointSets);
    
    /**
     * Remove duplicate points from a vector and sort by (y,x)
     */
    std::vector<Point> deduplicateAndSort(std::vector<Point> points);
}