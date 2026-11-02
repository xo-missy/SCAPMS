#pragma once
#include <string>
#include "../utils/FileHandler.h"

namespace scapms {
namespace model {

enum class AppointmentStatus { Pending, Completed, Cancelled };

class Appointment {
public:
    // Validates date (YYYY-MM-DD) and time (HH:MM); throws InvalidAppointmentException.
    Appointment(int id, int patientId, const std::string& date, const std::string& time,
                const std::string& reason, AppointmentStatus status = AppointmentStatus::Pending);

    int id() const { return id_; }
    int patientId() const { return patientId_; }
    const std::string& date() const { return date_; }
    const std::string& time() const { return time_; }
    const std::string& reason() const { return reason_; }
    AppointmentStatus status() const { return status_; }
    void setStatus(AppointmentStatus s) { status_ = s; }

    std::string statusText() const;
    static std::string statusToString(AppointmentStatus s);
    static AppointmentStatus statusFromString(const std::string& s);

    utils::Row toRow() const;
    static Appointment fromRow(const utils::Row& row);   // row = APPOINTMENT,id,patientId,date,time,reason,status

private:
    int id_;
    int patientId_;
    std::string date_;
    std::string time_;
    std::string reason_;
    AppointmentStatus status_;
};

} // namespace model
} // namespace scapms
