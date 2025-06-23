#include <iostream>

#include <database/index.h>

#include "database/database.h"

int main()
{
    uber_backend::uber_database *database = new uber_backend::uber_database();
    database->initalizeDatabase();
    return 0;
};