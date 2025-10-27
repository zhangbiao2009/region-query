#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../data/Point.h"
#include "../database/DatabaseManager.h"

/**
 * Handles loading data from text files into the database
 */
class DataLoader {
private:
    std::string data_directory;
    DatabaseManager& db_manager;

public:
    /**
     * Initialize data loader
     * @param data_dir Path to directory containing data files
     * @param db_mgr Reference to database manager
     */
    DataLoader(const std::string& data_dir, DatabaseManager& db_mgr);
    
    /**
     * Load all data from the directory into the database
     * This is the main entry point for Task 1
     */
    void loadData();
    
    /**
     * Validate that required files exist and are readable
     * @return true if all required files are present
     */
    bool validateFiles();
    
private:
    /**
     * Parse points.txt file
     * @return Vector of coordinate pairs
     */
    std::vector<std::pair<double, double>> parsePoints();
    
    /**
     * Parse categories.txt file
     * @return Vector of category IDs
     */
    std::vector<int> parseCategories();
    
    /**
     * Parse groups.txt file  
     * @return Vector of group IDs
     */
    std::vector<int64_t> parseGroups();
    
    /**
     * Get full path to a data file
     * @param filename Name of the file (e.g., "points.txt")
     * @return Full path to the file
     */
    std::string getFilePath(const std::string& filename);
    
    /**
     * Parse a text file with one number per line
     * @param filename Name of the file to parse
     * @return Vector of parsed numbers as strings
     */
    std::vector<std::string> parseNumberFile(const std::string& filename);
    
    /**
     * Validate that all three files have the same number of lines
     * @param points_count Number of points
     * @param categories_count Number of categories
     * @param groups_count Number of groups
     * @return true if counts match
     */
    bool validateLineCounts(size_t points_count, size_t categories_count, size_t groups_count);
    
    /**
     * Get unique group IDs from the groups vector
     * @param groups Vector of all group IDs
     * @return Vector of unique group IDs
     */
    std::vector<int64_t> getUniqueGroups(const std::vector<int64_t>& groups);
};