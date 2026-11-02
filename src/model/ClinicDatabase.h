#pragma once
#include <memory>
#include <vector>
#include "PatientRecord.h"

namespace scapms {
namespace model {

// Owns every registered patient (std::vector of polymorphic records).
class ClinicDatabase {
public:
    // Throws DuplicateIdException if the id is already registered.
    void add(std::unique_ptr<PatientRecord> p);
    void insertAt(size_t index, std::unique_ptr<PatientRecord> p);
    // Removes and returns the patient (so it can be restored by Undo). Throws NotFoundException.
    std::unique_ptr<PatientRecord> removeById(int id, size_t* indexOut = nullptr);

    PatientRecord* get(int id) const;           // nullptr when absent
    bool contains(int id) const { return get(id) != nullptr; }
    PatientRecord* findByAppointment(int appointmentId) const;

    size_t size() const { return patients_.size(); }
    bool empty() const { return patients_.empty(); }
    void clear() { patients_.clear(); }

    // Non-owning snapshot of the patients in their current order.
    std::vector<PatientRecord*> view() const;
    // Re-order the stored patients to match 'order' (same set of pointers, new sequence).
    void reorder(const std::vector<PatientRecord*>& order);

private:
    std::vector<std::unique_ptr<PatientRecord>> patients_;
};

} // namespace model
} // namespace scapms
