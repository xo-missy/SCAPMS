#pragma once
// Hand-written searching algorithms (no std::find / std::binary_search).
#include <string>
#include <vector>
#include "SortEngine.h"

namespace scapms {
namespace controller {

class SearchEngine {
public:
    // Works on any list. O(n).
    static model::PatientRecord* linearSearchById(const PatientList& v, int id);
    // REQUIRES the list to be sorted ascending by id. O(log n).
    static model::PatientRecord* binarySearchById(const PatientList& v, int id);
    // Recursive substring search (case-insensitive) over the name field.
    static void recursiveSearchByName(const PatientList& v, const std::string& keyword, size_t index,
                                      PatientList& out);
    // Iterative substring search over names (used when the user explicitly asks for a linear scan).
    static PatientList linearSearchByName(const PatientList& v, const std::string& keyword);
    static PatientList linearSearchByCategory(const PatientList& v, model::Category c);

    static bool containsIgnoreCase(const std::string& haystack, const std::string& needle);
};

} // namespace controller
} // namespace scapms
