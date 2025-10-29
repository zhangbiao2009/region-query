-- Initialization script for PostgreSQL container
-- This runs automatically when the container is first created

-- Create extensions that might be useful for spatial operations
CREATE EXTENSION IF NOT EXISTS btree_gist;

-- Create the database schema
-- Drop existing tables if they exist (for clean setup)
DROP TABLE IF EXISTS inspection_region CASCADE;
DROP TABLE IF EXISTS inspection_group CASCADE;

-- Create inspection_group table
CREATE TABLE inspection_group (
    id BIGINT NOT NULL,
    PRIMARY KEY (id)
);

-- Create inspection_region table
CREATE TABLE inspection_region (
    id BIGINT NOT NULL,
    group_id BIGINT,
    coord_x FLOAT,
    coord_y FLOAT,
    category INTEGER,
    PRIMARY KEY (id),
    FOREIGN KEY (group_id) REFERENCES inspection_group(id)
);

-- Create performance indexes for spatial queries
-- 1. Spatial index (GIST) for 2D range queries - CRITICAL for crop operations
CREATE INDEX idx_spatial_gist ON inspection_region 
USING gist(point(coord_x, coord_y));

-- 2. Group index for JOIN operations - CRITICAL for proper constraint
CREATE INDEX idx_group_id ON inspection_region(group_id);

-- 3. Category index for filtering
CREATE INDEX idx_category ON inspection_region(category);

-- 4. Composite index for proper constraint optimization
CREATE INDEX idx_group_spatial ON inspection_region(group_id, coord_x, coord_y);

-- 5. Index for sorting results by (y, x)
CREATE INDEX idx_sort ON inspection_region(coord_y, coord_x);

-- Verify database setup
SELECT 'PostgreSQL container initialized successfully with schema and indexes' as status;