# Solution 1 - Data Loading Program

## Purpose
This program loads 2D point data into a PostgreSQL database. It reads coordinate files and stores them in a structured database format for spatial queries.

## How to Use

### 1. Start Database
```bash
./docker-db.sh start
```

### 2. Build the Program
```bash
mkdir build && cd build
cmake .. && make
```

### 3. Load Data
```bash
./data_loader --data_directory=../../data/0
```

### 4. Quick Test (All in One)
```bash
./docker-db.sh build-and-test
```

## How It Works
1. **Read Files**: Loads points.txt, categories.txt, and groups.txt from data directory
2. **Database Setup**: Creates tables and indexes in PostgreSQL using Docker
3. **Data Insert**: Stores all points with their categories and group information
4. **Validation**: Verifies data consistency and creates performance indexes

This solution prepares the database for spatial queries in later tasks.