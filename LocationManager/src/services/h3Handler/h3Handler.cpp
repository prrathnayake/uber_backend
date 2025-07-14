#include "../../../include/services/h3Handler/h3Handler.h"

using namespace UberBackend;

H3Handler::H3Handler()
    : logger_(SingletonLogger::instance()), h3_() {}

H3Handler::~H3Handler() {}

std::string H3Handler::getH3Indexes(double lat, double lng, int resolution)
{
    auto index = h3_.getH3Index(lat, lng, resolution);
    return h3_.toString(index);
}

std::vector<std::string> H3Handler::getNearestDriversList(const std::string &h3IndexStr, int kRing)
{
    auto origin = h3_.fromString(h3IndexStr);
    if (!h3_.isValid(origin))
    {
        logger_.logMeta(SingletonLogger::ERROR, "invalid H3 index string : " + h3IndexStr, __FILE__, __LINE__, __func__);

        return {};
    }

    std::vector<H3Index> neighbors = h3_.getNeighbors(origin, kRing);
    std::vector<std::string> result;

    for (const auto &h3Index : neighbors)
    {
        result.push_back(h3_.toString(h3Index));
    }

    return result;
}
