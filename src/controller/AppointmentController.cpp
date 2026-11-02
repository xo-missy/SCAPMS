#include "AppointmentController.h"
#include "../utils/DateUtils.h"
#include "../utils/Exceptions.h"

namespace scapms {
namespace controller {

using namespace model;

int AppointmentController::schedule(int patientId, const std::string& date, const std::string& time,
                                    const std::string& reason) {
    PatientRecord* p = db_.get(patientId);
    if (!p) throw utils::NotFoundException("No patient with ID " + std::to_string(patientId) + " exists.");
    if (!utils::DateUtils::isValidDate(date))
        throw utils::InvalidAppointmentException("Appointment date must be a valid date in the form YYYY-MM-DD.");
    if (!utils::DateUtils::isValidTime(time))
        throw utils::InvalidAppointmentException("Appointment time must be a valid time in the form HH:MM (24-hour).");

    // Emergency patients may be double-booked; everybody else needs a free slot.
    if (p->priorityLevel() != 1) {
        std::vector<PatientRecord*> all = db_.view();
        for (size_t i = 0; i < all.size(); ++i)
            for (size_t j = 0; j < all[i]->appointments().size(); ++j) {
                const Appointment& a = all[i]->appointments()[j];
                if (a.status() == AppointmentStatus::Pending && a.date() == date && a.time() == time)
                    throw utils::InvalidAppointmentException(
                        "That time slot (" + date + " " + time + ") is already booked.");
            }
    }
    return p->scheduleAppointment(date, time, reason);   // polymorphic call
}

void AppointmentController::cancel(int appointmentId) {
    PatientRecord* p = db_.findByAppointment(appointmentId);
    if (!p) throw utils::NotFoundException("No appointment with ID " + std::to_string(appointmentId) + " exists.");
    if (!p->cancelAppointment(appointmentId))
        throw utils::InvalidAppointmentException("Only pending appointments can be cancelled.");
}

void AppointmentController::complete(int appointmentId) {
    PatientRecord* p = db_.findByAppointment(appointmentId);
    if (!p) throw utils::NotFoundException("No appointment with ID " + std::to_string(appointmentId) + " exists.");
    Appointment* a = p->findAppointment(appointmentId);
    if (a->status() != AppointmentStatus::Pending)
        throw utils::InvalidAppointmentException("Only pending appointments can be marked as completed.");
    a->setStatus(AppointmentStatus::Completed);
}

Appointment AppointmentController::remove(int appointmentId) {
    PatientRecord* p = db_.findByAppointment(appointmentId);
    if (!p) throw utils::NotFoundException("No appointment with ID " + std::to_string(appointmentId) + " exists.");
    Appointment copy = *p->findAppointment(appointmentId);
    p->removeAppointment(appointmentId);
    return copy;
}

void AppointmentController::restore(int patientId, const Appointment& a) {
    PatientRecord* p = db_.get(patientId);
    if (p) p->addAppointment(a);
}

std::vector<AppointmentRow> AppointmentController::all() const {
    std::vector<AppointmentRow> rows;
    std::vector<PatientRecord*> ps = db_.view();
    for (size_t i = 0; i < ps.size(); ++i)
        for (size_t j = 0; j < ps[i]->appointments().size(); ++j)
            rows.push_back(AppointmentRow{ps[i]->id(), ps[i]->name(), ps[i]->appointments()[j]});
    // insertion sort by (date, time, id)
    for (size_t i = 1; i < rows.size(); ++i) {
        AppointmentRow key = rows[i];
        size_t j = i;
        while (j > 0) {
            const Appointment& prev = rows[j - 1].appt;
            bool before = (key.appt.date() != prev.date()) ? key.appt.date() < prev.date()
                        : (key.appt.time() != prev.time()) ? key.appt.time() < prev.time()
                        : key.appt.id() < prev.id();
            if (!before) break;
            rows[j] = rows[j - 1];
            --j;
        }
        rows[j] = key;
    }
    return rows;
}

std::vector<AppointmentRow> AppointmentController::pending() const {
    std::vector<AppointmentRow> out, rows = all();
    for (size_t i = 0; i < rows.size(); ++i)
        if (rows[i].appt.status() == AppointmentStatus::Pending) out.push_back(rows[i]);
    return out;
}

std::vector<AppointmentRow> AppointmentController::upcoming(int withinMinutes) const {
    std::vector<AppointmentRow> out, rows = pending();
    for (size_t i = 0; i < rows.size(); ++i) {
        long m = utils::DateUtils::minutesFromNow(rows[i].appt.date(), rows[i].appt.time());
        if (m >= 0 && m <= withinMinutes) out.push_back(rows[i]);
    }
    return out;
}

DailySummary AppointmentController::summaryFor(const std::string& date) const {
    DailySummary s;
    std::vector<AppointmentRow> rows = all();
    for (size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].appt.date() != date) continue;
        ++s.total;
        switch (rows[i].appt.status()) {
        case AppointmentStatus::Completed: ++s.completed; break;
        case AppointmentStatus::Pending: ++s.pending; break;
        case AppointmentStatus::Cancelled: ++s.cancelled; break;
        }
    }
    return s;
}

} // namespace controller
} // namespace scapms
