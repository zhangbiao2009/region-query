#!/usr/bin/env python3
"""
Visualization and Testing Tool for Inspection Region Query System (Task 2)

This script:
1. Connects to the PostgreSQL database
2. Executes queries using the C++ query processor
3. Visualizes points, crop regions, and valid regions
4. Validates query results against expected spatial logic
"""

import psycopg2
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import json
import subprocess
import os
import sys
from typing import List, Tuple, Dict, Optional
import numpy as np

class Point:
    def __init__(self, x: float, y: float, point_id: int = 0, group_id: int = 0, category: int = 0):
        self.x = x
        self.y = y
        self.id = point_id
        self.group_id = group_id
        self.category = category
    
    def __repr__(self):
        return f"Point({self.x:.2f}, {self.y:.2f}, id={self.id}, group={self.group_id}, cat={self.category})"

class Rectangle:
    def __init__(self, x_min: float, y_min: float, x_max: float, y_max: float):
        self.x_min = x_min
        self.y_min = y_min
        self.x_max = x_max
        self.y_max = y_max
    
    def contains(self, point: Point) -> bool:
        return (self.x_min <= point.x <= self.x_max and 
                self.y_min <= point.y <= self.y_max)
    
    def width(self) -> float:
        return self.x_max - self.x_min
    
    def height(self) -> float:
        return self.y_max - self.y_min

class QueryTester:
    def __init__(self, db_connection_string: str = "postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"):
        self.db_connection_string = db_connection_string
        self.query_processor_path = "./query_processor"
        
    def connect_to_database(self):
        """Connect to PostgreSQL database"""
        try:
            # Parse connection string
            parts = self.db_connection_string.replace("postgresql://", "").split("@")
            user_pass = parts[0].split(":")
            host_db = parts[1].split("/")
            host_port = host_db[0].split(":")
            
            self.conn = psycopg2.connect(
                host=host_port[0],
                port=int(host_port[1]) if len(host_port) > 1 else 5432,
                database=host_db[1],
                user=user_pass[0],
                password=user_pass[1]
            )
            print("✓ Connected to database successfully")
            return True
        except Exception as e:
            print(f"❌ Database connection failed: {e}")
            return False
    
    def get_all_points(self) -> List[Point]:
        """Fetch all points from database"""
        cursor = self.conn.cursor()
        cursor.execute("SELECT id, coord_x, coord_y, group_id, category FROM inspection_region ORDER BY id")
        
        points = []
        for row in cursor.fetchall():
            points.append(Point(row[1], row[2], row[0], row[3], row[4]))
        
        cursor.close()
        print(f"✓ Loaded {len(points)} points from database")
        return points
    
    def create_test_query(self, valid_region: Rectangle, crop_region: Rectangle, 
                         category: Optional[int] = None, 
                         groups: Optional[List[int]] = None,
                         proper: Optional[bool] = None) -> str:
        """Create a JSON test query"""
        query = {
            "valid_region": {
                "p_min": {"x": valid_region.x_min, "y": valid_region.y_min},
                "p_max": {"x": valid_region.x_max, "y": valid_region.y_max}
            },
            "query": {
                "operator_crop": {
                    "region": {
                        "p_min": {"x": crop_region.x_min, "y": crop_region.y_min},
                        "p_max": {"x": crop_region.x_max, "y": crop_region.y_max}
                    }
                }
            }
        }
        
        # Add optional filters
        if category is not None:
            query["query"]["operator_crop"]["category"] = category
        
        if groups is not None:
            query["query"]["operator_crop"]["one_of_groups"] = groups
            
        if proper is not None:
            query["query"]["operator_crop"]["proper"] = proper
        
        return json.dumps(query, indent=2)
    
    def execute_cpp_query(self, query_json: str, output_file: str = "test_results.txt") -> List[Point]:
        """Execute query using C++ query processor"""
        # Write query to temporary file
        query_file = "temp_query.json"
        with open(query_file, 'w') as f:
            f.write(query_json)
        
        try:
            # Execute C++ query processor
            result = subprocess.run([
                self.query_processor_path, 
                f"--query={query_file}",
                f"--output={output_file}"
            ], capture_output=True, text=True, cwd=".")
            
            if result.returncode != 0:
                print(f"❌ Query processor failed: {result.stderr}")
                return []
            
            print("✓ C++ query executed successfully")
            
            # Parse results
            points = []
            if os.path.exists(output_file):
                with open(output_file, 'r') as f:
                    for line in f:
                        parts = line.strip().split()
                        if len(parts) >= 2:
                            points.append(Point(float(parts[0]), float(parts[1])))
            
            # Clean up temporary files
            os.remove(query_file)
            if os.path.exists(output_file):
                os.remove(output_file)
                
            return points
            
        except Exception as e:
            print(f"❌ Error executing query: {e}")
            return []
    
    def validate_query_results(self, all_points: List[Point], result_points: List[Point], 
                             valid_region: Rectangle, crop_region: Rectangle,
                             category: Optional[int] = None, groups: Optional[List[int]] = None,
                             proper: Optional[bool] = None) -> bool:
        """Validate that query results match expected spatial logic"""
        print("\n=== Validating Query Results ===")
        
        # Find expected points manually
        expected_points = []
        
        # Step 1: Find proper/improper groups if needed
        proper_groups = set()
        improper_groups = set()
        if proper is not None:  # proper constraint is specified (true or false)
            group_points = {}
            for point in all_points:
                if point.group_id not in group_points:
                    group_points[point.group_id] = []
                group_points[point.group_id].append(point)
            
            for group_id, points in group_points.items():
                if all(valid_region.contains(p) for p in points):
                    proper_groups.add(group_id)
                else:
                    improper_groups.add(group_id)
            
            if proper:
                print(f"Proper groups within valid region: {sorted(proper_groups)}")
            else:
                print(f"Improper groups (not all within valid region): {sorted(improper_groups)}")
        
        # Step 2: Apply all filters
        for point in all_points:
            # Check crop region
            if not crop_region.contains(point):
                continue
                
            # Check proper/improper groups constraint
            if proper is True and point.group_id not in proper_groups:
                continue
            elif proper is False and point.group_id not in improper_groups:
                continue
                
            # Check category filter
            if category is not None and point.category != category:
                continue
                
            # Check group filter
            if groups is not None and point.group_id not in groups:
                continue
                
            expected_points.append(point)
        
        # Sort expected points by (y, x)
        expected_points.sort(key=lambda p: (p.y, p.x))
        
        print(f"Expected points: {len(expected_points)}")
        print(f"Actual points: {len(result_points)}")
        
        # Compare results
        if len(expected_points) != len(result_points):
            print("❌ Point count mismatch!")
            return False
        
        # Check coordinates (with small tolerance for floating point)
        tolerance = 1e-6
        for i, (expected, actual) in enumerate(zip(expected_points, result_points)):
            if (abs(expected.x - actual.x) > tolerance or 
                abs(expected.y - actual.y) > tolerance):
                print(f"❌ Point {i} mismatch: expected ({expected.x}, {expected.y}), got ({actual.x}, {actual.y})")
                return False
        
        print("✅ Query results validation passed!")
        return True
    
    def visualize_query(self, all_points: List[Point], result_points: List[Point],
                       valid_region: Rectangle, crop_region: Rectangle,
                       title: str = "Query Visualization", 
                       filename: str = "query_result.png",
                       query_spec: Optional[Dict] = None):
        """Create visualization of query results"""
        fig, ax = plt.subplots(1, 1, figsize=(12, 10))
        
        # Color map for categories
        category_colors = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray']
        
        # Group points by category for legend
        points_by_category = {}
        for point in all_points:
            if point.category not in points_by_category:
                points_by_category[point.category] = {'points': [], 'color': category_colors[point.category % len(category_colors)]}
            points_by_category[point.category]['points'].append(point)
        
        # Plot all points (bigger with group numbers inside)
        for category, data in points_by_category.items():
            points = data['points']
            color = data['color']
            x_coords = [p.x for p in points]
            y_coords = [p.y for p in points]
            ax.scatter(x_coords, y_coords, c=color, alpha=0.3, s=100, 
                      label=f'All Cat {category} ({len(points)})')
            
            # Add group numbers inside the circles
            for point in points:
                ax.text(point.x, point.y, str(point.group_id), 
                       fontsize=8, weight='bold', color='black',
                       ha='center', va='center', zorder=4)
        
        # Plot result points (keep category colors, add "r" markers)
        if result_points:
            result_by_category = {}
            for point in result_points:
                # Find the full point info from all_points
                full_point = None
                for p in all_points:
                    if abs(p.x - point.x) < 1e-6 and abs(p.y - point.y) < 1e-6:
                        full_point = p
                        break
                
                if full_point:
                    if full_point.category not in result_by_category:
                        result_by_category[full_point.category] = []
                    result_by_category[full_point.category].append(full_point)
            
            for category, points in result_by_category.items():
                color = category_colors[category % len(category_colors)]
                x_coords = [p.x for p in points]
                y_coords = [p.y for p in points]
                ax.scatter(x_coords, y_coords, c=color, alpha=1.0, s=120, 
                          edgecolors='black', linewidth=2, zorder=5,
                          label=f'Result Cat {category} ({len(points)})')
                
                # Add group numbers inside result point circles
                for point in points:
                    ax.text(point.x, point.y, str(point.group_id), 
                           fontsize=9, weight='bold', color='white',
                           ha='center', va='center', zorder=7)
                
                # Add "r" labels next to result points
                for point in points:
                    ax.text(point.x + 15, point.y + 15, 'r', 
                           fontsize=10, weight='bold', color='red',
                           bbox=dict(boxstyle="round,pad=0.2", facecolor='white', alpha=0.8),
                           zorder=6)
        
        # Draw valid region (blue dashed rectangle) only if proper is specified (true or false)
        proper_specified = False
        if query_spec and 'query' in query_spec and 'operator_crop' in query_spec['query']:
            proper_specified = 'proper' in query_spec['query']['operator_crop']
        
        if proper_specified:
            valid_rect = patches.Rectangle(
                (valid_region.x_min, valid_region.y_min),
                valid_region.width(), valid_region.height(),
                fill=False, edgecolor='blue', linestyle='--', linewidth=2,
                label=f'Valid Region'
            )
            ax.add_patch(valid_rect)
        
        # Draw crop region (red solid rectangle)
        crop_rect = patches.Rectangle(
            (crop_region.x_min, crop_region.y_min),
            crop_region.width(), crop_region.height(),
            fill=False, edgecolor='red', linestyle='-', linewidth=3,
            label=f'Crop Region'
        )
        ax.add_patch(crop_rect)
        
        # Group numbers are now shown inside point circles, no separate group labels needed
        
        ax.set_xlabel('X Coordinate')
        ax.set_ylabel('Y Coordinate')
        ax.set_title(f'{title}\nFound {len(result_points)} points')
        ax.legend(bbox_to_anchor=(1.02, 1), loc='upper left')
        ax.grid(True, alpha=0.3)
        ax.set_aspect('equal', adjustable='box')
        
        # Add query JSON text on the right side under the legend if provided
        if query_spec:
            query_text = json.dumps(query_spec, indent=2)
            # Position the text on the right side, below the legend
            fig.text(0.72, 0.45, f"Query JSON:\n{query_text}", 
                     fontsize=7, family='monospace', 
                     bbox=dict(boxstyle="round,pad=0.4", facecolor='lightblue', alpha=0.9),
                     verticalalignment='top', horizontalalignment='left',
                     wrap=True)
        
        plt.tight_layout()
        # Adjust layout to make room for the right-side text
        plt.subplots_adjust(right=0.70)
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        plt.show()
        print(f"✓ Visualization saved to {filename}")
    
    def run_test_suite(self):
        """Run comprehensive test suite with visualizations"""
        if not self.connect_to_database():
            return False
        
        all_points = self.get_all_points()
        if not all_points:
            print("❌ No points found in database")
            return False
        
        # Calculate bounds for test regions
        x_coords = [p.x for p in all_points]
        y_coords = [p.y for p in all_points]
        x_min, x_max = min(x_coords), max(x_coords)
        y_min, y_max = min(y_coords), max(y_coords)
        
        # Add some padding
        padding = 50
        data_bounds = Rectangle(x_min - padding, y_min - padding, 
                               x_max + padding, y_max + padding)
        
        print(f"\nData bounds: ({x_min:.1f}, {y_min:.1f}) to ({x_max:.1f}, {y_max:.1f})")
        
        tests = [
            {
                "name": "Test 1: Simple Crop Query",
                "valid_region": Rectangle(0, 0, 1000, 1000),
                "crop_region": Rectangle(200, 200, 600, 600),
                "category": None,
                "groups": None,
                "proper": None
            },
            {
                "name": "Test 2: Category Filter",
                "valid_region": Rectangle(0, 0, 1000, 1000),
                "crop_region": Rectangle(0, 0, 1000, 1000),
                "category": 1,
                "groups": None,
                "proper": None
            },
            {
                "name": "Test 3: Group Filter",
                "valid_region": Rectangle(0, 0, 1000, 1000),
                "crop_region": Rectangle(0, 0, 1000, 1000),
                "category": 2,
                "groups": [0],
                "proper": None
            },
            {
                "name": "Test 4: Proper Groups Only",
                "valid_region": Rectangle(10, 10, 900, 900),
                "crop_region": Rectangle(0, 0, 1000, 1000),
                "category": None,
                "groups": None,
                "proper": True
            },
            {
                "name": "Test 5: Improper Groups Only",
                "valid_region": Rectangle(100, 10, 900, 950),
                "crop_region": Rectangle(0, 0, 1000, 1000),
                "category": None,
                "groups": None,
                "proper": False
            },
            {
                "name": "Test 6: Complex Query",
                "valid_region": Rectangle(0, 0, 1000, 1000),
                "crop_region": Rectangle(200, 300, 700, 800),
                "category": 1,
                "groups": [0],
                "proper": True
            },
            {
                "name": "Test 7: Small Valid Region vs Large Crop",
                "valid_region": Rectangle(400, 400, 600, 600),
                "crop_region": Rectangle(0, 0, 1200, 1200),
                "category": None,
                "groups": None,
                "proper": None
            }
        ]
        
        all_passed = True
        
        for i, test in enumerate(tests):
            print(f"\n{'='*50}")
            print(f"Running {test['name']}")
            print(f"{'='*50}")
            
            # Create and execute query
            query_json = self.create_test_query(
                test['valid_region'], test['crop_region'],
                test['category'], test['groups'], test['proper']
            )
            
            print("Query JSON:")
            print(query_json)
            
            result_points = self.execute_cpp_query(query_json)
            
            # Validate results
            validation_passed = self.validate_query_results(
                all_points, result_points,
                test['valid_region'], test['crop_region'],
                test['category'], test['groups'], test['proper']
            )
            
            if not validation_passed:
                all_passed = False
            
            # Create visualization
            filename = f"test_{i+1}_{test['name'].lower().replace(' ', '_').replace(':', '')}.png"
            query_spec_dict = json.loads(query_json)
            self.visualize_query(
                all_points, result_points,
                test['valid_region'], test['crop_region'],
                test['name'], filename, query_spec_dict
            )
        
        self.conn.close()
        
        print(f"\n{'='*50}")
        print(f"TEST SUITE RESULTS")
        print(f"{'='*50}")
        if all_passed:
            print("✅ All tests passed!")
        else:
            print("❌ Some tests failed!")
        
        return all_passed

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--help":
        print(__doc__)
        return
    
    print("Inspection Region Query System - Test & Visualization Tool")
    print("=" * 60)
    
    # Check if we're in the right directory
    if not os.path.exists("./query_processor"):
        print("❌ query_processor executable not found!")
        print("Please run this script from the 'solution 2/build' directory")
        return
    
    tester = QueryTester()
    tester.run_test_suite()

if __name__ == "__main__":
    main()