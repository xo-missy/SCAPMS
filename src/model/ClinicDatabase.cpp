#include "ClinicDatabase.h"
#include "../utils/Exceptions.h"
#include <map>

namespace scapms {
namespace model {

void ClinicDatabase::add(std::unique_ptr<PatientRecord> p) {
    insertAt(patients_.size(), std::move(p));
}

void ClinicDatabase::insertAt(size_t index, std::unique_ptr<PatientRecord> p) {
    if (!p) throw utils::InvalidPatientException("Cannot add an empty patient record.");
    if (contains(p->id())) throw utils::DuplicateIdException();
    if (index > patients_.size()) index = patients_.size();
    patients_.insert(patients_.begin() + static_cast<long>(index), std::move(p));
}

std::unique_ptr<PatientRecord> ClinicDatabase::removeById(int id, size_t* indexOut) {
    for (size_t i = 0; i < patients_.size(); ++i) {
        if (patients_[i]->id() == id) {
            std::unique_ptr<PatientRecord> p = std::move(patients_[i]);
            patients_.erase(patients_.begin() + static_cast<long>(i));
            if (indexOut) *indexOut = i;
            return p;
        }
    }
    throw utils::NotFoundException("No patient with ID " + std::to_string(id) + " exists.");
}

PatientRecord* ClinicDatabase::get(int id) const {
    for (size_t i = 0; i < patients_.size(); ++i)
        if (patients_[i]->id() == id) return patients_[i].get();
    return nullptr;
}

PatientRecord* ClinicDatabase::findByAppointment(int appointmentId) const {
    for (size_t i = 0; i < patients_.size(); ++i)
        for (size_t j = 0; j < patients_[i]->appointments().size(); ++j)
            if (patients_[i]->appointments()[j].id() == appointmentId) return patients_[i].get();
    return nullptr;
}

std::vector<PatientRecord*> ClinicDatabase::view() const {
    std::vector<PatientRecord*> v;
    v.reserve(patients_.size());
    for (size_t i = 0; i < patients_.size(); ++i) v.push_back(patients_[i].get());
    return v;
}

void ClinicDatabase::reorder(const std::vector<PatientRecord*>& order) {
    if (order.size() != patients_.size())
        throw utils::ClinicException("Internal error: reorder size mismatch.");
    std::map<PatientRecord*, std::unique_ptr<PatientRecord>> pool;
    for (size_t i = 0; i < patients_.size(); ++i) {
        PatientRecord* raw = patients_[i].get();
        pool[raw] = std::move(patients_[i]);
    }
    patients_.clear();
    for (size_t i = 0; i < order.size(); ++i) {
        std::map<PatientRecord*, std::unique_ptr<PatientRecord>>::iterator it = pool.find(order[i]);
        if (it == pool.end()) throw utils::ClinicException("Internal error: reorder mismatch.");
        patients_.push_back(std::move(it->second));
        pool.erase(it);
    }
}

} // namespace model
} // namespace scapms
