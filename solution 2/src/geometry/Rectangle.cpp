#include "Rectangle.h"
#include <sstream>
#include <algorithm>

bool Rectangle::contains(const Point& point) const {
    return point.x >= p_min.x && point.x <= p_max.x &&
           point.y >= p_min.y && point.y <= p_max.y;
}

bool Rectangle::intersects(const Rectangle& other) const {
    return !(p_max.x < other.p_min.x || p_min.x > other.p_max.x ||
             p_max.y < other.p_min.y || p_min.y > other.p_max.y);
}

double Rectangle::area() const {
    if (!isValid()) return 0.0;
    return (p_max.x - p_min.x) * (p_max.y - p_min.y);
}

bool Rectangle::isValid() const {
    return p_min.x <= p_max.x && p_min.y <= p_max.y;
}

std::string Rectangle::toString() const {
    std::ostringstream oss;
    oss << "[(" << p_min.x << "," << p_min.y << ") - (" 
        << p_max.x << "," << p_max.y << ")]";
    return oss.str();
}