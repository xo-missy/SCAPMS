#pragma once
// Hand-written sorting algorithms (no std::sort / std::stable_sort).
#include <functional>
#include <string>
#include <vector>
#include "../model/PatientRecord.h"

namespace scapms {
namespace controller {

enum class SortAlgorithm { Selection, Insertion, Merge, Quick };
enum class SortKey { Name, Id, Age, AppointmentDate };

// comp(a, b) == true  <=>  a must be placed strictly before b.
using Comparator = std::function<bool(const model::PatientRecord*, const model::PatientRecord*)>;
using PatientList = std::vector<model::PatientRecord*>;

class SortEngine {
public:
    static void selectionSort(PatientList& v, const Comparator& comp);
    static void insertionSort(PatientList& v, const Comparator& comp);
    static void mergeSort(PatientList& v, const Comparator& comp);      // recursive
    static void quickSort(PatientList& v, const Comparator& comp);      // recursive

    static void sort(PatientList& v, SortAlgorithm algo, SortKey key);
    static Comparator comparatorFor(SortKey key);

    static std::string algorithmName(SortAlgorithm a);
    static std::string keyName(SortKey k);
    // Plain-English reason why this algorithm suits a list of 'n' records.
    static std::string explain(SortAlgorithm a, size_t n);

    static std::string toLower(const std::string& s);

private:
    static void mergeSortRange(PatientList& v, PatientList& tmp, size_t lo, size_t hi, const Comparator& comp);
    static void quickSortRange(PatientList& v, long lo, long hi, const Comparator& comp);
};

} // namespace controller
} // namespace scapms
