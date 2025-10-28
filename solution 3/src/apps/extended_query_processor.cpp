#include "query/ExtendedJsonParser.h"
#include "database/DatabaseManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <set>

/**
 * Extended Query Processor for Task 3
 * Supports logical operators: operator_and, operator_or, operator_crop
 * Maintains backward compatibility with Task 2 queries
 */
class ExtendedQueryProcessor {
private:
    std::string connection_string;
    
public:
    ExtendedQueryProcessor(const std::string& conn_str) : connection_string(conn_str) {}
    
    int run(const std::string& query_file, const std::string& output_file) {
        try {
            std::cout << "Extended Query Processor - Task 3" << std::endl;
            std::cout << "=========================================" << std::endl;
            std::cout << "Query file: " << query_file << std::endl;
            std::cout << "Output file: " << output_file << std::endl;
            std::cout << "Database: " << connection_string << std::endl;
            std::cout << std::endl;
            
            // Read query file
            std::ifstream file(query_file);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open query file: " + query_file);
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string query_json = buffer.str();
            file.close();
            
            std::cout << "Query JSON:" << std::endl;
            std::cout << query_json << std::endl;
            std::cout << std::endl;
            
            // Parse extended query
            std::cout << "Parsing extended query..." << std::endl;
            auto [valid_region, root_operator] = ExtendedJsonParser::parseQuery(query_json);
            
            std::cout << "Valid region: [(" << valid_region.p_min.x << "," << valid_region.p_min.y << ") - "
                      << "(" << valid_region.p_max.x << "," << valid_region.p_max.y << ")]" << std::endl;
            std::cout << "Root operator: " << root_operator->getDescription() << std::endl;
            std::cout << std::endl;
            
            // Connect to database
            std::cout << "Connecting to database..." << std::endl;
            DatabaseManager db_manager(connection_string);
            if (!db_manager.testConnection()) {
                throw std::runtime_error("Database connection test failed");
            }
            std::cout << "✓ Database connection established" << std::endl;
            std::cout << std::endl;
            
            // Execute query
            std::cout << "=== Executing Extended Query ===" << std::endl;
            auto start_time = std::chrono::high_resolution_clock::now();
            
            std::vector<Point> results = root_operator->execute(valid_region, db_manager);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            std::cout << "Query executed successfully in " << duration.count() << " ms." << std::endl;
            std::cout << std::endl;
            
            // Write results
            std::ofstream output(output_file);
            if (!output.is_open()) {
                throw std::runtime_error("Cannot create output file: " + output_file);
            }
            
            for (const Point& point : results) {
                output << std::fixed << std::setprecision(6) 
                       << point.x << " " << point.y << std::endl;
            }
            output.close();
            
            std::cout << "Results written to: " << output_file << " (" << results.size() << " points)" << std::endl;
            std::cout << std::endl;
            
            // Print summary
            printSummary(results, duration.count());
            
            std::cout << std::endl;
            std::cout << "🎉 Task 3 completed successfully!" << std::endl;
            std::cout << "Results saved to: " << output_file << std::endl;
            
            return 0;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    
private:
    void printSummary(const std::vector<Point>& results, long long duration_ms) {
        std::cout << "=== Query Results Summary ===" << std::endl;
        std::cout << "Total points found: " << results.size() << std::endl;
        std::cout << "Query execution time: " << duration_ms << " ms" << std::endl;
        
        if (!results.empty()) {
            // Calculate statistics
            std::set<long long> unique_groups;
            std::set<int> unique_categories;
            double min_x = results[0].x, max_x = results[0].x;
            double min_y = results[0].y, max_y = results[0].y;
            
            for (const Point& point : results) {
                unique_groups.insert(point.group_id);
                unique_categories.insert(point.category);
                min_x = std::min(min_x, point.x);
                max_x = std::max(max_x, point.x);
                min_y = std::min(min_y, point.y);
                max_y = std::max(max_y, point.y);
            }
            
            std::cout << "Unique groups: " << unique_groups.size() << std::endl;
            std::cout << "Unique categories: " << unique_categories.size() << std::endl;
            std::cout << "Bounding box: [(" << std::fixed << std::setprecision(2) 
                      << min_x << "," << min_y << ") - (" << max_x << "," << max_y << ")]" << std::endl;
        }
    }
};

void printUsage(const char* program_name) {
    std::cout << "Extended Query Processor - Task 3" << std::endl;
    std::cout << "This program executes extended spatial queries with logical operators." << std::endl;
    std::cout << "The database must be populated using the Task 1 data loader first." << std::endl;
    std::cout << std::endl;
    std::cout << "Extended Query JSON Format:" << std::endl;
    std::cout << R"({
  "valid_region": { "p_min": {"x": 0, "y": 0}, "p_max": {"x": 1000, "y": 1000} },
  "query": {
    "operator_and": [
      {
        "operator_crop": {
          "region": { "p_min": {"x": 200, "y": 200}, "p_max": {"x": 400, "y": 300} }
        }
      },
      {
        "operator_or": [
          {
            "operator_crop": {
              "region": { "p_min": {"x": 100, "y": 100}, "p_max": {"x": 250, "y": 1000} }
            }
          },
          {
            "operator_crop": {
              "region": { "p_min": {"x": 350, "y": 100}, "p_max": {"x": 500, "y": 1000} },
              "proper": true
            }
          }
        ]
      }
    ]
  }
})" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " --query=extended_query1.json" << std::endl;
    std::cout << "  " << program_name << " --query=extended_query1.json --output=results.txt" << std::endl;
    std::cout << "  " << program_name << " --query=extended_query1.json --database=postgresql://user:pass@localhost:5432/mydb" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::string query_file;
    std::string output_file = "extended_query_results.txt";
    std::string database_url = "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.substr(0, 8) == "--query=") {
            query_file = arg.substr(8);
        } else if (arg.substr(0, 9) == "--output=") {
            output_file = arg.substr(9);
        } else if (arg.substr(0, 11) == "--database=") {
            database_url = arg.substr(11);
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (query_file.empty()) {
        std::cerr << "Error: --query argument is required" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    ExtendedQueryProcessor processor(database_url);
    return processor.run(query_file, output_file);
}