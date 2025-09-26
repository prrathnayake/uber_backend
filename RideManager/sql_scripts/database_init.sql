-- Create the database
CREATE DATABASE IF NOT EXISTS uber_database;

USE uber_database;

-- Users (Riders)
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20),
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Drivers
CREATE TABLE IF NOT EXISTS drivers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20),
    license_number VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    is_available BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Vehicles
CREATE TABLE IF NOT EXISTS vehicles (
    id INT AUTO_INCREMENT PRIMARY KEY,
    driver_id INT NOT NULL,
    make VARCHAR(50),
    model VARCHAR(50),
    plate_number VARCHAR(20) UNIQUE NOT NULL,
    color VARCHAR(30),
    year INT,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Rides
CREATE TABLE IF NOT EXISTS rides (
    id INT AUTO_INCREMENT PRIMARY KEY,
    ride_identifier VARCHAR(64) NOT NULL UNIQUE,
    user_id INT NOT NULL,
    driver_id INT,
    pickup_location VARCHAR(255) NOT NULL,
    dropoff_location VARCHAR(255) NOT NULL,
    pickup_lat DECIMAL(10, 6) DEFAULT 0,
    pickup_lng DECIMAL(10, 6) DEFAULT 0,
    dropoff_lat DECIMAL(10, 6) DEFAULT 0,
    dropoff_lng DECIMAL(10, 6) DEFAULT 0,
    requested_at DATETIME,
    updated_at DATETIME,
    status ENUM('requested', 'pending_driver', 'accepted', 'in_progress', 'completed', 'cancelled') DEFAULT 'requested',
    status_reason VARCHAR(255) DEFAULT NULL,
    fare DECIMAL(10, 2),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (driver_id) REFERENCES drivers(id)
);

-- Payments
CREATE TABLE IF NOT EXISTS payments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    ride_id INT NOT NULL,
    amount DECIMAL(10, 2) NOT NULL,
    method ENUM('card', 'cash', 'wallet') NOT NULL,
    status ENUM('pending', 'completed', 'failed') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (ride_id) REFERENCES rides(id) ON DELETE CASCADE
);

-- Locations (optional live tracking or saved places)
CREATE TABLE IF NOT EXISTS locations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    name VARCHAR(100), -- e.g., Home, Work
    latitude DECIMAL(9,6),
    longitude DECIMAL(9,6),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);
