#include <iostream>
#include <stdexcept>
#include <chrono>
#include <gflags/gflags.h>
#include "../query/QueryEngine.h"

// Define command line flags
DEFINE_string(query, "", "Path to JSON query file (required)");
DEFINE_string(output, "", "Path to output file (optional, defaults to query_result.txt)");
DEFINE_string(database, "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db", 
              "PostgreSQL connection string");

/**
 * Task 2: Query Processor
 * 
 * This program executes spatial queries on inspection region data loaded in Task 1.
 * It supports the operator_crop query with various filters and the "proper" semantics.
 */

int main(int argc, char* argv[]) {
    // Set usage message
    gflags::SetUsageMessage("Inspection Region Query Processor - Task 2\n"
                           "This program executes spatial queries on inspection region data.\n"
                           "The database must be populated using the Task 1 data loader first.\n\n"
                           "Query JSON Format:\n"
                           "{\n"
                           "  \"valid_region\": { \"p_min\": {\"x\": 0, \"y\": 0}, \"p_max\": {\"x\": 1000, \"y\": 1000} },\n"
                           "  \"query\": {\n"
                           "    \"operator_crop\": {\n"
                           "      \"region\": { \"p_min\": {\"x\": 100, \"y\": 100}, \"p_max\": {\"x\": 500, \"y\": 500} },\n"
                           "      \"category\": 1,           // optional\n"
                           "      \"one_of_groups\": [0, 5], // optional\n"
                           "      \"proper\": true           // optional\n"
                           "    }\n"
                           "  }\n"
                           "}\n\n"
                           "Examples:\n"
                           "  " + std::string(argv[0]) + " --query=query1.json\n"
                           "  " + std::string(argv[0]) + " --query=query1.json --output=results.txt\n"
                           "  " + std::string(argv[0]) + " --query=query1.json --database=postgresql://user:pass@localhost:5432/mydb");
    
    // Parse command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    
    try {
        // Validate required arguments
        if (FLAGS_query.empty()) {
            std::cerr << "Error: --query argument is required" << std::endl;
            std::cerr << gflags::ProgramUsage() << std::endl;
            return 1;
        }
        
        // Get arguments from flags
        std::string query_file = FLAGS_query;
        std::string output_file = FLAGS_output.empty() ? "query_result.txt" : FLAGS_output;
        std::string connection_string = FLAGS_database;
        
        std::cout << "Inspection Region Query Processor - Task 2" << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << "Query file: " << query_file << std::endl;
        std::cout << "Output file: " << output_file << std::endl;
        std::cout << "Database: " << connection_string << std::endl;
        std::cout << std::endl;
        
        // Record start time for performance measurement
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Initialize query engine
        std::cout << "Connecting to database..." << std::endl;
        QueryEngine query_engine(connection_string);
        
        if (!query_engine.testConnection()) {
            throw std::runtime_error("Database connection test failed");
        }
        
        std::cout << "✓ Database connection established" << std::endl;
        
        // Execute query
        QueryResult result = query_engine.executeQueryFile(query_file);
        
        // Write results to output file
        result.writeToFile(output_file);
        
        // Display results and performance metrics
        result.printSummary();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << std::endl << "=== Performance Summary ===" << std::endl;
        if (result.getQueryDuration() > 0) {
            std::cout << "Database query time: " << result.getQueryDuration() << " ms" << std::endl;
        }
        std::cout << "Total execution time: " << duration.count() << " ms" << std::endl;
        std::cout << "Query result size: " << result.size() << " points" << std::endl;
        
        std::cout << std::endl << "🎉 Task 2 completed successfully!" << std::endl;
        std::cout << "Results saved to: " << output_file << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << std::endl << "❌ Error: " << e.what() << std::endl;
        std::cerr << std::endl << "Task 2 failed. Please check the error message above and try again." << std::endl;
        return 1;
    }
}