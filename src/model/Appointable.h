#pragma once
// Interface-like abstract class: anything that can be scheduled for appointments.
#include <string>

namespace scapms {
namespace model {

class Appointable {
public:
    virtual ~Appointable() = default;
    // Schedules an appointment and returns the new appointment id.
    virtual int scheduleAppointment(const std::string& date, const std::string& time,
                                    const std::string& reason) = 0;
    // Cancels a pending appointment. Returns false if it was not found / not pending.
    virtual bool cancelAppointment(int appointmentId) = 0;
};

} // namespace model
} // namespace scapms
