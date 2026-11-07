#pragma once
#include <QString>
#include <optional>

namespace vm::geo {

struct GeoFix {
    double lat = 0.0;
    double lng = 0.0;
    QString country_code;
    QString source;
};

std::optional<GeoFix> locate_by_ip();
QString reverse_country(double lat, double lng);

} // namespace vm::geo
