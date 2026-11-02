#include "GeneralPatient.h"

namespace scapms {
namespace model {

GeneralPatient::GeneralPatient(int id, const std::string& name, int age, const std::string& contact,
                               const std::string& complaint)
    : PatientRecord(id, name, age, contact), complaint_(complaint) {}

std::string GeneralPatient::describe() const {
    return "General patient - routine consultation for: " + complaint_;
}

void GeneralPatient::setDetails(const std::vector<std::string>& values) {
    std::string c = values.empty() ? std::string() : trim(values[0]);
    complaint_ = c.empty() ? "General check-up" : c;
}

} // namespace model
} // namespace scapms
