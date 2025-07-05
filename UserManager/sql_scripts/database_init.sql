CREATE DATABASE IF NOT EXISTS userManagerDatabase;
USE userManagerDatabase;

CREATE TABLE IF NOT EXISTS users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    middle_name VARCHAR(50),
    last_name VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    username VARCHAR(50) UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    country_code VARCHAR(5),
    phone_number VARCHAR(20),
    role ENUM('rider', 'driver') NOT NULL,
    mobile_verified BOOLEAN NOT NULL DEFAULT FALSE,
    email_verified BOOLEAN NOT NULL DEFAULT FALSE,
    temp_verification_code VARCHAR(6),
    profile_picture_url VARCHAR(255),
    appwrite_url VARCHAR(255),
    preferred_language VARCHAR(10) DEFAULT 'en',
    currency VARCHAR(5),
    country VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS user_addresses (
    address_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    street VARCHAR(100),
    city VARCHAR(50),
    state_or_province VARCHAR(50),
    postal_code VARCHAR(20),
    country VARCHAR(100),
    latitude DECIMAL(10, 7),
    longitude DECIMAL(10, 7),
    is_primary BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS payment_details (
    payment_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    payment_type ENUM(
        'credit_card',
        'debit_card',
        'paypal',
        'apple_pay',
        'google_pay'
    ),
    card_number VARCHAR(20),
    expire_date DATE,
    provider VARCHAR(50),
    secret VARCHAR(100),
    is_default BOOLEAN NOT NULL DEFAULT FALSE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS vehicle_details (
    vehicle_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    vehicle_type VARCHAR(50),
    make VARCHAR(50),
    model VARCHAR(50),
    license_plate VARCHAR(20),
    registration_year INT,
    color VARCHAR(30),
    country VARCHAR(100),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS ride_history (
    ride_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    origin_address VARCHAR(200),
    origin_lat DECIMAL(10, 7),
    origin_lng DECIMAL(10, 7),
    destination_address VARCHAR(200),
    destination_lat DECIMAL(10, 7),
    destination_lng DECIMAL(10, 7),
    start_time DATETIME,
    end_time DATETIME,
    fare_amount DECIMAL(10, 2),
    currency VARCHAR(5),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS bank_details (
    bank_detail_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    bank_name VARCHAR(100) NOT NULL,
    account_holder_name VARCHAR(100) NOT NULL,
    account_number VARCHAR(30) NOT NULL,
    bsb_code VARCHAR(20),
    bank_country VARCHAR(100) NOT NULL,
    iban VARCHAR(34),
    swift_code VARCHAR(11),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);