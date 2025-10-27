#include "DataLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <unordered_set>
#include <iomanip>

DataLoader::DataLoader(const std::string& data_dir, DatabaseManager& db_mgr)
    : data_directory(data_dir), db_manager(db_mgr) {
}

void DataLoader::loadData() {
    std::cout << "=== Task 1: Data Loading ===" << std::endl;
    std::cout << "Data directory: " << data_directory << std::endl;
    
    // Step 1: Validate files exist
    if (!validateFiles()) {
        throw std::runtime_error("File validation failed");
    }
    
    std::cout << "✓ File validation passed" << std::endl;
    
    // Step 2: Parse all data files
    std::cout << "Parsing data files..." << std::endl;
    
    auto points = parsePoints();
    auto categories = parseCategories();  
    auto groups = parseGroups();
    
    std::cout << "✓ Parsed " << points.size() << " points" << std::endl;
    std::cout << "✓ Parsed " << categories.size() << " categories" << std::endl;
    std::cout << "✓ Parsed " << groups.size() << " group assignments" << std::endl;
    
    // Step 3: Validate data consistency
    if (!validateLineCounts(points.size(), categories.size(), groups.size())) {
        throw std::runtime_error("Data files have mismatched line counts");
    }
    
    std::cout << "✓ Data consistency validation passed" << std::endl;
    
    // Step 4: Verify database schema and clear existing data
    if (!db_manager.tablesExist()) {
        throw std::runtime_error("Required database tables do not exist. "
                                "Please run schema setup: docker-compose exec postgres psql -U inspection_user -d inspection_db -f /schema.sql");
    }
    
    std::cout << "✓ Database schema validated" << std::endl;
    db_manager.clearTables();  // Clear any existing data
    
    // Step 5: Insert unique groups first (due to foreign key constraint)
    auto unique_groups = getUniqueGroups(groups);
    std::cout << "Found " << unique_groups.size() << " unique groups" << std::endl;
    
    db_manager.insertGroups(unique_groups);
    
    // Step 6: Prepare points data
    std::vector<Point> point_data;
    point_data.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        Point point;
        point.id = static_cast<int64_t>(i + 1);  // 1-based IDs
        point.group_id = groups[i];
        point.coord_x = points[i].first;
        point.coord_y = points[i].second;
        point.category = categories[i];
        
        point_data.push_back(point);
    }
    
    // Step 7: Insert all points
    db_manager.insertPoints(point_data);
    
    // Step 8: Verify data was loaded correctly
    size_t groups_count = db_manager.getTableCount("inspection_group");
    size_t points_count = db_manager.getTableCount("inspection_region");
    
    std::cout << std::endl << "=== Data Loading Summary ===" << std::endl;
    std::cout << "Groups in database: " << groups_count << std::endl;
    std::cout << "Points in database: " << points_count << std::endl;
    
    if (points_count == points.size() && groups_count == unique_groups.size()) {
        std::cout << "✅ Data loading completed successfully!" << std::endl;
    } else {
        throw std::runtime_error("Data loading verification failed");
    }
}

bool DataLoader::validateFiles() {
    std::vector<std::string> required_files = {"points.txt", "categories.txt", "groups.txt"};
    
    for (const auto& filename : required_files) {
        std::string filepath = getFilePath(filename);
        
        if (!std::filesystem::exists(filepath)) {
            std::cerr << "❌ File not found: " << filepath << std::endl;
            return false;
        }
        
        if (!std::filesystem::is_regular_file(filepath)) {
            std::cerr << "❌ Not a regular file: " << filepath << std::endl;
            return false;
        }
        
        // Check if file is readable
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "❌ Cannot read file: " << filepath << std::endl;
            return false;
        }
        file.close();
    }
    
    return true;
}

std::vector<std::pair<double, double>> DataLoader::parsePoints() {
    std::string filepath = getFilePath("points.txt");
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open points.txt");
    }
    
    std::vector<std::pair<double, double>> points;
    std::string line;
    size_t line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        if (line.empty()) continue;  // Skip empty lines
        
        std::istringstream iss(line);
        double x, y;
        
        if (!(iss >> x >> y)) {
            throw std::runtime_error("Invalid point format at line " + std::to_string(line_number) + 
                                   " in points.txt: " + line);
        }
        
        // Check for extra data on the line
        std::string extra;
        if (iss >> extra) {
            throw std::runtime_error("Extra data found at line " + std::to_string(line_number) + 
                                   " in points.txt: " + line);
        }
        
        points.emplace_back(x, y);
    }
    
    if (points.empty()) {
        throw std::runtime_error("points.txt is empty or contains no valid data");
    }
    
    return points;
}

std::vector<int> DataLoader::parseCategories() {
    auto raw_data = parseNumberFile("categories.txt");
    std::vector<int> categories;
    categories.reserve(raw_data.size());
    
    for (size_t i = 0; i < raw_data.size(); ++i) {
        try {
            double value = std::stod(raw_data[i]);
            int category = static_cast<int>(value);
            
            // Validate that it's actually an integer value
            if (std::abs(value - category) > 1e-9) {
                throw std::runtime_error("Category at line " + std::to_string(i + 1) + 
                                       " is not an integer: " + raw_data[i]);
            }
            
            if (category < 0) {
                throw std::runtime_error("Category at line " + std::to_string(i + 1) + 
                                       " is negative: " + std::to_string(category));
            }
            
            categories.push_back(category);
            
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid category format at line " + std::to_string(i + 1) + 
                                   " in categories.txt: " + raw_data[i]);
        }
    }
    
    return categories;
}

std::vector<int64_t> DataLoader::parseGroups() {
    auto raw_data = parseNumberFile("groups.txt");
    std::vector<int64_t> groups;
    groups.reserve(raw_data.size());
    
    for (size_t i = 0; i < raw_data.size(); ++i) {
        try {
            double value = std::stod(raw_data[i]);
            int64_t group_id = static_cast<int64_t>(value);
            
            // Validate that it's actually an integer value
            if (std::abs(value - group_id) > 1e-9) {
                throw std::runtime_error("Group ID at line " + std::to_string(i + 1) + 
                                       " is not an integer: " + raw_data[i]);
            }
            
            if (group_id < 0) {
                throw std::runtime_error("Group ID at line " + std::to_string(i + 1) + 
                                       " is negative: " + std::to_string(group_id));
            }
            
            groups.push_back(group_id);
            
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid group format at line " + std::to_string(i + 1) + 
                                   " in groups.txt: " + raw_data[i]);
        }
    }
    
    return groups;
}

std::string DataLoader::getFilePath(const std::string& filename) {
    return std::filesystem::path(data_directory) / filename;
}

std::vector<std::string> DataLoader::parseNumberFile(const std::string& filename) {
    std::string filepath = getFilePath(filename);
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open " + filename);
    }
    
    std::vector<std::string> numbers;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;  // Skip empty lines
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (!line.empty()) {
            numbers.push_back(line);
        }
    }
    
    if (numbers.empty()) {
        throw std::runtime_error(filename + " is empty or contains no valid data");
    }
    
    return numbers;
}

bool DataLoader::validateLineCounts(size_t points_count, size_t categories_count, size_t groups_count) {
    if (points_count != categories_count || points_count != groups_count) {
        std::cerr << "❌ Line count mismatch:" << std::endl;
        std::cerr << "  Points: " << points_count << std::endl;
        std::cerr << "  Categories: " << categories_count << std::endl;
        std::cerr << "  Groups: " << groups_count << std::endl;
        return false;
    }
    
    return true;
}

std::vector<int64_t> DataLoader::getUniqueGroups(const std::vector<int64_t>& groups) {
    std::unordered_set<int64_t> unique_set;
    
    for (int64_t group_id : groups) {
        unique_set.insert(group_id);
    }
    
    std::vector<int64_t> unique_groups(unique_set.begin(), unique_set.end());
    std::sort(unique_groups.begin(), unique_groups.end());
    
    return unique_groups;
}