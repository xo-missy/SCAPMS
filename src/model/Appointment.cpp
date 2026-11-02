#include "Appointment.h"
#include "../utils/DateUtils.h"
#include "../utils/Exceptions.h"

namespace scapms {
namespace model {

Appointment::Appointment(int id, int patientId, const std::string& date, const std::string& time,
                         const std::string& reason, AppointmentStatus status)
    : id_(id), patientId_(patientId), date_(date), time_(time),
      reason_(reason.empty() ? "Consultation" : reason), status_(status) {
    if (!utils::DateUtils::isValidDate(date))
        throw utils::InvalidAppointmentException("Appointment date must be a valid date in the form YYYY-MM-DD.");
    if (!utils::DateUtils::isValidTime(time))
        throw utils::InvalidAppointmentException("Appointment time must be a valid time in the form HH:MM (24-hour).");
}

std::string Appointment::statusToString(AppointmentStatus s) {
    switch (s) {
    case AppointmentStatus::Pending: return "Pending";
    case AppointmentStatus::Completed: return "Completed";
    case AppointmentStatus::Cancelled: return "Cancelled";
    }
    return "Pending";
}

AppointmentStatus Appointment::statusFromString(const std::string& s) {
    if (s == "Completed") return AppointmentStatus::Completed;
    if (s == "Cancelled") return AppointmentStatus::Cancelled;
    if (s == "Pending") return AppointmentStatus::Pending;
    throw utils::InvalidAppointmentException("Unknown appointment status: " + s);
}

std::string Appointment::statusText() const { return statusToString(status_); }

utils::Row Appointment::toRow() const {
    return {"APPOINTMENT", std::to_string(id_), std::to_string(patientId_), date_, time_, reason_,
            statusText()};
}

Appointment Appointment::fromRow(const utils::Row& r) {
    if (r.size() < 7) throw utils::InvalidAppointmentException("Appointment record has missing fields.");
    try {
        return Appointment(std::stoi(r[1]), std::stoi(r[2]), r[3], r[4], r[5], statusFromString(r[6]));
    } catch (const std::logic_error&) {   // stoi failures
        throw utils::InvalidAppointmentException("Appointment record contains a bad number.");
    }
}

} // namespace model
} // namespace scapms
