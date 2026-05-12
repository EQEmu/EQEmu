#!/usr/bin/env bash

# Variables
ROOT_PASSWORD="eqemu"
MARIADB_CONFIG="/etc/mysql/mariadb.conf.d/50-server.cnf"

# Update and install MariaDB
echo "Installing MariaDB..."
sudo apt update
sudo apt install -y mariadb-server mariadb-client

# Ensure MariaDB is stopped before configuration
echo "Stopping MariaDB service..."
sudo systemctl stop mariadb

# Initialize the data directory (in case it's not already initialized)
echo "Initializing MariaDB data directory..."
sudo mysqld --initialize --user=mysql --datadir=/var/lib/mysql

# Start MariaDB in safe mode
echo "Starting MariaDB in safe mode..."
sudo mysqld_safe --skip-grant-tables --skip-networking &
sleep 5

# Reset root password and configure authentication
echo "Resetting root password and configuring authentication..."
mariadb <<EOF
FLUSH PRIVILEGES;
ALTER USER 'root'@'localhost' IDENTIFIED VIA mysql_native_password USING PASSWORD('$ROOT_PASSWORD');
GRANT ALL PRIVILEGES ON *.* TO 'root'@'localhost' IDENTIFIED BY '$ROOT_PASSWORD' WITH GRANT OPTION;
FLUSH PRIVILEGES;
EOF

# Stop MariaDB safe mode
echo "Stopping MariaDB safe mode..."
sudo killall mysqld

# Configure MariaDB to allow remote connections (optional)
echo "Configuring MariaDB to allow remote connections..."
sudo sed -i "s/^bind-address.*/bind-address = 0.0.0.0/" $MARIADB_CONFIG

# Restart MariaDB service
echo "Restarting MariaDB service..."
sudo systemctl restart mariadb

# Test connection
echo "Testing MariaDB connection..."
mysql -u root -p"$ROOT_PASSWORD" -e "SELECT VERSION();"

# Display completion message
echo "MariaDB setup completed!"
echo "Root password: $ROOT_PASSWORD"

# Set Database Credentials
DB_USER="root"
DB_PASS="eqemu"
DB_HOST="localhost"
DB_NAME="peq"
SQL_DIR="/tmp/db/peq-dump"

# Download the latest database dump
echo "Downloading the latest PEQ database dump..."
curl -s http://db.projecteq.net/api/v1/dump/latest -o /tmp/db.zip

# Unzip the database dump
echo "Extracting the database dump..."
unzip -o /tmp/db.zip -d /tmp/db/

# Ensure MariaDB is running
echo "Ensuring MariaDB is running..."
sudo systemctl start mariadb

# Wait for MariaDB to be ready
echo "Waiting for MariaDB to be ready..."
while ! mysqladmin ping -u${DB_USER} -p${DB_PASS} -h${DB_HOST} --silent; do
    sleep 1
done

# Create the peq database
echo "Creating the '${DB_NAME}' database..."
mysql -u${DB_USER} -p${DB_PASS} -h${DB_HOST} -e "DROP DATABASE IF EXISTS ${DB_NAME}; CREATE DATABASE ${DB_NAME};"

# Parallelize the import process
echo "Importing tables in parallel..."
shopt -s nullglob
create_table_files=("${SQL_DIR}"/create_tables_*.sql)

if [ ${#create_table_files[@]} -gt 0 ]; then
	printf '%s\0' "${create_table_files[@]}" |
		xargs -0 -P 4 -I {} sh -c 'mysql -u"$1" -p"$2" -h"$3" "$4" < "$5"' _ "${DB_USER}" "${DB_PASS}" "${DB_HOST}" "${DB_NAME}" "{}"
fi

# Clean up temporary files
echo "Cleaning up temporary files..."
rm -rf /tmp/db/

echo "Database import complete!"

touch /tmp/import-done
