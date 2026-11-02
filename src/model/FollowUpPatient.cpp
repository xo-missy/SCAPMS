#include "FollowUpPatient.h"
#include "../utils/DateUtils.h"
#include "../utils/Exceptions.h"

namespace scapms {
namespace model {

FollowUpPatient::FollowUpPatient(int id, const std::string& name, int age, const std::string& contact,
                                 const std::string& previousVisit, const std::string& doctor)
    : PatientRecord(id, name, age, contact) {
    setDetails({previousVisit, doctor});
}

std::string FollowUpPatient::describe() const {
    return "Follow-up patient - last visit " + (previousVisit_.empty() ? std::string("n/a") : previousVisit_) +
           " with " + doctor_;
}

void FollowUpPatient::setDetails(const std::vector<std::string>& values) {
    std::string prev = values.empty() ? std::string() : trim(values[0]);
    if (!prev.empty() && !utils::DateUtils::isValidDate(prev))
        throw utils::InvalidPatientException("Previous visit must be a valid date (YYYY-MM-DD) or left blank.");
    std::string doc = values.size() > 1 ? trim(values[1]) : std::string();
    previousVisit_ = prev;
    doctor_ = doc.empty() ? "Unassigned" : doc;
}

int FollowUpPatient::scheduleAppointment(const std::string& date, const std::string& time,
                                         const std::string& reason) {
    const std::string tag = "Follow-up: ";
    std::string r = (reason.compare(0, tag.size(), tag) == 0) ? reason : tag + reason;
    return PatientRecord::scheduleAppointment(date, time, r);
}

} // namespace model
} // namespace scapms
