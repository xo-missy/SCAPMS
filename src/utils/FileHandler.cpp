
#include "FileHandler.h"
#include "Exceptions.h"
#include <fstream>
 
namespace scapms {
namespace utils {
 
// Every field is written as "..." with " doubled, \ written as \\ and line breaks written as \n / \r,
// so the text never contains a raw comma-breaking quote or a real newline. parseLine() undoes exactly these
// rules, so parseLine(escapeField(s)) == {s} for any string s.
std::string FileHandler::escapeField(const std::string& field) {
    std::string out = "\"";
    for (char c : field) {
        if (c == '"') out += "\"\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    out += '"';
    return out;
}
 
Row FileHandler::parseLine(const std::string& line) {
    Row fields;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { cur += '"'; ++i; }
                else inQuotes = false;
            } else if (c == '\\' && i + 1 < line.size() &&
                       (line[i + 1] == 'n' || line[i + 1] == 'r' || line[i + 1] == '\\')) {
                char n = line[++i];
                cur += (n == 'n') ? '\n' : (n == 'r') ? '\r' : '\\';
            } else cur += c;
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(cur);
            cur.clear();
        } else cur += c;
    }
    fields.push_back(cur);
    return fields;
}
 
void FileHandler::writeCsv(const std::string& path, const std::vector<Row>& rows) {
    std::ofstream out(path.c_str());
    if (!out.is_open()) throw FileException("Cannot open file for writing: " + path);
    for (const Row& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i) out << ',';
            out << escapeField(row[i]);
        }
        out << '\n';
    }
    out.flush();
    if (!out) throw FileException("Failed while writing file: " + path);
}
 
std::vector<Row> FileHandler::readCsv(const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in.is_open()) throw FileException("Cannot open file for reading: " + path);
    std::vector<Row> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        rows.push_back(parseLine(line));
    }
    return rows;
}
 
} // namespace utils
} // namespace scapms
