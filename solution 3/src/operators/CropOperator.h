#pragma once

#include "QueryOperator.h"
#include "query/QueryEngine.h"
#include "query/JsonParser.h"

/**
 * CropOperator wraps the existing QueryEngine functionality
 * Supports all Task 2 features: region, category, one_of_groups, proper
 */
class CropOperator : public QueryOperator {
private:
    Rectangle crop_region;
    std::optional<int> category;
    std::optional<std::vector<long long>> one_of_groups;
    std::optional<bool> proper;
    
    // Reuse existing QueryEngine
    std::unique_ptr<QueryEngine> query_engine;
    
public:
    CropOperator(
        const Rectangle& region,
        const std::optional<int>& cat = std::nullopt,
        const std::optional<std::vector<long long>>& groups = std::nullopt,
        const std::optional<bool>& prop = std::nullopt
    );
    
    /**
     * Execute crop operation using existing QueryEngine
     */
    std::vector<Point> execute(
        const Rectangle& valid_region,
        DatabaseManager& db_manager
    ) override;
    
    std::string getDescription() const override;
    
    // Static factory method for JSON parsing
    static std::unique_ptr<CropOperator> fromJson(const nlohmann::json& json);
};