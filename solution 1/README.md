# Inspection Region System - Solution 1

This solution implements Task 1 of the inspection region management system: loading 2D point data into PostgreSQL.

## Requirements

### System Dependencies
- **C++17** compiler (GCC 8+, Clang 8+, or MSVC 2019+)
- **CMake** 3.16 or higher
- **Docker Desktop** (for PostgreSQL container)
- **libpqxx** 7.0+ (PostgreSQL C++ library)

### Installing Dependencies

#### macOS (using Homebrew)
```bash
brew install libpqxx cmake
```

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install libpq-dev libpqxx-dev cmake build-essential
```

#### CentOS/RHEL/Fedora
```bash
sudo dnf install libpqxx-devel cmake gcc-c++
```

## Database Setup with Docker 🐳

**No need to install PostgreSQL locally!** We use Docker for a clean, isolated database setup.

### Quick Start
```bash
# Start PostgreSQL container
./docker-db.sh start

# Build and test everything
./docker-db.sh build-and-test
```

### Docker Commands
```bash
# Start database container
./docker-db.sh start

# Check database status
./docker-db.sh status

# Connect to database with psql
./docker-db.sh connect

# View database logs
./docker-db.sh logs

# Schema management
./docker-db.sh schema          # Create/recreate schema and indexes
./docker-db.sh clear           # Clear data (preserve schema)

# Stop database
./docker-db.sh stop

# Reset database (destroys all data and schema)
./docker-db.sh reset
```

### Manual Docker Setup (if needed)
```bash
# Start PostgreSQL container
docker-compose up -d

# Check if it's running
docker-compose ps

# Connect to database
docker-compose exec postgres psql -U inspection_user -d inspection_db
```

### Database Connection Details
- **Host:** localhost
- **Port:** 5432
- **Database:** inspection_db
- **Username:** inspection_user
- **Password:** inspection_pass
- **Connection String:** `postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db`

## Building the Project

1. **Clone/navigate to project directory:**
   ```bash
   cd "/Users/bzhang/Downloads/cpp_try/solution 1"
   ```

2. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake:**
   ```bash
   cmake ..
   ```

4. **Build the project:**
   ```bash
   make -j$(nproc)
   ```

## Running Task 1: Data Loading

### Complete Workflow (Recommended)
```bash
# One command does everything: start DB, build, test
./docker-db.sh build-and-test
```

### Step-by-Step Usage
```bash
# 1. Start PostgreSQL container
./docker-db.sh start

# 2. Build project
mkdir build && cd build
cmake .. && make

# 3. Run data loader
./data_loader --data_directory=../../data/0 --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"
```

### Quick Testing
```bash
# Test with dataset 0
./data_loader --data_directory=../data/0 --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"

# Test with dataset 1  
./data_loader --data_directory=../data/1 --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"
```

### Command Line Options
- `--data_directory=PATH`: **Required.** Path to directory containing data files
- `--database=CONNECTION`: Optional. PostgreSQL connection string (default: `postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db`)
- `--help`: Show usage information

### Expected Output
```
Inspection Region Data Loader - Task 1
=======================================
Data directory: ../data/0
Database: postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db

Connecting to database...
✓ Database connection established
=== Task 1: Data Loading ===
Data directory: ../data/0
✓ File validation passed
Parsing data files...
✓ Parsed 11 points
✓ Parsed 11 categories
✓ Parsed 11 group assignments
✓ Data consistency validation passed
Creating database tables...
Database tables created successfully.
Clearing existing data...
Tables cleared successfully.
Found 2 unique groups
Inserting 2 unique groups...
Groups inserted successfully.
Inserting 11 points...
Points inserted successfully.
Creating database indexes for optimal performance...
Database indexes created successfully.

=== Data Loading Summary ===
Groups in database: 2
Points in database: 11
✅ Data loading completed successfully!

=== Performance Summary ===
Total execution time: 156 ms
Final database state:
  - Groups: 2
  - Points: 11

🎉 Task 1 completed successfully!
The database is now ready for spatial queries (Task 2).
```

## Data Format

The program expects three synchronized text files in the data directory:

### points.txt
- One point per line: `x y`
- Coordinates in scientific notation format
- Example: `5.773781372021976495e+02 6.029674418592738903e+02`

### categories.txt
- One category ID per line
- Integer values in scientific notation
- Example: `2.000000000000000000e+00`

### groups.txt
- One group ID per line
- Integer values in scientific notation
- Example: `0.000000000000000000e+00`

**Important:** Line `i` in all three files corresponds to the same inspection region.

## Database Schema

The database schema is managed via SQL scripts (not C++ code):

### Schema Files
- **`init.sql`** - Automatically creates schema when Docker container starts
- **`schema.sql`** - Manual schema creation/recreation script  
- **`clear_data.sql`** - Clears data while preserving schema

### Tables Created
```sql
-- Groups table
CREATE TABLE inspection_group (
    id BIGINT NOT NULL,
    PRIMARY KEY (id)
);

-- Regions/Points table
CREATE TABLE inspection_region (
    id BIGINT NOT NULL,
    group_id BIGINT,
    coord_x FLOAT,
    coord_y FLOAT,
    category INTEGER,
    PRIMARY KEY (id),
    FOREIGN KEY (group_id) REFERENCES inspection_group(id)
);
```

### Performance Indexes (Automatically Created)
- **`idx_spatial_gist`** - GIST spatial index for 2D range queries
- **`idx_group_id`** - B-tree index for JOIN operations  
- **`idx_category`** - B-tree index for category filtering
- **`idx_group_spatial`** - Composite index for proper constraint optimization
- **`idx_sort`** - Index for result sorting by (y, x)

### Schema Management
```bash
# Recreate schema manually
./docker-db.sh schema

# Clear data only
./docker-db.sh clear

# Connect and run custom SQL
./docker-db.sh connect
```

## Troubleshooting

### Build Issues
1. **libpqxx not found:**
   ```bash
   # Install libpqxx development package
   # Then try: export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
   ```

2. **PostgreSQL headers missing:**
   ```bash
   # Install postgresql-devel or libpq-dev package
   ```

### Runtime Issues
1. **Database connection failed:**
   - Check Docker container is running: `./docker-db.sh status`
   - Start database if needed: `./docker-db.sh start`
   - Check logs: `./docker-db.sh logs`

2. **File not found:**
   - Verify data directory path is correct
   - Check all three files (points.txt, categories.txt, groups.txt) exist

3. **Permission denied:**
   - Check file permissions are readable
   - For Docker issues, try: `./docker-db.sh restart`

4. **Docker issues:**
   - Ensure Docker Desktop is running
   - Reset database: `./docker-db.sh reset`
   - Check Docker logs: `docker-compose logs`

### Performance Notes
- Initial run may be slower due to index creation
- Subsequent runs will be faster with warm database cache
- Large datasets (>100K points) may take several minutes

## Testing

### Easy Testing
```bash
# Test everything with one command
./docker-db.sh build-and-test
```

### Manual Testing
```bash
# Start database
./docker-db.sh start

# Build and test
cd build
./data_loader --data_directory=../../data/0 --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"
./data_loader --data_directory=../../data/1 --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"
```

### Verify Results
```bash
# Connect to database
./docker-db.sh connect

# Check data in PostgreSQL
SELECT COUNT(*) FROM inspection_group;
SELECT COUNT(*) FROM inspection_region;
SELECT * FROM inspection_region LIMIT 5;
```

## Next Steps

After successful completion of Task 1, you can proceed with:
- **Task 2:** Spatial query implementation
- **Task 3:** Logical operators (AND/OR)

The database is now optimally configured with indexes for efficient spatial queries.