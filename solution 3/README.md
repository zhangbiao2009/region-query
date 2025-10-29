# Solution 3 - Extended Query Processor

## Purpose  
This program implements Task 3 with advanced logical operators for spatial queries. It supports AND, OR, and CROP operations that can be nested to create complex query expressions.

## How to Use

### 1. Build the Program
```bash
cd "solution 3/build"
cmake .. && make
```

### 2. Run Extended Queries
```bash
./extended_query_processor --query=simple_crop.json
./extended_query_processor --query=test_and.json
./extended_query_processor --query=complex_nested.json
```

### 3. Query Format
Create JSON files with nested operators:
```json
{
  "valid_region": {
    "p_min": {"x": 0, "y": 0},
    "p_max": {"x": 1000, "y": 1000}
  },
  "query": {
    "operator_or": [
      {
        "operator_and": [
          {"operator_crop": {"region": {...}}},
          {"operator_crop": {"region": {...}}}
        ]
      },
      {"operator_crop": {"region": {...}}}
    ]
  }
}
```

### 4. Available Operators
- **`operator_crop`**: Find points in rectangular region (reuses Task 2 logic)
- **`operator_and`**: Intersection of multiple operators  
- **`operator_or`**: Union of multiple operators

## How It Works
1. **Recursive Parsing**: Builds operator tree from nested JSON structure
2. **Composite Pattern**: Each operator can contain other operators as children
3. **Set Operations**: AND performs intersection, OR performs union with deduplication
4. **Code Reuse**: Directly includes Solution 2 source files without duplication
5. **Optimization**: Early termination for AND operations when operand returns empty results

This solution extends Task 2 with powerful logical operators while maintaining full backward compatibility.