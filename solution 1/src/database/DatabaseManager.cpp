#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

DatabaseManager::DatabaseManager(const std::string& conn_str) 
    : connection_string(conn_str) {
    try {
        connection = std::make_unique<pqxx::connection>(connection_string);
        
        if (!connection->is_open()) {
            throw std::runtime_error("Failed to open database connection");
        }
        
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
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec("SELECT 1");
        txn.commit();
        return result.size() == 1;
    } catch (const std::exception& e) {
        std::cerr << "Connection test failed: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::tablesExist() {
    try {
        pqxx::work txn(*connection);
        
        // Check if both required tables exist
        pqxx::result result = txn.exec(
            "SELECT COUNT(*) FROM information_schema.tables "
            "WHERE table_name IN ('inspection_group', 'inspection_region') "
            "AND table_schema = 'public'"
        );
        
        txn.commit();
        
        size_t table_count = result[0][0].as<size_t>();
        return table_count == 2;
        
    } catch (const std::exception& e) {
        std::cerr << "Error checking table existence: " << e.what() << std::endl;
        return false;
    }
}

void DatabaseManager::clearTables() {
    try {
        pqxx::work txn(*connection);
        
        std::cout << "Clearing existing data..." << std::endl;
        
        // Clear in correct order due to foreign key constraints
        txn.exec("DELETE FROM inspection_region");
        txn.exec("DELETE FROM inspection_group");
        
        txn.commit();
        std::cout << "Tables cleared successfully." << std::endl;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to clear tables: " + std::string(e.what()));
    }
}

void DatabaseManager::insertGroup(int64_t group_id) {
    try {
        pqxx::work txn(*connection);
        
        txn.exec_params("INSERT INTO inspection_group (id) VALUES ($1) ON CONFLICT (id) DO NOTHING",
                       group_id);
        
        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to insert group " + std::to_string(group_id) + 
                                ": " + std::string(e.what()));
    }
}

void DatabaseManager::insertGroups(const std::vector<int64_t>& group_ids) {
    if (group_ids.empty()) return;
    
    try {
        pqxx::work txn(*connection);
        
        std::cout << "Inserting " << group_ids.size() << " unique groups..." << std::endl;
        
        // Build batch INSERT statement
        std::stringstream query;
        query << "INSERT INTO inspection_group (id) VALUES ";
        
        for (size_t i = 0; i < group_ids.size(); ++i) {
            if (i > 0) query << ", ";
            query << "(" << group_ids[i] << ")";
        }
        
        query << " ON CONFLICT (id) DO NOTHING";
        
        txn.exec(query.str());
        txn.commit();
        
        std::cout << "Groups inserted successfully." << std::endl;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to insert groups: " + std::string(e.what()));
    }
}

void DatabaseManager::insertPoint(const Point& point) {
    try {
        pqxx::work txn(*connection);
        
        txn.exec_params("INSERT INTO inspection_region (id, group_id, coord_x, coord_y, category) "
                       "VALUES ($1, $2, $3, $4, $5)",
                       point.id, point.group_id, point.coord_x, point.coord_y, point.category);
        
        txn.commit();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to insert point " + std::to_string(point.id) + 
                                ": " + std::string(e.what()));
    }
}

void DatabaseManager::insertPoints(const std::vector<Point>& points) {
    if (points.empty()) return;
    
    try {
        pqxx::work txn(*connection);
        
        std::cout << "Inserting " << points.size() << " points..." << std::endl;
        
        // Use batch INSERT for better performance
        std::stringstream query;
        query << "INSERT INTO inspection_region (id, group_id, coord_x, coord_y, category) VALUES ";
        
        for (size_t i = 0; i < points.size(); ++i) {
            if (i > 0) query << ", ";
            query << "(" << points[i].id << ", " 
                  << points[i].group_id << ", " 
                  << points[i].coord_x << ", " 
                  << points[i].coord_y << ", " 
                  << points[i].category << ")";
        }
        
        txn.exec(query.str());
        txn.commit();
        
        std::cout << "Points inserted successfully." << std::endl;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to insert points: " + std::string(e.what()));
    }
}

pqxx::connection& DatabaseManager::getConnection() {
    return *connection;
}

pqxx::result DatabaseManager::executeQuery(const std::string& query) {
    try {
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec(query);
        txn.commit();
        return result;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Query execution failed: " + std::string(e.what()));
    }
}

size_t DatabaseManager::getTableCount(const std::string& table_name) {
    try {
        pqxx::work txn(*connection);
        pqxx::result result = txn.exec("SELECT COUNT(*) FROM " + table_name);
        txn.commit();
        
        return result[0][0].as<size_t>();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get table count: " + std::string(e.what()));
    }
}