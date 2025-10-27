#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

DatabaseManager::DatabaseManager(const std::string& conn_str) : connection_string(conn_str) {
    try {
        connection = std::make_unique<pqxx::connection>(connection_string);
        std::cout << "Connected to database: " << connection->dbname() << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Database connection failed: " + std::string(e.what()));
    }
}

DatabaseManager::~DatabaseManager() {
    if (connection && connection->is_open()) {
        connection->close();
        std::cout << "Database connection closed." << std::endl;
    }
}

bool DatabaseManager::testConnection() {
    try {
        if (!connection || !connection->is_open()) {
            return false;
        }
        
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec("SELECT 1");
        txn.commit();
        
        return result.size() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Point> DatabaseManager::executeCropQuery(
    const Rectangle& crop_region,
    const Rectangle& valid_region,
    const std::vector<int>& category_filter,
    const std::vector<long long>& group_filter,
    bool proper_only
) {
    std::vector<long long> proper_groups;
    
    // Get proper groups if needed
    if (proper_only) {
        proper_groups = getProperGroups(valid_region);
        if (proper_groups.empty()) {
            // No proper groups found, return empty result
            return {};
        }
    }
    
    // Build and execute query
    std::string query = buildCropQuery(crop_region, category_filter, group_filter, proper_groups);
    
    try {
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec(query);
        txn.commit();
        
        std::vector<Point> points;
        points.reserve(result.size());
        
        for (const auto& row : result) {
            points.push_back(resultToPoint(row));
        }
        
        // Sort by (y, x) as required
        std::sort(points.begin(), points.end());
        
        return points;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Query execution failed: " + std::string(e.what()));
    }
}

std::vector<long long> DatabaseManager::getProperGroups(const Rectangle& valid_region) {
    try {
        pqxx::work txn(*connection);
        
        // Find groups where ALL points are within valid_region
        std::string query = R"(
            SELECT group_id 
            FROM inspection_region 
            GROUP BY group_id 
            HAVING MIN(coord_x) >= $1 AND MAX(coord_x) <= $2 
               AND MIN(coord_y) >= $3 AND MAX(coord_y) <= $4
        )";
        
        pqxx::result result = txn.exec_params(query,
            valid_region.p_min.x, valid_region.p_max.x,
            valid_region.p_min.y, valid_region.p_max.y);
        txn.commit();
        
        std::vector<long long> proper_groups;
        proper_groups.reserve(result.size());
        
        for (const auto& row : result) {
            proper_groups.push_back(row[0].as<long long>());
        }
        
        return proper_groups;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Proper groups query failed: " + std::string(e.what()));
    }
}

size_t DatabaseManager::getTableCount(const std::string& table_name) {
    try {
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec("SELECT COUNT(*) FROM " + table_name);
        txn.commit();
        
        return result[0][0].as<size_t>();
    } catch (const std::exception& e) {
        throw std::runtime_error("Table count query failed: " + std::string(e.what()));
    }
}

std::string DatabaseManager::buildCropQuery(
    const Rectangle& crop_region,
    const std::vector<int>& category_filter,
    const std::vector<long long>& group_filter,
    const std::vector<long long>& proper_groups
) {
    std::ostringstream query;
    
    query << "SELECT id, coord_x, coord_y, group_id, category "
          << "FROM inspection_region WHERE ";
    
    std::vector<std::string> conditions;
    
    // Crop region condition
    conditions.push_back("coord_x >= " + std::to_string(crop_region.p_min.x));
    conditions.push_back("coord_x <= " + std::to_string(crop_region.p_max.x));
    conditions.push_back("coord_y >= " + std::to_string(crop_region.p_min.y));
    conditions.push_back("coord_y <= " + std::to_string(crop_region.p_max.y));
    
    // Category filter
    if (!category_filter.empty()) {
        std::ostringstream cat_condition;
        cat_condition << "category IN (";
        for (size_t i = 0; i < category_filter.size(); ++i) {
            if (i > 0) cat_condition << ", ";
            cat_condition << category_filter[i];
        }
        cat_condition << ")";
        conditions.push_back(cat_condition.str());
    }
    
    // Group filter (one_of_groups)
    if (!group_filter.empty()) {
        std::ostringstream group_condition;
        group_condition << "group_id IN (";
        for (size_t i = 0; i < group_filter.size(); ++i) {
            if (i > 0) group_condition << ", ";
            group_condition << group_filter[i];
        }
        group_condition << ")";
        conditions.push_back(group_condition.str());
    }
    
    // Proper groups filter
    if (!proper_groups.empty()) {
        std::ostringstream proper_condition;
        proper_condition << "group_id IN (";
        for (size_t i = 0; i < proper_groups.size(); ++i) {
            if (i > 0) proper_condition << ", ";
            proper_condition << proper_groups[i];
        }
        proper_condition << ")";
        conditions.push_back(proper_condition.str());
    }
    
    // Combine conditions
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) query << " AND ";
        query << "(" << conditions[i] << ")";
    }
    
    // Order by (y, x)
    query << " ORDER BY coord_y, coord_x";
    
    return query.str();
}

Point DatabaseManager::resultToPoint(const pqxx::row& row) {
    return Point(
        row["coord_x"].as<double>(),
        row["coord_y"].as<double>(),
        row["id"].as<long long>(),
        row["group_id"].as<long long>(),
        row["category"].as<int>()
    );
}