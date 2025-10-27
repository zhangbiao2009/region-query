#!/bin/bash

# Quick Docker PostgreSQL Test
# This script tests that Docker PostgreSQL is working correctly

set -e

echo "🐳 Testing Docker PostgreSQL Setup"
echo "=================================="

# Check if Docker is running
if ! docker info >/dev/null 2>&1; then
    echo "❌ Docker is not running. Please start Docker Desktop."
    exit 1
fi

echo "✓ Docker is running"

# Start the database
echo "Starting PostgreSQL container..."
if ./docker-db.sh start; then
    echo "✓ PostgreSQL container started"
else
    echo "❌ Failed to start PostgreSQL container"
    exit 1
fi

# Test database connection
echo "Testing database connection..."
if docker-compose exec postgres psql -U inspection_user -d inspection_db -c "SELECT 'Database connection successful!' as status;" >/dev/null 2>&1; then
    echo "✓ Database connection successful"
else
    echo "❌ Database connection failed"
    exit 1
fi

# Show database info
echo ""
echo "Database Information:"
docker-compose exec postgres psql -U inspection_user -d inspection_db -c "
    SELECT 
        current_database() as database,
        current_user as \"user\",
        inet_server_addr() as host,
        inet_server_port() as port;
"

echo ""
echo "🎉 Docker PostgreSQL is working correctly!"
echo ""
echo "Next steps:"
echo "1. Build and test: ./docker-db.sh build-and-test"
echo "2. Or manual build: mkdir build && cd build && cmake .. && make"
echo "3. Connect to DB: ./docker-db.sh connect"