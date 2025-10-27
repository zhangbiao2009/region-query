#include <gtest/gtest.h>
#include "src/query/QueryEngine.h"
#include "src/query/JsonParser.h"
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cmath>

class RandomQueryEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        connection_string = "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db";
        // Create QueryEngine in test mode to enable brute force comparison
        engine = std::make_unique<QueryEngine>(connection_string, true);
        
        // Get data bounds from loaded points
        data_bounds = engine->getDataBounds();
        
        // Initialize random generator with current time
        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
        std::cout << "Random seed: " << seed << std::endl;
        std::cout << "Data bounds: x[" << data_bounds.min_x << ", " << data_bounds.max_x 
                  << "], y[" << data_bounds.min_y << ", " << data_bounds.max_y 
                  << "], categories[" << data_bounds.min_category << ", " << data_bounds.max_category 
                  << "], groups[" << data_bounds.min_group_id << ", " << data_bounds.max_group_id 
                  << "], total points: " << data_bounds.total_points << std::endl;
    }

    void TearDown() override {
        engine.reset();
    }

    void compareQueryResults(const QueryResult& db_result, const QueryResult& bf_result) {
        const auto& db_points = db_result.getPoints();
        const auto& bf_points = bf_result.getPoints();
        
        // Check point count
        EXPECT_EQ(db_points.size(), bf_points.size()) 
            << "Database found " << db_points.size() << " points, "
            << "brute force found " << bf_points.size() << " points";
        
        if (db_points.size() != bf_points.size()) {
            return; // Skip detailed comparison if counts don't match
        }
        
        // Compare each point in detail
        for (size_t i = 0; i < db_points.size() && i < 100; ++i) { // Limit to first 100 for performance
            const auto& db_pt = db_points[i];
            const auto& bf_pt = bf_points[i];
            
            EXPECT_EQ(db_pt.id, bf_pt.id) << "Point " << i << " ID mismatch";
            EXPECT_DOUBLE_EQ(db_pt.x, bf_pt.x) << "Point " << i << " X coordinate mismatch";
            EXPECT_DOUBLE_EQ(db_pt.y, bf_pt.y) << "Point " << i << " Y coordinate mismatch";
            EXPECT_EQ(db_pt.group_id, bf_pt.group_id) << "Point " << i << " group_id mismatch";
            EXPECT_EQ(db_pt.category, bf_pt.category) << "Point " << i << " category mismatch";
        }
    }

    void testRandomQuery(const std::string& test_name, const std::string& query_json) {
        std::cout << "\n=== Testing: " << test_name << " ===" << std::endl;
        std::cout << "Query: " << query_json << std::endl;
        
        QuerySpec query_spec = JsonParser::parseQueryString(query_json);
        
        // Execute both queries
        auto start_time = std::chrono::high_resolution_clock::now();
        QueryResult db_result = engine->executeQuery(query_spec);
        auto db_end = std::chrono::high_resolution_clock::now();
        
        QueryResult bf_result = engine->executeQueryBruteForce(query_spec);
        auto bf_end = std::chrono::high_resolution_clock::now();
        
        auto db_duration = std::chrono::duration_cast<std::chrono::microseconds>(db_end - start_time);
        auto bf_duration = std::chrono::duration_cast<std::chrono::microseconds>(bf_end - db_end);
        
        std::cout << "Database query: " << db_duration.count() << "μs, "
                  << "Brute force: " << bf_duration.count() << "μs" << std::endl;
        std::cout << "Results: " << db_result.getPoints().size() << " points" << std::endl;
        
        // Compare results
        compareQueryResults(db_result, bf_result);
    }

    // Generate random rectangle within bounds
    Rectangle generateRandomRectangle(double min_x, double max_x, double min_y, double max_y, 
                                    double min_width = 10.0, double min_height = 10.0) {
        std::uniform_real_distribution<double> x_dist(min_x, max_x - min_width);
        std::uniform_real_distribution<double> y_dist(min_y, max_y - min_height);
        std::uniform_real_distribution<double> width_dist(min_width, (max_x - min_x) * 0.3);
        std::uniform_real_distribution<double> height_dist(min_height, (max_y - min_y) * 0.3);
        
        double x1 = x_dist(rng);
        double y1 = y_dist(rng);
        double width = std::min(width_dist(rng), max_x - x1);
        double height = std::min(height_dist(rng), max_y - y1);
        
        return Rectangle{x1, y1, x1 + width, y1 + height};
    }

    // Generate random category filter
    std::vector<int> generateRandomCategoryFilter() {
        std::uniform_int_distribution<int> should_filter(0, 1);
        if (should_filter(rng) == 0) {
            return {}; // No filter
        }
        
        int range = data_bounds.max_category - data_bounds.min_category + 1;
        std::uniform_int_distribution<int> num_cats_dist(1, std::min(3, range));
        std::uniform_int_distribution<int> cat_dist(data_bounds.min_category, data_bounds.max_category);
        
        int num_categories = num_cats_dist(rng);
        std::vector<int> categories;
        
        for (int i = 0; i < num_categories; ++i) {
            int cat = cat_dist(rng);
            if (std::find(categories.begin(), categories.end(), cat) == categories.end()) {
                categories.push_back(cat);
            }
        }
        
        return categories;
    }

    // Generate random group filter
    std::vector<long long> generateRandomGroupFilter() {
        std::uniform_int_distribution<int> should_filter(0, 2); // 1/3 chance of no filter
        if (should_filter(rng) == 0) {
            return {}; // No filter
        }
        
        long long range = data_bounds.max_group_id - data_bounds.min_group_id + 1;
        std::uniform_int_distribution<int> num_groups_dist(1, std::min(5LL, range));
        std::uniform_int_distribution<long long> group_dist(data_bounds.min_group_id, data_bounds.max_group_id);
        
        int num_groups = num_groups_dist(rng);
        std::vector<long long> groups;
        
        for (int i = 0; i < num_groups; ++i) {
            long long grp = group_dist(rng);
            if (std::find(groups.begin(), groups.end(), grp) == groups.end()) {
                groups.push_back(grp);
            }
        }
        
        return groups;
    }

    // Generate random query JSON
    std::string generateRandomQuery(double scale_factor = 1.0) {
        double coord_range_x = data_bounds.max_x - data_bounds.min_x;
        double coord_range_y = data_bounds.max_y - data_bounds.min_y;
        
        // Generate valid region (larger)
        Rectangle valid_region = generateRandomRectangle(
            data_bounds.min_x, data_bounds.min_x + coord_range_x * scale_factor * 0.8, 
            data_bounds.min_y, data_bounds.min_y + coord_range_y * scale_factor * 0.8, 
            coord_range_x * 0.05, coord_range_y * 0.05);
        
        // Generate crop region (within or overlapping valid region)
        double crop_min_x = std::max(data_bounds.min_x, valid_region.p_min.x - coord_range_x * 0.1);
        double crop_max_x = std::min(data_bounds.max_x, valid_region.p_max.x + coord_range_x * 0.1);
        double crop_min_y = std::max(data_bounds.min_y, valid_region.p_min.y - coord_range_y * 0.1);
        double crop_max_y = std::min(data_bounds.max_y, valid_region.p_max.y + coord_range_y * 0.1);
        
        Rectangle crop_region = generateRandomRectangle(crop_min_x, crop_max_x, crop_min_y, crop_max_y,
                                                      coord_range_x * 0.01, coord_range_y * 0.01);
        
        // Generate filters
        auto category_filter = generateRandomCategoryFilter();
        auto group_filter = generateRandomGroupFilter();
        
        // Random proper flag (sometimes omitted)
        std::uniform_int_distribution<int> proper_dist(0, 2); // 0=omit, 1=false, 2=true
        int proper_choice = proper_dist(rng);
        
        // Build JSON
        std::ostringstream json;
        json << "{"
             << "\"valid_region\": {"
             << "\"p_min\": {\"x\": " << valid_region.p_min.x << ", \"y\": " << valid_region.p_min.y << "}, "
             << "\"p_max\": {\"x\": " << valid_region.p_max.x << ", \"y\": " << valid_region.p_max.y << "}"
             << "}, "
             << "\"query\": {"
             << "\"operator_crop\": {"
             << "\"region\": {"
             << "\"p_min\": {\"x\": " << crop_region.p_min.x << ", \"y\": " << crop_region.p_min.y << "}, "
             << "\"p_max\": {\"x\": " << crop_region.p_max.x << ", \"y\": " << crop_region.p_max.y << "}"
             << "}";
        
        // Add category filter if present
        if (!category_filter.empty()) {
            json << ", \"category\": " << category_filter[0];
        }
        
        // Add group filter if present  
        if (!group_filter.empty()) {
            json << ", \"one_of_groups\": [";
            for (size_t i = 0; i < group_filter.size(); ++i) {
                if (i > 0) json << ", ";
                json << group_filter[i];
            }
            json << "]";
        }
        
        // Add proper flag if not omitted
        if (proper_choice != 0) {
            json << ", \"proper\": " << (proper_choice == 2 ? "true" : "false");
        }
        
        json << "}}}";
        
        return json.str();
    }

    std::string connection_string;
    std::unique_ptr<QueryEngine> engine;
    std::default_random_engine rng;
    DataBounds data_bounds;
};

// Test cases
TEST_F(RandomQueryEngineTest, RandomTest01_SmallCropRegion) {
    auto query = generateRandomQuery(0.1);
    testRandomQuery("RandomTest01_SmallCropRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest02_LargeCropRegion) {
    auto query = generateRandomQuery(1.0);
    testRandomQuery("RandomTest02_LargeCropRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest03_CategoryFilter) {
    auto query = generateRandomQuery(0.5);
    testRandomQuery("RandomTest03_CategoryFilter", query);
}

TEST_F(RandomQueryEngineTest, RandomTest04_GroupFilter) {
    auto query = generateRandomQuery(0.3);
    testRandomQuery("RandomTest04_GroupFilter", query);
}

TEST_F(RandomQueryEngineTest, RandomTest05_ProperConstraint) {
    auto query = generateRandomQuery(0.4);
    testRandomQuery("RandomTest05_ProperConstraint", query);
}

TEST_F(RandomQueryEngineTest, RandomTest06_MediumRegion) {
    auto query = generateRandomQuery(0.6);
    testRandomQuery("RandomTest06_MediumRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest07_TinyRegion) {
    auto query = generateRandomQuery(0.05);
    testRandomQuery("RandomTest07_TinyRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest08_FullRange) {
    auto query = generateRandomQuery(1.0);
    testRandomQuery("RandomTest08_FullRange", query);
}

TEST_F(RandomQueryEngineTest, RandomTest09_CornerRegion) {
    auto query = generateRandomQuery(0.2);
    testRandomQuery("RandomTest09_CornerRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest10_MultipleFilters) {
    auto query = generateRandomQuery(0.7);
    testRandomQuery("RandomTest10_MultipleFilters", query);
}

TEST_F(RandomQueryEngineTest, RandomTest11_EdgeCase) {
    auto query = generateRandomQuery(0.15);
    testRandomQuery("RandomTest11_EdgeCase", query);
}

TEST_F(RandomQueryEngineTest, RandomTest12_WideRegion) {
    auto query = generateRandomQuery(0.8);
    testRandomQuery("RandomTest12_WideRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest13_NarrowRegion) {
    auto query = generateRandomQuery(0.25);
    testRandomQuery("RandomTest13_NarrowRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest14_RandomSized) {
    auto query = generateRandomQuery(0.45);
    testRandomQuery("RandomTest14_RandomSized", query);
}

TEST_F(RandomQueryEngineTest, RandomTest15_MidRange) {
    auto query = generateRandomQuery(0.55);
    testRandomQuery("RandomTest15_MidRange", query);
}

TEST_F(RandomQueryEngineTest, RandomTest16_LargeValid) {
    auto query = generateRandomQuery(0.9);
    testRandomQuery("RandomTest16_LargeValid", query);
}

TEST_F(RandomQueryEngineTest, RandomTest17_CompactRegion) {
    auto query = generateRandomQuery(0.35);
    testRandomQuery("RandomTest17_CompactRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest18_ExtendedRegion) {
    auto query = generateRandomQuery(0.65);
    testRandomQuery("RandomTest18_ExtendedRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest19_VariableRegion) {
    auto query = generateRandomQuery(0.75);
    testRandomQuery("RandomTest19_VariableRegion", query);
}

TEST_F(RandomQueryEngineTest, RandomTest20_FinalTest) {
    auto query = generateRandomQuery(0.85);
    testRandomQuery("RandomTest20_FinalTest", query);
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "\n=== Random Query Test Suite ===" << std::endl;
    std::cout << "Data bounds will be determined from loaded dataset" << std::endl;
    std::cout << "Random seed: Current system time" << std::endl;
    std::cout << "==============================\n" << std::endl;
    
    return RUN_ALL_TESTS();
}