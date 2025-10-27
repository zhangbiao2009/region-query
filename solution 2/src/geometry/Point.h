#pragma once

/**
 * 2D Point representation for spatial queries
 */
struct Point {
    double x;
    double y;
    long long id;
    long long group_id;
    int category;
    
    Point() : x(0.0), y(0.0), id(0), group_id(0), category(0) {}
    Point(double x_, double y_) : x(x_), y(y_), id(0), group_id(0), category(0) {}
    Point(double x_, double y_, long long id_, long long group_id_, int category_) 
        : x(x_), y(y_), id(id_), group_id(group_id_), category(category_) {}
    
    /**
     * Compare points for sorting by (y, x)
     */
    bool operator<(const Point& other) const {
        if (y != other.y) return y < other.y;
        return x < other.x;
    }
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};