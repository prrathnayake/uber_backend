-- Create the database
CREATE DATABASE IF NOT EXISTS locationManagerDatabase;
USE locationManagerDatabase;

-- Users (Riders)
CREATE TABLE IF NOT EXISTS riders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20),
    password_hash VARCHAR(255) NOT NULL
);

-- Drivers
CREATE TABLE IF NOT EXISTS drivers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20),
    license_number VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    is_available BOOLEAN DEFAULT TRUE
);

-- Vehicles
CREATE TABLE IF NOT EXISTS vehicles (
    id INT AUTO_INCREMENT PRIMARY KEY,
    driver_id INT NOT NULL,
    make VARCHAR(50),
    model VARCHAR(50),
    plate_number VARCHAR(20) NOT NULL UNIQUE,
    color VARCHAR(30),
    year INT CHECK (year >= 1900), -- Removed dynamic check
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
);

-- Rides
CREATE TABLE IF NOT EXISTS rides (
    id INT AUTO_INCREMENT PRIMARY KEY,
    rider_id INT NOT NULL,
    driver_id INT,
    pickup_location VARCHAR(255) NOT NULL,
    dropoff_location VARCHAR(255) NOT NULL,
    start_time DATETIME,
    end_time DATETIME,
    status ENUM('requested', 'ongoing', 'completed', 'cancelled') DEFAULT 'requested',
    fare DECIMAL(10, 2),
    FOREIGN KEY (rider_id) REFERENCES riders(id) ON DELETE CASCADE,
    FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE SET NULL
);

-- Payments
CREATE TABLE IF NOT EXISTS payments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    ride_id INT NOT NULL,
    amount DECIMAL(10, 2) NOT NULL CHECK (amount >= 0),
    method ENUM('card', 'cash', 'wallet') NOT NULL,
    status ENUM('pending', 'completed', 'failed') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (ride_id) REFERENCES rides(id) ON DELETE CASCADE
);

-- Locations (saved places or live tracking)
CREATE TABLE IF NOT EXISTS locations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    rider_id INT,
    name VARCHAR(100), -- e.g., Home, Work
    latitude DECIMAL(9,6),
    longitude DECIMAL(9,6),
    FOREIGN KEY (rider_id) REFERENCES riders(id) ON DELETE CASCADE
);
