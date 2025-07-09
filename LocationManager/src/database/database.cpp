#include "../../include/database/database.h"
#include <filesystem>
#include <utils/index.h>

namespace fs = std::filesystem;

using namespace UberBackend;

uber_database::uber_database(const std::string &host,
                             const std::string &user,
                             const std::string &password,
                             const std::string &dbName,
                             unsigned int port)
    : SharedDatabase(host, user, password, dbName, port)
{
}