#include <iostream>
#include <stdexcept>
#include <chrono>
#include <gflags/gflags.h>
#include "../database/DatabaseManager.h"
#include "../data/DataLoader.h"

// Define command line flags
DEFINE_string(data_directory, "", "Path to directory containing data files (required)");
DEFINE_string(database, "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db", 
              "PostgreSQL connection string");

/**
 * Task 1: Data Loading Program
 * 
 * This program loads inspection region data from text files into PostgreSQL.
 * It implements the requirements specified in Task 1 of the problem description.
 */

int main(int argc, char* argv[]) {
    // Set usage message
    gflags::SetUsageMessage("Inspection Region Data Loader - Task 1\n"
                           "This program loads inspection region data from text files into PostgreSQL.\n\n"
                           "The data directory must contain three synchronized text files:\n"
                           "- points.txt: x y coordinates (one point per line)\n"
                           "- categories.txt: category ID for each point (one per line)\n" 
                           "- groups.txt: group ID for each point (one per line)\n"
                           "Line i in all three files corresponds to the same region.\n\n"
                           "Examples:\n"
                           "  " + std::string(argv[0]) + " --data_directory=./data/0\n"
                           "  " + std::string(argv[0]) + " --data_directory=./data/1 --database=postgresql://user:pass@localhost:5432/mydb");
    
    // Parse command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    try {
        // Validate required arguments
        if (FLAGS_data_directory.empty()) {
            std::cerr << "Error: --data_directory argument is required" << std::endl;
            std::cerr << gflags::ProgramUsage() << std::endl;
            return 1;
        }
        
        // Get arguments from flags
        std::string data_directory = FLAGS_data_directory;
        std::string connection_string = FLAGS_database;
        
        std::cout << "Inspection Region Data Loader - Task 1" << std::endl;
        std::cout << "=======================================" << std::endl;
        std::cout << "Data directory: " << data_directory << std::endl;
        std::cout << "Database: " << connection_string << std::endl;
        std::cout << std::endl;
        
        // Record start time for performance measurement
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Initialize database connection
        std::cout << "Connecting to database..." << std::endl;
        DatabaseManager db_manager(connection_string);
        
        if (!db_manager.testConnection()) {
            throw std::runtime_error("Database connection test failed");
        }
        
        std::cout << "✓ Database connection established" << std::endl;
        
        // Initialize data loader and load data
        DataLoader loader(data_directory, db_manager);
        loader.loadData();
        
        // Calculate and display performance metrics
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << std::endl << "=== Performance Summary ===" << std::endl;
        std::cout << "Total execution time: " << duration.count() << " ms" << std::endl;
        
        // Verify final database state
        size_t total_groups = db_manager.getTableCount("inspection_group");
        size_t total_points = db_manager.getTableCount("inspection_region");
        
        std::cout << "Final database state:" << std::endl;
        std::cout << "  - Groups: " << total_groups << std::endl;
        std::cout << "  - Points: " << total_points << std::endl;
        
        std::cout << std::endl << "🎉 Task 1 completed successfully!" << std::endl;
        std::cout << "The database is now ready for spatial queries (Task 2)." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << std::endl << "❌ Error: " << e.what() << std::endl;
        std::cerr << std::endl << "Task 1 failed. Please check the error message above and try again." << std::endl;
        return 1;
    }
}