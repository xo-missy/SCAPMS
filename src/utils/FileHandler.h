#pragma once
// Plain-text (CSV) persistence helpers. Knows nothing about patients.
#include <string>
#include <vector>

namespace scapms {
namespace utils {

using Row = std::vector<std::string>;

class FileHandler {
public:
    // Throws FileException if the file cannot be written.
    static void writeCsv(const std::string& path, const std::vector<Row>& rows);
    // Throws FileException if the file cannot be opened / read.
    static std::vector<Row> readCsv(const std::string& path);

    static std::string escapeField(const std::string& field);
    static Row parseLine(const std::string& line);
};

} // namespace utils
} // namespace scapms
