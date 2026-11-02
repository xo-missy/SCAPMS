#include "DateUtils.h"
#include <cctype>
#include <cstdio>
#include <ctime>

namespace scapms {
namespace utils {
namespace DateUtils {

namespace {
bool digits(const std::string& s, size_t from, size_t count) {
    if (from + count > s.size()) return false;
    for (size_t i = from; i < from + count; ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}
}

bool isValidDate(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    if (!digits(d, 0, 4) || !digits(d, 5, 2) || !digits(d, 8, 2)) return false;
    int y = std::stoi(d.substr(0, 4)), m = std::stoi(d.substr(5, 2)), day = std::stoi(d.substr(8, 2));
    if (y < 1900 || y > 2200 || m < 1 || m > 12 || day < 1) return false;
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = dim[m - 1];
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (m == 2 && leap) maxDay = 29;
    return day <= maxDay;
}

bool isValidTime(const std::string& t) {
    if (t.size() != 5 || t[2] != ':' || !digits(t, 0, 2) || !digits(t, 3, 2)) return false;
    int h = std::stoi(t.substr(0, 2)), m = std::stoi(t.substr(3, 2));
    return h >= 0 && h <= 23 && m >= 0 && m <= 59;
}

std::string today() {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char buf[64];
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
    return buf;
}

std::string nowTime() {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%02d:%02d", lt->tm_hour, lt->tm_min);
    return buf;
}

long minutesFromNow(const std::string& date, const std::string& time) {
    std::tm tm = {};
    tm.tm_year = std::stoi(date.substr(0, 4)) - 1900;
    tm.tm_mon = std::stoi(date.substr(5, 2)) - 1;
    tm.tm_mday = std::stoi(date.substr(8, 2));
    tm.tm_hour = std::stoi(time.substr(0, 2));
    tm.tm_min = std::stoi(time.substr(3, 2));
    tm.tm_isdst = -1;
    std::time_t then = std::mktime(&tm);
    double diff = std::difftime(then, std::time(nullptr));
    return static_cast<long>(diff / 60.0);
}

} // namespace DateUtils
} // namespace utils
} // namespace scapms
