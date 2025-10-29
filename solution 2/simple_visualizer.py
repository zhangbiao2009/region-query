#!/usr/bin/env python3
"""
Simple Visualization Tool for Query Results

Usage:
  python3 simple_visualizer.py query.json [output_image.png]

This script:
1. Executes the C++ query processor with the given JSON query
2. Loads all points from the database
3. Creates a visualization showing:
   - All points (faded by category)
   - Query result points (highlighted)
   - Valid region (blue dashed rectangle)
   - Crop region (red solid rectangle)
   - Group labels
"""

import psycopg2
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import json
import subprocess
import os
import sys
import numpy as np

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

def execute_query(query_file):
    """Execute C++ query processor"""
    try:
        result = subprocess.run([
            "./query_processor", 
            f"--query={query_file}",
            "--output=viz_results.txt"
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"Query failed: {result.stderr}")
            return []
        
        # Parse results
        result_points = []
        if os.path.exists("viz_results.txt"):
            with open("viz_results.txt", 'r') as f:
                for line in f:
                    parts = line.strip().split()
                    if len(parts) >= 2:
                        result_points.append({'x': float(parts[0]), 'y': float(parts[1])})
            os.remove("viz_results.txt")
        
        return result_points
        
    except Exception as e:
        print(f"Query execution error: {e}")
        return []

def visualize(all_points, result_points, query_data, output_file="query_visualization.png"):
    """Create visualization"""
    fig, ax = plt.subplots(1, 1, figsize=(12, 10))
    
    # Extract regions from query
    valid_region = query_data["valid_region"]
    crop_region = query_data["query"]["operator_crop"]["region"]
    
    # Color map for categories  
    colors = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray']
    
    # Plot all points (bigger with group numbers inside)
    categories = set(p['category'] for p in all_points)
    for cat in categories:
        cat_points = [p for p in all_points if p['category'] == cat]
        x_coords = [p['x'] for p in cat_points]
        y_coords = [p['y'] for p in cat_points]
        color = colors[cat % len(colors)]
        ax.scatter(x_coords, y_coords, c=color, alpha=0.3, s=100, 
                  label=f'All Category {cat} ({len(cat_points)})')
        
        # Add group numbers inside the circles
        for point in cat_points:
            ax.text(point['x'], point['y'], str(point['group_id']), 
                   fontsize=8, weight='bold', color='black',
                   ha='center', va='center', zorder=4)
    
    # Plot result points (keep category colors, add "r" markers)
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
            color = colors[cat % len(colors)]
            ax.scatter(x_coords, y_coords, c=color, alpha=1.0, s=120, 
                      edgecolors='black', linewidth=2, zorder=5,
                      label=f'Result Cat {cat} ({len(points)})')
            
            # Add group numbers inside result point circles
            for point in points:
                ax.text(point['x'], point['y'], str(point['group_id']), 
                       fontsize=9, weight='bold', color='white',
                       ha='center', va='center', zorder=7)
            
            # Add "r" labels next to result points
            for point in points:
                ax.text(point['x'] + 15, point['y'] + 15, 'r', 
                       fontsize=12, weight='bold', color='red',
                       bbox=dict(boxstyle="round,pad=0.2", facecolor='white', alpha=0.8),
                       zorder=6)
    
    # Draw valid region (blue dashed) only if proper is specified (true or false)
    op_crop = query_data['query']['operator_crop']
    proper_specified = 'proper' in op_crop
    
    if proper_specified:
        valid_rect = patches.Rectangle(
            (valid_region["p_min"]["x"], valid_region["p_min"]["y"]),
            valid_region["p_max"]["x"] - valid_region["p_min"]["x"],
            valid_region["p_max"]["y"] - valid_region["p_min"]["y"],
            fill=False, edgecolor='blue', linestyle='--', linewidth=2,
            label='Valid Region'
        )
        ax.add_patch(valid_rect)
    
    # Draw crop region (red solid)
    crop_rect = patches.Rectangle(
        (crop_region["p_min"]["x"], crop_region["p_min"]["y"]),
        crop_region["p_max"]["x"] - crop_region["p_min"]["x"],
        crop_region["p_max"]["y"] - crop_region["p_min"]["y"],
        fill=False, edgecolor='red', linestyle='-', linewidth=3,
        label='Crop Region'
    )
    ax.add_patch(crop_rect)
    
    # Group numbers are now shown inside point circles, no separate group labels needed
    
    # Format plot
    ax.set_xlabel('X Coordinate', fontsize=12)
    ax.set_ylabel('Y Coordinate', fontsize=12)
    
    # Build title with query info
    title_parts = [f"Query Results: {len(result_points)} points found"]
    op_crop = query_data["query"]["operator_crop"]
    if "category" in op_crop:
        title_parts.append(f"Category: {op_crop['category']}")
    if "one_of_groups" in op_crop:
        title_parts.append(f"Groups: {op_crop['one_of_groups']}")
    if "proper" in op_crop:
        title_parts.append(f"Proper: {op_crop['proper']}")
    
    ax.set_title('\n'.join(title_parts), fontsize=14, weight='bold')
    ax.legend(bbox_to_anchor=(1.02, 1), loc='upper left')
    ax.grid(True, alpha=0.3)
    ax.set_aspect('equal', adjustable='box')
    
    # Add query JSON text on the right side under the legend
    query_text = json.dumps(query_data, indent=2)
    fig.text(0.72, 0.45, f"Query JSON:\n{query_text}", 
             fontsize=7, family='monospace', 
             bbox=dict(boxstyle="round,pad=0.4", facecolor='lightblue', alpha=0.9),
             verticalalignment='top', horizontalalignment='left',
             wrap=True)
    
    plt.tight_layout()
    # Adjust layout to make room for the right-side text
    plt.subplots_adjust(right=0.70)
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"✓ Visualization saved to {output_file}")
    plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 simple_visualizer.py query.json [output.png]")
        print("Example: python3 simple_visualizer.py ../simple_query.json")
        return
    
    query_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "query_visualization.png"
    
    if not os.path.exists(query_file):
        print(f"Query file not found: {query_file}")
        return
    
    if not os.path.exists("./query_processor"):
        print("query_processor executable not found!")
        print("Please run this script from the 'solution 2/build' directory")
        return
    
    print(f"Loading query from: {query_file}")
    
    # Load and parse query
    with open(query_file, 'r') as f:
        query_data = json.load(f)
    
    print("Loading points from database...")
    all_points = load_points_from_db()
    if not all_points:
        print("No points found in database!")
        return
    
    print(f"Loaded {len(all_points)} points from database")
    
    print("Executing C++ query...")
    result_points = execute_query(query_file)
    print(f"Query returned {len(result_points)} points")
    
    # Print detailed information about returned points
    if result_points:
        print("\nReturned points details:")
        print("-" * 50)
        for i, point in enumerate(result_points, 1):
            # Find full point info from database
            full_point = None
            for db_point in all_points:
                if abs(db_point['x'] - point['x']) < 1e-6 and abs(db_point['y'] - point['y']) < 1e-6:
                    full_point = db_point
                    break
            
            if full_point:
                print(f"Point {i}: ID={full_point['id']}, Coords=({point['x']:.1f}, {point['y']:.1f}), "
                      f"Group={full_point['group_id']}, Category={full_point['category']}")
            else:
                print(f"Point {i}: Coords=({point['x']:.1f}, {point['y']:.1f}) [details not found]")
        print("-" * 50)
    else:
        print("\nNo points returned by the query.")
    
    print("Creating visualization...")
    visualize(all_points, result_points, query_data, output_file)
    print("Done!")

if __name__ == "__main__":
    main()