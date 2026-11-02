#pragma once
#include "PatientRecord.h"

namespace scapms {
namespace model {

// Emergency patient: highest priority, has a triage severity, appointments are flagged urgent.
class EmergencyPatient : public PatientRecord {
public:
    EmergencyPatient(int id, const std::string& name, int age, const std::string& contact,
                     int severity = 3, const std::string& emergencyType = "Unspecified");

    Category category() const override { return Category::Emergency; }
    std::string categoryName() const override { return "Emergency"; }
    std::string priorityLabel() const override { return "High Priority"; }
    int priorityLevel() const override { return 1; }
    std::string describe() const override;
    std::vector<std::string> detailLabels() const override { return labelsFor(Category::Emergency); }
    std::vector<std::string> details() const override;
    void setDetails(const std::vector<std::string>& values) override;

    // Emergency appointments are always marked urgent.
    int scheduleAppointment(const std::string& date, const std::string& time,
                            const std::string& reason) override;

    int severity() const { return severity_; }

private:
    int severity_;
    std::string emergencyType_;
};

} // namespace model
} // namespace scapms
