#include "QueryOperator.h"
#include <set>
#include <algorithm>
#include <iostream>

namespace PointSetUtils {

std::vector<Point> intersectPoints(const std::vector<std::vector<Point>>& pointSets) {
    if (pointSets.empty()) {
        return {};
    }
    
    if (pointSets.size() == 1) {
        return deduplicateAndSort(pointSets[0]);
    }
    
    // Start with first set
    std::set<long long> commonIds;
    for (const Point& point : pointSets[0]) {
        commonIds.insert(point.id);
    }
    
    // Intersect with each subsequent set
    for (size_t i = 1; i < pointSets.size(); ++i) {
        std::set<long long> currentIds;
        for (const Point& point : pointSets[i]) {
            currentIds.insert(point.id);
        }
        
        // Keep only IDs that are in both sets
        std::set<long long> intersection;
        std::set_intersection(
            commonIds.begin(), commonIds.end(),
            currentIds.begin(), currentIds.end(),
            std::inserter(intersection, intersection.begin())
        );
        
        commonIds = std::move(intersection);
    }
    
    // Build result vector with points that have IDs in the intersection
    std::vector<Point> result;
    for (const Point& point : pointSets[0]) {
        if (commonIds.find(point.id) != commonIds.end()) {
            result.push_back(point);
        }
    }
    
    return deduplicateAndSort(std::move(result));
}

std::vector<Point> unionPoints(const std::vector<std::vector<Point>>& pointSets) {
    std::set<long long> seenIds;
    std::vector<Point> result;
    
    for (const auto& pointSet : pointSets) {
        for (const Point& point : pointSet) {
            if (seenIds.find(point.id) == seenIds.end()) {
                seenIds.insert(point.id);
                result.push_back(point);
            }
        }
    }
    
    return deduplicateAndSort(std::move(result));
}

std::vector<Point> deduplicateAndSort(std::vector<Point> points) {
    // Remove duplicates by ID
    std::set<long long> seenIds;
    std::vector<Point> unique;
    
    for (const Point& point : points) {
        if (seenIds.find(point.id) == seenIds.end()) {
            seenIds.insert(point.id);
            unique.push_back(point);
        }
    }
    
    // Sort by (y, x) as required
    std::sort(unique.begin(), unique.end());
    
    return unique;
}

} // namespace PointSetUtils