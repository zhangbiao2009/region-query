#pragma once

#include <cstdint>

/**
 * Represents a 2D inspection region point with associated metadata
 */
struct Point {
    int64_t id;
    int64_t group_id;
    double coord_x;
    double coord_y;
    int category;
    
    Point() = default;
    
    Point(int64_t id, int64_t group_id, double x, double y, int category)
        : id(id), group_id(group_id), coord_x(x), coord_y(y), category(category) {}
    
    // Comparison operators for sorting and testing
    bool operator==(const Point& other) const {
        return id == other.id;
    }
    
    bool operator<(const Point& other) const {
        if (coord_y != other.coord_y) return coord_y < other.coord_y;
        return coord_x < other.coord_x;
    }
};