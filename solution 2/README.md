# Solution 2 - Spatial Query Processor

## Purpose
This program performs advanced spatial queries on a PostgreSQL database. It finds points within rectangular regions and supports filtering by categories, groups, and proper/improper constraints.

## How to Use

### 1. Build the Program
```bash
cd "solution 2/build"
cmake .. && make
```

### 2. Run Spatial Queries
```bash
./query_processor --query=../simple_query.json
```

### 3. Test with Visualization (Python)
```bash
python3 test_visualization.py
python3 simple_visualizer.py simple_query.json
```

### 4. Query Format
Create JSON files like this:
```json
{
  "valid_region": {
    "p_min": {"x": 0, "y": 0},
    "p_max": {"x": 1000, "y": 1000}
  },
  "crop_query": {
    "region": {
      "p_min": {"x": 100, "y": 100},
      "p_max": {"x": 500, "y": 500}
    },
    "category": 2,
    "one_of_groups": [0, 1],
    "proper": true
  }
}
```

## How It Works
1. **JSON Parsing**: Reads complex query specifications with optional filters
2. **Database Query**: Executes optimized SQL with spatial and categorical constraints  
3. **Proper Logic**: Handles three-state proper constraint (true/false/null)
4. **Result Processing**: Returns sorted, filtered points
5. **Testing**: Includes brute-force validation and Python visualization tools

This solution provides the complete Task 2 functionality with comprehensive testing infrastructure.