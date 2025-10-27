#!/bin/bash

# Docker PostgreSQL Management Script
# This script helps manage the PostgreSQL container for the Inspection Region System

set -e

CONTAINER_NAME="inspection_postgres"
DB_NAME="inspection_db"
DB_USER="inspection_user"
DB_PASS="inspection_pass"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 {start|stop|restart|status|logs|connect|reset|schema|clear|build-and-test}"
    echo ""
    echo "Commands:"
    echo "  start           Start PostgreSQL container"
    echo "  stop            Stop PostgreSQL container"
    echo "  restart         Restart PostgreSQL container"
    echo "  status          Show container status"
    echo "  logs            Show container logs"
    echo "  connect         Connect to PostgreSQL with psql"
    echo "  reset           Stop, remove container and volumes, then start fresh"
    echo "  schema          Create/recreate database schema and indexes"
    echo "  clear           Clear all data from tables (preserve schema)"
    echo "  build-and-test  Start database, build project, and run tests"
}

start_database() {
    echo -e "${BLUE}Starting PostgreSQL container...${NC}"
    
    if docker-compose up -d; then
        echo -e "${GREEN}✓ PostgreSQL container started${NC}"
        
        # Wait for database to be ready
        echo -e "${YELLOW}Waiting for database to be ready...${NC}"
        timeout=30
        while [ $timeout -gt 0 ]; do
            if docker-compose exec postgres pg_isready -U $DB_USER -d $DB_NAME >/dev/null 2>&1; then
                echo -e "${GREEN}✓ Database is ready${NC}"
                return 0
            fi
            sleep 1
            timeout=$((timeout - 1))
        done
        
        echo -e "${RED}❌ Database failed to start within 30 seconds${NC}"
        return 1
    else
        echo -e "${RED}❌ Failed to start PostgreSQL container${NC}"
        return 1
    fi
}

stop_database() {
    echo -e "${BLUE}Stopping PostgreSQL container...${NC}"
    if docker-compose down; then
        echo -e "${GREEN}✓ PostgreSQL container stopped${NC}"
    else
        echo -e "${RED}❌ Failed to stop PostgreSQL container${NC}"
        return 1
    fi
}

restart_database() {
    echo -e "${BLUE}Restarting PostgreSQL container...${NC}"
    stop_database
    start_database
}

show_status() {
    echo -e "${BLUE}PostgreSQL Container Status:${NC}"
    docker-compose ps
    echo ""
    
    if docker-compose exec postgres pg_isready -U $DB_USER -d $DB_NAME >/dev/null 2>&1; then
        echo -e "${GREEN}✓ Database is accessible${NC}"
        
        # Show database info
        echo -e "${BLUE}Database Information:${NC}"
        docker-compose exec postgres psql -U $DB_USER -d $DB_NAME -c "
            SELECT 
                current_database() as database,
                current_user as user,
                version() as postgresql_version;
        " 2>/dev/null || echo -e "${YELLOW}Could not fetch database info${NC}"
    else
        echo -e "${RED}❌ Database is not accessible${NC}"
    fi
}

show_logs() {
    echo -e "${BLUE}PostgreSQL Container Logs:${NC}"
    docker-compose logs -f postgres
}

connect_database() {
    echo -e "${BLUE}Connecting to PostgreSQL...${NC}"
    echo "Connection details:"
    echo "  Database: $DB_NAME"
    echo "  User: $DB_USER"
    echo "  Password: $DB_PASS"
    echo ""
    docker-compose exec postgres psql -U $DB_USER -d $DB_NAME
}

reset_database() {
    echo -e "${YELLOW}⚠️  This will destroy all data in the database!${NC}"
    read -p "Are you sure you want to reset? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${BLUE}Resetting PostgreSQL container and data...${NC}"
        docker-compose down -v  # Remove volumes too
        docker-compose up -d
        
        echo -e "${YELLOW}Waiting for database to initialize...${NC}"
        sleep 5
        
        if start_database; then
            echo -e "${GREEN}✓ Database reset complete${NC}"
        else
            echo -e "${RED}❌ Database reset failed${NC}"
            return 1
        fi
    else
        echo "Reset cancelled."
    fi
}

build_and_test() {
    echo -e "${BLUE}=== Full Build and Test Pipeline ===${NC}"
    
    # Start database
    if ! start_database; then
        echo -e "${RED}❌ Failed to start database${NC}"
        return 1
    fi
    
    # Build project
    echo -e "${BLUE}Building project...${NC}"
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    if cmake .. && make -j$(nproc 2>/dev/null || echo 4); then
        echo -e "${GREEN}✓ Build completed${NC}"
    else
        echo -e "${RED}❌ Build failed${NC}"
        cd ..
        return 1
    fi
    
    # Test with sample data
    echo -e "${BLUE}Testing with sample datasets...${NC}"
    
    # Test dataset 0
    echo -e "${YELLOW}Testing dataset 0...${NC}"
    if ./data_loader --data_directory=../../data/0 \
       --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"; then
        echo -e "${GREEN}✓ Dataset 0 test passed${NC}"
    else
        echo -e "${RED}❌ Dataset 0 test failed${NC}"
        cd ..
        return 1
    fi
    
    sleep 2
    
    # Test dataset 1
    echo -e "${YELLOW}Testing dataset 1...${NC}"
    if ./data_loader --data_directory=../../data/1 \
       --database="postgresql://inspection_user:inspection_pass@localhost:5432/inspection_db"; then
        echo -e "${GREEN}✓ Dataset 1 test passed${NC}"
    else
        echo -e "${RED}❌ Dataset 1 test failed${NC}"
        cd ..
        return 1
    fi
    
    cd ..
    
    echo ""
    echo -e "${GREEN}🎉 All tests passed! Task 1 is working correctly with Docker PostgreSQL.${NC}"
    echo ""
    echo -e "${BLUE}Next steps:${NC}"
    echo "1. Connect to database: $0 connect"
    echo "2. View data: SELECT COUNT(*) FROM inspection_group; SELECT COUNT(*) FROM inspection_region;"
    echo "3. Implement Task 2 (spatial queries)"
}

create_schema() {
    echo -e "${BLUE}Creating database schema...${NC}"
    
    if docker-compose exec postgres psql -U $DB_USER -d $DB_NAME -f /schema.sql; then
        echo -e "${GREEN}✓ Database schema created successfully${NC}"
    else
        echo -e "${RED}❌ Failed to create database schema${NC}"
        return 1
    fi
}

clear_data() {
    echo -e "${BLUE}Clearing database data...${NC}"
    
    if docker-compose exec postgres psql -U $DB_USER -d $DB_NAME -f /clear_data.sql; then
        echo -e "${GREEN}✓ Database data cleared successfully${NC}"
    else
        echo -e "${RED}❌ Failed to clear database data${NC}"
        return 1
    fi
}

# Main script logic
case "$1" in
    start)
        start_database
        ;;
    stop)
        stop_database
        ;;
    restart)
        restart_database
        ;;
    status)
        show_status
        ;;
    logs)
        show_logs
        ;;
    connect)
        connect_database
        ;;
    reset)
        reset_database
        ;;
    build-and-test)
        build_and_test
        ;;
    schema)
        create_schema
        ;;
    clear)
        clear_data
        ;;
    *)
        print_usage
        exit 1
        ;;
esac