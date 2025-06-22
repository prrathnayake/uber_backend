#include <iostream>

#include <database/index.h>

int main()
{
    database::MySQLDatabase *database = new database::MySQLDatabase();

    const std::string host = "localhost";
    const std::string user = "pasan";
    const std::string password = "pasan";
    const std::string databaseName = "sampleDatabase";
    unsigned int port = 3306;

    database->connect(host, user, password, databaseName, port);

    delete database;
    return 0;
};