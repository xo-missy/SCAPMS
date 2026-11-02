#pragma once
#include <string>
#include <vector>
#include "../model/ClinicDatabase.h"

namespace scapms {
namespace controller {

// A flattened, copy-based view of one appointment for tables and reports.
struct AppointmentRow {
    int patientId;
    std::string patientName;
    model::Appointment appt;
};

struct DailySummary {
    int total = 0, completed = 0, pending = 0, cancelled = 0;
};

// Business rules for appointments; works on the patients stored in a ClinicDatabase.
class AppointmentController {
public:
    explicit AppointmentController(model::ClinicDatabase& db) : db_(db) {}

    int schedule(int patientId, const std::string& date, const std::string& time, const std::string& reason);
    void cancel(int appointmentId);
    void complete(int appointmentId);
    // Permanently removes an appointment and returns a copy (for Undo).
    model::Appointment remove(int appointmentId);
    void restore(int patientId, const model::Appointment& a);

    std::vector<AppointmentRow> all() const;                         // ordered by date, then time
    std::vector<AppointmentRow> pending() const;
    std::vector<AppointmentRow> upcoming(int withinMinutes) const;   // pending, starting soon
    DailySummary summaryFor(const std::string& date) const;

private:
    model::ClinicDatabase& db_;
};

} // namespace controller
} // namespace scapms
