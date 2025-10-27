#pragma once

#include "Point.h"
#include <string>

/**
 * Axis-aligned rectangle for spatial queries
 */
class Rectangle {
public:
    Point p_min;  // Bottom-left corner
    Point p_max;  // Top-right corner
    
    Rectangle() = default;
    Rectangle(const Point& min_pt, const Point& max_pt) : p_min(min_pt), p_max(max_pt) {}
    Rectangle(double min_x, double min_y, double max_x, double max_y)
        : p_min(min_x, min_y), p_max(max_x, max_y) {}
    
    /**
     * Check if a point is inside this rectangle (inclusive bounds)
     * @param point Point to test
     * @return true if point is inside or on the boundary
     */
    bool contains(const Point& point) const;
    
    /**
     * Check if this rectangle intersects with another rectangle
     * @param other Other rectangle to test
     * @return true if rectangles intersect or touch
     */
    bool intersects(const Rectangle& other) const;
    
    /**
     * Get the area of this rectangle
     * @return Area (width * height)
     */
    double area() const;
    
    /**
     * Check if this rectangle is valid (min <= max for both dimensions)
     * @return true if rectangle is valid
     */
    bool isValid() const;
    
    /**
     * Get string representation for debugging
     */
    std::string toString() const;
};