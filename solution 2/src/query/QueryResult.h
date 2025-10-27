#pragma once

#include <vector>
#include <string>
#include "../geometry/Point.h"

/**
 * Query result container and output formatter
 */
class QueryResult {
private:
    std::vector<Point> points;
    long long query_duration_ms = 0;  // Query execution time in milliseconds
    
public:
    /**
     * Constructor with points
     */
    explicit QueryResult(const std::vector<Point>& result_points);
    
    /**
     * Default constructor
     */
    QueryResult() = default;
    
    /**
     * Get the points in the result
     */
    const std::vector<Point>& getPoints() const { return points; }
    
    /**
     * Get number of points in result
     */
    size_t size() const { return points.size(); }
    
    /**
     * Check if result is empty
     */
    bool empty() const { return points.empty(); }
    
    /**
     * Set query execution duration
     * @param duration_ms Duration in milliseconds
     */
    void setQueryDuration(long long duration_ms) { query_duration_ms = duration_ms; }
    
    /**
     * Get query execution duration
     * @return Duration in milliseconds
     */
    long long getQueryDuration() const { return query_duration_ms; }
    
    /**
     * Write results to output file
     * Format: Each line contains "x y" (space-separated coordinates)
     * Points are already sorted by (y, x)
     * 
     * @param filename Output file path
     * @throws std::runtime_error on file I/O errors
     */
    void writeToFile(const std::string& filename) const;
    
    /**
     * Get string representation of results for console output
     */
    std::string toString() const;
    
    /**
     * Print summary statistics
     */
    void printSummary() const;
};