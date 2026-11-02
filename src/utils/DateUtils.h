#pragma once
// Small date/time helpers (dates are "YYYY-MM-DD", times are "HH:MM").
#include <string>

namespace scapms {
namespace utils {
namespace DateUtils {

bool isValidDate(const std::string& date);
bool isValidTime(const std::string& time);
std::string today();                       // current local date
std::string nowTime();                     // current local time
// Minutes from now until the given moment (negative if it is in the past).
long minutesFromNow(const std::string& date, const std::string& time);

} // namespace DateUtils
} // namespace utils
} // namespace scapms
