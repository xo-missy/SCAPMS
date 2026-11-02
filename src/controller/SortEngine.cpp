#include "SortEngine.h"
#include <cctype>
#include <utility>

namespace scapms {
namespace controller {

using model::PatientRecord;

std::string SortEngine::toLower(const std::string& s) {
    std::string r = s;
    for (size_t i = 0; i < r.size(); ++i) r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    return r;
}

// ---------------- Selection sort: O(n^2) comparisons, at most n-1 swaps ----------------
void SortEngine::selectionSort(PatientList& v, const Comparator& comp) {
    for (size_t i = 0; i + 1 < v.size(); ++i) {
        size_t best = i;
        for (size_t j = i + 1; j < v.size(); ++j)
            if (comp(v[j], v[best])) best = j;
        if (best != i) std::swap(v[i], v[best]);
    }
}

// ---------------- Insertion sort: O(n^2) worst, O(n) on nearly-sorted data, stable ----------------
void SortEngine::insertionSort(PatientList& v, const Comparator& comp) {
    for (size_t i = 1; i < v.size(); ++i) {
        PatientRecord* key = v[i];
        size_t j = i;
        while (j > 0 && comp(key, v[j - 1])) {
            v[j] = v[j - 1];
            --j;
        }
        v[j] = key;
    }
}

// ---------------- Merge sort: O(n log n) always, stable, recursive ----------------
void SortEngine::mergeSortRange(PatientList& v, PatientList& tmp, size_t lo, size_t hi, const Comparator& comp) {
    if (hi - lo < 2) return;                       // range [lo, hi)
    size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(v, tmp, lo, mid, comp);
    mergeSortRange(v, tmp, mid, hi, comp);
    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (comp(v[j], v[i])) tmp[k++] = v[j++];   // only take right if strictly smaller -> stable
        else tmp[k++] = v[i++];
    }
    while (i < mid) tmp[k++] = v[i++];
    while (j < hi) tmp[k++] = v[j++];
    for (size_t x = lo; x < hi; ++x) v[x] = tmp[x];
}

void SortEngine::mergeSort(PatientList& v, const Comparator& comp) {
    PatientList tmp(v.size());
    mergeSortRange(v, tmp, 0, v.size(), comp);
}

// ---------------- Quick sort: O(n log n) average, in place, recursive ----------------
void SortEngine::quickSortRange(PatientList& v, long lo, long hi, const Comparator& comp) {
    while (lo < hi) {
        long mid = lo + (hi - lo) / 2;
        std::swap(v[static_cast<size_t>(mid)], v[static_cast<size_t>(hi)]);   // middle element as pivot
        PatientRecord* pivot = v[static_cast<size_t>(hi)];
        long store = lo;
        for (long i = lo; i < hi; ++i)
            if (comp(v[static_cast<size_t>(i)], pivot)) {
                std::swap(v[static_cast<size_t>(i)], v[static_cast<size_t>(store)]);
                ++store;
            }
        std::swap(v[static_cast<size_t>(store)], v[static_cast<size_t>(hi)]);
        // recurse into the smaller half, loop on the larger one (bounded stack depth)
        if (store - lo < hi - store) {
            quickSortRange(v, lo, store - 1, comp);
            lo = store + 1;
        } else {
            quickSortRange(v, store + 1, hi, comp);
            hi = store - 1;
        }
    }
}

void SortEngine::quickSort(PatientList& v, const Comparator& comp) {
    if (v.size() > 1) quickSortRange(v, 0, static_cast<long>(v.size()) - 1, comp);
}

// ---------------- helpers ----------------
Comparator SortEngine::comparatorFor(SortKey key) {
    switch (key) {
    case SortKey::Name:
        return [](const PatientRecord* a, const PatientRecord* b) {
            std::string x = toLower(a->name()), y = toLower(b->name());
            if (x != y) return x < y;
            return a->id() < b->id();
        };
    case SortKey::Id:
        return [](const PatientRecord* a, const PatientRecord* b) { return a->id() < b->id(); };
    case SortKey::Age:
        return [](const PatientRecord* a, const PatientRecord* b) {
            if (a->age() != b->age()) return a->age() < b->age();
            return a->id() < b->id();
        };
    case SortKey::AppointmentDate:
        return [](const PatientRecord* a, const PatientRecord* b) {
            std::string x = a->earliestPendingDate(), y = b->earliestPendingDate();
            if (x != y) return x < y;
            return a->id() < b->id();
        };
    }
    return [](const PatientRecord* a, const PatientRecord* b) { return a->id() < b->id(); };
}

void SortEngine::sort(PatientList& v, SortAlgorithm algo, SortKey key) {
    Comparator comp = comparatorFor(key);
    switch (algo) {
    case SortAlgorithm::Selection: selectionSort(v, comp); break;
    case SortAlgorithm::Insertion: insertionSort(v, comp); break;
    case SortAlgorithm::Merge: mergeSort(v, comp); break;
    case SortAlgorithm::Quick: quickSort(v, comp); break;
    }
}

std::string SortEngine::algorithmName(SortAlgorithm a) {
    switch (a) {
    case SortAlgorithm::Selection: return "Selection Sort";
    case SortAlgorithm::Insertion: return "Insertion Sort";
    case SortAlgorithm::Merge: return "Merge Sort";
    case SortAlgorithm::Quick: return "Quick Sort";
    }
    return "";
}

std::string SortEngine::keyName(SortKey k) {
    switch (k) {
    case SortKey::Name: return "Patient Name";
    case SortKey::Id: return "Patient ID";
    case SortKey::Age: return "Age";
    case SortKey::AppointmentDate: return "Appointment Date";
    }
    return "";
}

std::string SortEngine::explain(SortAlgorithm a, size_t n) {
    std::string size = std::to_string(n) + " record" + (n == 1 ? "" : "s");
    switch (a) {
    case SortAlgorithm::Selection:
        return "Selection Sort (" + size + "): simple and makes the fewest swaps (at most n-1), which suits small "
               "clinic lists, but it always does O(n^2) comparisons so it slows down on large lists.";
    case SortAlgorithm::Insertion:
        return "Insertion Sort (" + size + "): very fast on small or nearly-sorted lists (best case O(n)) and stable - "
               "ideal when a few new patients were just added to an already sorted list.";
    case SortAlgorithm::Merge:
        return "Merge Sort (" + size + "): guaranteed O(n log n) and stable (patients with equal keys keep their order), "
               "so it is the safest choice for large or unpredictable data, at the cost of extra memory.";
    case SortAlgorithm::Quick:
        return "Quick Sort (" + size + "): O(n log n) on average and sorts in place with little extra memory, so it is "
               "very fast for large lists; it is not stable and its worst case is O(n^2) (reduced here by a middle pivot).";
    }
    return "";
}

} // namespace controller
} // namespace scapms
