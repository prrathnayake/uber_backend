#pragma once

#include <memory>
#include <string>
#include <vector>
#include <h3/h3api.h>

#include <utils/index.h>
#include <h3/index.h>

using namespace utils;
using namespace h3;

namespace UberBackend {

    class H3Handler {
    public:
        H3Handler();
        ~H3Handler();

        std::string getH3Indexes(double lat, double lng, int resolution = 8);
        std::vector<std::string> getNearestDriversList(const std::string& h3IndexStr, int kRing);

    private:
        SingletonLogger &logger_;
        H3 h3_;
    };

}
