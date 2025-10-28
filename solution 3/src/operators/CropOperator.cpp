#include "CropOperator.h"
#include <sstream>

CropOperator::CropOperator(
    const Rectangle& region,
    const std::optional<int>& cat,
    const std::optional<std::vector<long long>>& groups,
    const std::optional<bool>& prop
) : crop_region(region), category(cat), one_of_groups(groups), proper(prop) {
    // QueryEngine will be created when we execute
}

std::vector<Point> CropOperator::execute(
    const Rectangle& valid_region,
    DatabaseManager& db_manager
) {
    // Create QuerySpec using existing structures
    CropQuery crop_query(crop_region);
    
    if (category.has_value()) {
        crop_query.category_filter = {category.value()};
    }
    
    if (one_of_groups.has_value()) {
        crop_query.group_filter = one_of_groups.value();
    }
    
    crop_query.proper = proper;
    
    QuerySpec spec(valid_region, crop_query);
    
    // Use the existing DatabaseManager directly instead of creating a new QueryEngine
    std::vector<int> category_filter;
    if (category.has_value()) {
        category_filter.push_back(category.value());
    }
    
    std::vector<long long> group_filter;
    if (one_of_groups.has_value()) {
        group_filter = one_of_groups.value();
    }
    
    return db_manager.executeCropQuery(
        crop_region,
        valid_region,
        category_filter,
        group_filter,
        proper
    );
}

std::string CropOperator::getDescription() const {
    std::ostringstream desc;
    desc << "CropOperator{"
         << "region=[(" << crop_region.p_min.x << "," << crop_region.p_min.y << ")-"
         << "(" << crop_region.p_max.x << "," << crop_region.p_max.y << ")]";
    
    if (category.has_value()) {
        desc << ", category=" << category.value();
    }
    
    if (one_of_groups.has_value()) {
        desc << ", groups=[";
        for (size_t i = 0; i < one_of_groups->size(); ++i) {
            if (i > 0) desc << ",";
            desc << (*one_of_groups)[i];
        }
        desc << "]";
    }
    
    if (proper.has_value()) {
        desc << ", proper=" << (proper.value() ? "true" : "false");
    }
    
    desc << "}";
    return desc.str();
}

std::unique_ptr<CropOperator> CropOperator::fromJson(const nlohmann::json& json) {
    if (!json.contains("region")) {
        throw std::runtime_error("CropOperator requires 'region' field");
    }
    
    // Parse region
    const auto& region_json = json["region"];
    Rectangle region(
        region_json["p_min"]["x"].get<double>(),
        region_json["p_min"]["y"].get<double>(),
        region_json["p_max"]["x"].get<double>(),
        region_json["p_max"]["y"].get<double>()
    );
    
    // Parse optional fields
    std::optional<int> category;
    if (json.contains("category")) {
        category = json["category"].get<int>();
    }
    
    std::optional<std::vector<long long>> groups;
    if (json.contains("one_of_groups")) {
        groups = json["one_of_groups"].get<std::vector<long long>>();
    }
    
    std::optional<bool> proper;
    if (json.contains("proper")) {
        proper = json["proper"].get<bool>();
    }
    
    return std::make_unique<CropOperator>(region, category, groups, proper);
}