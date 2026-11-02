#include "SearchEngine.h"

namespace scapms {
namespace controller {

using model::PatientRecord;

bool SearchEngine::containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    std::string h = SortEngine::toLower(haystack), n = SortEngine::toLower(needle);
    if (n.size() > h.size()) return false;
    for (size_t i = 0; i + n.size() <= h.size(); ++i) {
        size_t k = 0;
        while (k < n.size() && h[i + k] == n[k]) ++k;
        if (k == n.size()) return true;
    }
    return false;
}

PatientRecord* SearchEngine::linearSearchById(const PatientList& v, int id) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i]->id() == id) return v[i];
    return nullptr;
}

PatientRecord* SearchEngine::binarySearchById(const PatientList& v, int id) {
    long lo = 0, hi = static_cast<long>(v.size()) - 1;
    while (lo <= hi) {
        long mid = lo + (hi - lo) / 2;
        int midId = v[static_cast<size_t>(mid)]->id();
        if (midId == id) return v[static_cast<size_t>(mid)];
        if (midId < id) lo = mid + 1;
        else hi = mid - 1;
    }
    return nullptr;
}

void SearchEngine::recursiveSearchByName(const PatientList& v, const std::string& keyword, size_t index,
                                         PatientList& out) {
    if (index >= v.size()) return;                       // base case
    if (containsIgnoreCase(v[index]->name(), keyword)) out.push_back(v[index]);
    recursiveSearchByName(v, keyword, index + 1, out);   // recursive case
}

PatientList SearchEngine::linearSearchByName(const PatientList& v, const std::string& keyword) {
    PatientList out;
    for (size_t i = 0; i < v.size(); ++i)
        if (containsIgnoreCase(v[i]->name(), keyword)) out.push_back(v[i]);
    return out;
}

PatientList SearchEngine::linearSearchByCategory(const PatientList& v, model::Category c) {
    PatientList out;
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i]->category() == c) out.push_back(v[i]);
    return out;
}

} // namespace controller
} // namespace scapms
