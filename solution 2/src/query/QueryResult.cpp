#include "QueryResult.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>

QueryResult::QueryResult(const std::vector<Point>& result_points) : points(result_points) {
}

void QueryResult::writeToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open output file: " + filename);
    }
    
    // Write points in "x y" format, one per line
    // Points are already sorted by (y, x)
    for (const Point& point : points) {
        file << std::fixed << std::setprecision(6) 
             << point.x << " " << point.y << std::endl;
    }
    
    file.close();
    
    std::cout << "Results written to: " << filename << " (" << points.size() << " points)" << std::endl;
}

std::string QueryResult::toString() const {
    std::ostringstream oss;
    
    oss << "Query Results (" << points.size() << " points):\n";
    
    if (points.empty()) {
        oss << "  (no points found)\n";
        return oss.str();
    }
    
    // Show first few points for preview
    size_t preview_count = std::min(size_t(10), points.size());
    
    for (size_t i = 0; i < preview_count; ++i) {
        const Point& p = points[i];
        oss << "  " << std::fixed << std::setprecision(2) 
            << p.x << " " << p.y 
            << " (id=" << p.id << ", group=" << p.group_id << ", cat=" << p.category << ")\n";
    }
    
    if (points.size() > preview_count) {
        oss << "  ... and " << (points.size() - preview_count) << " more points\n";
    }
    
    return oss.str();
}

void QueryResult::printSummary() const {
    std::cout << "\n=== Query Results Summary ===" << std::endl;
    std::cout << "Total points found: " << points.size() << std::endl;
    if (query_duration_ms > 0) {
        std::cout << "Query execution time: " << query_duration_ms << " ms" << std::endl;
    }
    
    if (points.empty()) {
        std::cout << "No points matched the query criteria." << std::endl;
        return;
    }
    
    // Calculate some basic statistics
    std::set<long long> unique_groups;
    std::set<int> unique_categories;
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    
    for (const Point& p : points) {
        unique_groups.insert(p.group_id);
        unique_categories.insert(p.category);
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }
    
    std::cout << "Unique groups: " << unique_groups.size() << std::endl;
    std::cout << "Unique categories: " << unique_categories.size() << std::endl;
    std::cout << "Bounding box: [(" << std::fixed << std::setprecision(2) 
              << min_x << "," << min_y << ") - (" << max_x << "," << max_y << ")]" << std::endl;
}