#!/usr/bin/env python3
"""
Extended Visualization Tool for Task 3 Queries (AND/OR Operations)

Usage:
  python3 extended_visualizer.py query.json [output_image.png]

This script:
1. Executes the C++ extended_query_processor with the given JSON query
2. Loads all points from the database
3. Creates a visualization showing:
   - All points (faded by category)
   - Query result points (highlighted)
   - Valid region (blue dashed rectangle)
   - All crop regions involved in the query (different colors)
   - Logical operation structure
   - Group labels and detailed query info
"""

import psycopg2
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import json
import subprocess
import os
import sys
import numpy as np
from matplotlib.colors import ListedColormap

def load_points_from_db():
    """Load all points from database"""
    try:
        conn = psycopg2.connect(
            host="localhost",
            port=5432,
            database="inspection_db",
            user="inspection_user",
            password="inspection_pass"
        )
        
        cursor = conn.cursor()
        cursor.execute("SELECT id, coord_x, coord_y, group_id, category FROM inspection_region ORDER BY id")
        
        points = []
        for row in cursor.fetchall():
            points.append({
                'id': row[0],
                'x': row[1], 
                'y': row[2],
                'group_id': row[3],
                'category': row[4]
            })
        
        cursor.close()
        conn.close()
        return points
        
    except Exception as e:
        print(f"Database error: {e}")
        return []

def execute_extended_query(query_file):
    """Execute C++ extended_query_processor"""
    try:
        # Get just the filename if it's a path
        query_filename = os.path.basename(query_file)
        
        result = subprocess.run([
            "./extended_query_processor", 
            f"--query={query_filename}",
            "--output=extended_viz_results.txt"
        ], capture_output=True, text=True, cwd="../solution 3/build")
        
        if result.returncode != 0:
            print(f"Extended query failed: {result.stderr}")
            return []
        
        # Parse results
        result_points = []
        result_file = "../solution 3/build/extended_viz_results.txt"
        if os.path.exists(result_file):
            with open(result_file, 'r') as f:
                for line in f:
                    parts = line.strip().split()
                    if len(parts) >= 2:
                        result_points.append({'x': float(parts[0]), 'y': float(parts[1])})
            os.remove(result_file)
        
        return result_points
        
    except Exception as e:
        print(f"Extended query execution error: {e}")
        return []

def extract_crop_regions(query_obj, regions_list=None):
    """Recursively extract all crop regions from nested query structure"""
    if regions_list is None:
        regions_list = []
    
    if isinstance(query_obj, dict):
        if "operator_crop" in query_obj:
            crop_info = query_obj["operator_crop"]
            region_info = {
                'region': crop_info['region'],
                'type': 'crop',
                'filters': {}
            }
            
            # Add filter information
            if 'category' in crop_info:
                region_info['filters']['category'] = crop_info['category']
            if 'one_of_groups' in crop_info:
                region_info['filters']['one_of_groups'] = crop_info['one_of_groups']
            if 'proper' in crop_info:
                region_info['filters']['proper'] = crop_info['proper']
                
            regions_list.append(region_info)
        
        elif "operator_and" in query_obj:
            for operand in query_obj["operator_and"]:
                extract_crop_regions(operand, regions_list)
        
        elif "operator_or" in query_obj:
            for operand in query_obj["operator_or"]:
                extract_crop_regions(operand, regions_list)
        
        # Recursively check all values
        for value in query_obj.values():
            if isinstance(value, (dict, list)):
                extract_crop_regions(value, regions_list)
    
    elif isinstance(query_obj, list):
        for item in query_obj:
            extract_crop_regions(item, regions_list)
    
    return regions_list

def get_query_description(query_obj):
    """Generate human-readable description of the query structure"""
    if isinstance(query_obj, dict):
        if "operator_crop" in query_obj:
            crop = query_obj["operator_crop"]
            desc = "CROP"
            filters = []
            if 'category' in crop:
                filters.append(f"cat={crop['category']}")
            if 'one_of_groups' in crop:
                groups = crop['one_of_groups']
                if len(groups) <= 3:
                    filters.append(f"groups={groups}")
                else:
                    filters.append(f"groups=[{groups[0]}...{groups[-1]}]({len(groups)})")
            if 'proper' in crop:
                filters.append(f"proper={crop['proper']}")
            
            if filters:
                desc += f"({', '.join(filters)})"
            return desc
        
        elif "operator_and" in query_obj:
            operands = [get_query_description(op) for op in query_obj["operator_and"]]
            return f"AND[{', '.join(operands)}]"
        
        elif "operator_or" in query_obj:
            operands = [get_query_description(op) for op in query_obj["operator_or"]]
            return f"OR[{', '.join(operands)}]"
    
    return str(query_obj)

def visualize_extended(all_points, result_points, query_data, output_file="extended_query_visualization.png"):
    """Create extended visualization for AND/OR queries"""
    fig, ax = plt.subplots(1, 1, figsize=(16, 12))
    
    # Extract regions and query structure
    valid_region = query_data["valid_region"]
    crop_regions = extract_crop_regions(query_data["query"])
    query_desc = get_query_description(query_data["query"])
    
    # Color maps
    category_colors = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray']
    region_colors = ['darkred', 'darkblue', 'darkgreen', 'darkorange', 'darkviolet', 'darkgoldenrod', 'deeppink', 'darkslategray']
    
    # Plot all points (faded with group numbers)
    categories = set(p['category'] for p in all_points)
    for cat in categories:
        cat_points = [p for p in all_points if p['category'] == cat]
        x_coords = [p['x'] for p in cat_points]
        y_coords = [p['y'] for p in cat_points]
        color = category_colors[cat % len(category_colors)]
        ax.scatter(x_coords, y_coords, c=color, alpha=0.2, s=120, 
                  label=f'All Category {cat} ({len(cat_points)})')
        
        # Add group numbers inside the circles
        for point in cat_points:
            ax.text(point['x'], point['y'], str(point['group_id']), 
                   fontsize=7, weight='bold', color='black',
                   ha='center', va='center', zorder=3)
    
    # Plot result points (highlighted)
    if result_points:
        # Find full point info for each result point
        result_points_full = []
        for result_point in result_points:
            for full_point in all_points:
                if abs(full_point['x'] - result_point['x']) < 1e-6 and abs(full_point['y'] - result_point['y']) < 1e-6:
                    result_points_full.append(full_point)
                    break
        
        # Plot result points by category (full opacity)
        result_by_category = {}
        for point in result_points_full:
            cat = point['category']
            if cat not in result_by_category:
                result_by_category[cat] = []
            result_by_category[cat].append(point)
        
        for cat, points in result_by_category.items():
            x_coords = [p['x'] for p in points]
            y_coords = [p['y'] for p in points]
            color = category_colors[cat % len(category_colors)]
            ax.scatter(x_coords, y_coords, c=color, alpha=1.0, s=150, 
                      edgecolors='black', linewidth=3, zorder=6,
                      label=f'★ Result Cat {cat} ({len(points)})')
            
            # Add group numbers inside result point circles
            for point in points:
                ax.text(point['x'], point['y'], str(point['group_id']), 
                       fontsize=8, weight='bold', color='white',
                       ha='center', va='center', zorder=8)
            
            # Add star markers next to result points
            for point in points:
                ax.text(point['x'] + 15, point['y'] + 15, '★', 
                       fontsize=16, weight='bold', color='gold',
                       ha='center', va='center', zorder=7)
    
    # Draw valid region (blue dashed)
    has_proper_constraint = any('proper' in region.get('filters', {}) for region in crop_regions)
    if has_proper_constraint:
        valid_rect = patches.Rectangle(
            (valid_region["p_min"]["x"], valid_region["p_min"]["y"]),
            valid_region["p_max"]["x"] - valid_region["p_min"]["x"],
            valid_region["p_max"]["y"] - valid_region["p_min"]["y"],
            fill=False, edgecolor='blue', linestyle='--', linewidth=2,
            label=f'Valid Region (proper constraint)', zorder=2
        )
        ax.add_patch(valid_rect)
    
    # Draw all crop regions with different colors and styles
    for i, region_info in enumerate(crop_regions):
        region = region_info['region']
        filters = region_info.get('filters', {})
        color = region_colors[i % len(region_colors)]
        
        # Create label with filter info
        label_parts = [f'Crop Region {i+1}']
        if filters:
            filter_strs = []
            for key, value in filters.items():
                if key == 'one_of_groups' and isinstance(value, list) and len(value) > 3:
                    filter_strs.append(f"{key}=[{value[0]}...{value[-1]}]({len(value)})")
                else:
                    filter_strs.append(f"{key}={value}")
            label_parts.append(f"({', '.join(filter_strs)})")
        
        label = ' '.join(label_parts)
        
        crop_rect = patches.Rectangle(
            (region["p_min"]["x"], region["p_min"]["y"]),
            region["p_max"]["x"] - region["p_min"]["x"],
            region["p_max"]["y"] - region["p_min"]["y"],
            fill=False, edgecolor=color, linestyle='-', linewidth=2,
            label=label, zorder=4
        )
        ax.add_patch(crop_rect)
        
        # Add region number label
        center_x = (region["p_min"]["x"] + region["p_max"]["x"]) / 2
        center_y = region["p_max"]["y"] + 20
        ax.text(center_x, center_y, f'R{i+1}', 
               fontsize=12, weight='bold', color=color,
               ha='center', va='center',
               bbox=dict(boxstyle="round,pad=0.3", facecolor='white', alpha=0.8),
               zorder=5)
    
    # Format plot
    ax.set_xlabel('X Coordinate', fontsize=12)
    ax.set_ylabel('Y Coordinate', fontsize=12)
    
    # Build title with query structure
    title = f"Extended Query Results: {len(result_points)} points\nQuery: {query_desc}"
    ax.set_title(title, fontsize=14, weight='bold')
    
    # Legend on the right
    ax.legend(bbox_to_anchor=(1.02, 1), loc='upper left', fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_aspect('equal', adjustable='box')
    
    # Add query JSON text on the right side
    query_text = json.dumps(query_data, indent=2)
    fig.text(0.70, 0.65, f"Query JSON:\n{query_text}", 
             fontsize=6, family='monospace', 
             bbox=dict(boxstyle="round,pad=0.4", facecolor='lightgreen', alpha=0.9),
             verticalalignment='top', horizontalalignment='left')
    
    # Query structure explanation
    structure_text = f"Query Structure:\n{query_desc}\n\nRegions: {len(crop_regions)}\nLogical Ops: {'AND' if 'operator_and' in str(query_data) else ''}{'OR' if 'operator_or' in str(query_data) else ''}"
    fig.text(0.70, 0.35, structure_text,
             fontsize=8, weight='bold',
             bbox=dict(boxstyle="round,pad=0.4", facecolor='lightyellow', alpha=0.9),
             verticalalignment='top', horizontalalignment='left')
    
    plt.tight_layout()
    plt.subplots_adjust(right=0.68)  # Make room for right-side information
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"✓ Extended visualization saved to {output_file}")
    plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 extended_visualizer.py query.json [output.png]")
        print("Example: python3 extended_visualizer.py data1_test_query.json")
        print("")
        print("This visualizer supports Task 3 queries with:")
        print("  - operator_crop (basic crop operations)")
        print("  - operator_and (intersection of multiple operations)")
        print("  - operator_or (union of multiple operations)")
        print("  - Nested combinations of the above")
        return
    
    query_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "extended_query_visualization.png"
    
    if not os.path.exists(query_file):
        print(f"Query file not found: {query_file}")
        return
    
    # Check if we can access the extended_query_processor
    extended_processor = "../solution 3/build/extended_query_processor"
    if not os.path.exists(extended_processor):
        print("extended_query_processor executable not found!")
        print("Please make sure solution 3 is built and run this script from solution 2/build directory")
        return
    
    print(f"Loading extended query from: {query_file}")
    
    # Load and parse query
    with open(query_file, 'r') as f:
        query_data = json.load(f)
    
    # Validate it's an extended query
    query_obj = query_data.get("query", {})
    has_extended_ops = any(key in query_obj for key in ["operator_and", "operator_or", "operator_crop"])
    
    if not has_extended_ops:
        print("Warning: This doesn't appear to be a valid Task 3 extended query")
        print("Expected operators: operator_crop, operator_and, operator_or")
    
    print("Loading points from database...")
    all_points = load_points_from_db()
    if not all_points:
        print("No points found in database!")
        return
    
    print(f"Loaded {len(all_points)} points from database")
    
    # Extract and display query structure
    crop_regions = extract_crop_regions(query_data["query"])
    query_desc = get_query_description(query_data["query"])
    print(f"Query structure: {query_desc}")
    print(f"Found {len(crop_regions)} crop regions")
    
    print("Executing C++ extended query processor...")
    result_points = execute_extended_query(query_file)
    print(f"Extended query returned {len(result_points)} points")
    
    # Print detailed information about returned points
    if result_points:
        print("\nReturned points details:")
        print("-" * 60)
        for i, point in enumerate(result_points[:10], 1):  # Show first 10
            # Find full point info from database
            full_point = None
            for db_point in all_points:
                if abs(db_point['x'] - point['x']) < 1e-6 and abs(db_point['y'] - point['y']) < 1e-6:
                    full_point = db_point
                    break
            
            if full_point:
                print(f"Point {i:2d}: ID={full_point['id']:2d}, Coords=({point['x']:6.1f}, {point['y']:6.1f}), "
                      f"Group={full_point['group_id']}, Category={full_point['category']}")
            else:
                print(f"Point {i:2d}: Coords=({point['x']:6.1f}, {point['y']:6.1f}) [details not found]")
        
        if len(result_points) > 10:
            print(f"... and {len(result_points) - 10} more points")
        print("-" * 60)
    else:
        print("\nNo points returned by the extended query.")
    
    print("Creating extended visualization...")
    visualize_extended(all_points, result_points, query_data, output_file)
    print("Done!")

if __name__ == "__main__":
    main()