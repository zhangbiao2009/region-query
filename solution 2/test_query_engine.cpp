#include <gtest/gtest.h>
#include "src/query/QueryEngine.h"
#include "src/query/JsonParser.h"
#include <vector>
#include <string>

class QueryEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        connection_string = "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db";
        // Create QueryEngine in test mode to enable brute force comparison
        engine = std::make_unique<QueryEngine>(connection_string, true);
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
        for (size_t i = 0; i < db_points.size(); ++i) {
            const auto& db_pt = db_points[i];
            const auto& bf_pt = bf_points[i];
            
            EXPECT_EQ(db_pt.id, bf_pt.id) << "Point " << i << " ID mismatch";
            EXPECT_DOUBLE_EQ(db_pt.x, bf_pt.x) << "Point " << i << " X coordinate mismatch";
            EXPECT_DOUBLE_EQ(db_pt.y, bf_pt.y) << "Point " << i << " Y coordinate mismatch";
            EXPECT_EQ(db_pt.group_id, bf_pt.group_id) << "Point " << i << " group_id mismatch";
            EXPECT_EQ(db_pt.category, bf_pt.category) << "Point " << i << " category mismatch";
        }
    }

    void testQuery(const std::string& test_name, const std::string& query_json) {
        std::cout << "\n=== Testing: " << test_name << " ===" << std::endl;
        
        QuerySpec query_spec = JsonParser::parseQueryString(query_json);
        
        // Execute both queries
        QueryResult db_result = engine->executeQuery(query_spec);
        QueryResult bf_result = engine->executeQueryBruteForce(query_spec);
        
        // Print timing information
        std::cout << "Database timing: " << db_result.getQueryDuration() << " ms" << std::endl;
        std::cout << "Brute force timing: " << bf_result.getQueryDuration() << " ms" << std::endl;
        
        // Compare results
        compareQueryResults(db_result, bf_result);
    }

    std::string connection_string;
    std::unique_ptr<QueryEngine> engine;
};

TEST_F(QueryEngineTest, SimpleCropQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 200, "y": 200},
                    "p_max": {"x": 600, "y": 600}
                }
            }
        }
    })";
    
    testQuery("Simple Crop Query", query_json);
}

TEST_F(QueryEngineTest, CategoryFilterQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 0, "y": 0},
                    "p_max": {"x": 1000, "y": 1000}
                },
                "category": 1
            }
        }
    })";
    
    testQuery("Category Filter Query", query_json);
}

TEST_F(QueryEngineTest, GroupFilterQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 0, "y": 0},
                    "p_max": {"x": 1000, "y": 1000}
                },
                "one_of_groups": [0, 1]
            }
        }
    })";
    
    testQuery("Group Filter Query", query_json);
}

TEST_F(QueryEngineTest, ProperGroupsQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 100, "y": 100},
            "p_max": {"x": 900, "y": 900}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 0, "y": 0},
                    "p_max": {"x": 1000, "y": 1000}
                },
                "proper": true
            }
        }
    })";
    
    testQuery("Proper Groups Query", query_json);
}

TEST_F(QueryEngineTest, ComplexQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 200, "y": 300},
                    "p_max": {"x": 700, "y": 800}
                },
                "category": 1,
                "one_of_groups": [0],
                "proper": true
            }
        }
    })";
    
    testQuery("Complex Query", query_json);
}

TEST_F(QueryEngineTest, SmallCropRegion) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 400, "y": 400},
                    "p_max": {"x": 450, "y": 450}
                }
            }
        }
    })";
    
    testQuery("Small Crop Region", query_json);
}

TEST_F(QueryEngineTest, EmptyResultQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 2000, "y": 2000},
                    "p_max": {"x": 3000, "y": 3000}
                }
            }
        }
    })";
    
    testQuery("Empty Result Query", query_json);
}

TEST_F(QueryEngineTest, MultipleFiltersQuery) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 0, "y": 0},
            "p_max": {"x": 1000, "y": 1000}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 100, "y": 100},
                    "p_max": {"x": 800, "y": 800}
                },
                "category": 0,
                "one_of_groups": [0, 1, 2]
            }
        }
    })";
    
    testQuery("Multiple Filters Query", query_json);
}

TEST_F(QueryEngineTest, SmallValidRegionVsLargeCrop) {
    std::string query_json = R"({
        "valid_region": {
            "p_min": {"x": 400, "y": 400},
            "p_max": {"x": 600, "y": 600}
        },
        "query": {
            "operator_crop": {
                "region": {
                    "p_min": {"x": 0, "y": 0},
                    "p_max": {"x": 1200, "y": 1200}
                }
            }
        }
    })";
    
    testQuery("Small Valid Region vs Large Crop", query_json);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}