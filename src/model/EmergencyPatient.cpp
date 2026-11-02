#include "EmergencyPatient.h"
#include "../utils/Exceptions.h"

namespace scapms {
namespace model {

EmergencyPatient::EmergencyPatient(int id, const std::string& name, int age, const std::string& contact,
                                   int severity, const std::string& emergencyType)
    : PatientRecord(id, name, age, contact), severity_(3), emergencyType_(emergencyType) {
    setDetails({std::to_string(severity), emergencyType});
}

std::string EmergencyPatient::describe() const {
    return "EMERGENCY (severity " + std::to_string(severity_) + "/5) - " + emergencyType_ +
           " - must be seen immediately";
}

std::vector<std::string> EmergencyPatient::details() const {
    return {std::to_string(severity_), emergencyType_};
}

void EmergencyPatient::setDetails(const std::vector<std::string>& values) {
    int sev = 0;
    std::string s = values.empty() ? std::string() : trim(values[0]);
    try {
        size_t used = 0;
        sev = std::stoi(s, &used);
        if (used != s.size()) throw std::invalid_argument("trailing");
    } catch (const std::logic_error&) {
        throw utils::InvalidPatientException("Emergency severity must be a whole number from 1 to 5.");
    }
    if (sev < 1 || sev > 5)
        throw utils::InvalidPatientException("Emergency severity must be a whole number from 1 to 5.");
    std::string type = values.size() > 1 ? trim(values[1]) : std::string();
    severity_ = sev;
    emergencyType_ = type.empty() ? "Unspecified" : type;
}

int EmergencyPatient::scheduleAppointment(const std::string& date, const std::string& time,
                                          const std::string& reason) {
    const std::string tag = "[URGENT] ";
    std::string r = (reason.compare(0, tag.size(), tag) == 0) ? reason : tag + reason;
    return PatientRecord::scheduleAppointment(date, time, r);
}

} // namespace model
} // namespace scapms
