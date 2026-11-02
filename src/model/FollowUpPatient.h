#pragma once
#include "PatientRecord.h"

namespace scapms {
namespace model {

// Returning patient: remembers the previous visit and the doctor; appointments are tagged "Follow-up".
class FollowUpPatient : public PatientRecord {
public:
    FollowUpPatient(int id, const std::string& name, int age, const std::string& contact,
                    const std::string& previousVisit = "", const std::string& doctor = "Unassigned");

    Category category() const override { return Category::FollowUp; }
    std::string categoryName() const override { return "Follow-Up"; }
    std::string priorityLabel() const override { return "Scheduled"; }
    int priorityLevel() const override { return 3; }
    std::string describe() const override;
    std::vector<std::string> detailLabels() const override { return labelsFor(Category::FollowUp); }
    std::vector<std::string> details() const override { return {previousVisit_, doctor_}; }
    void setDetails(const std::vector<std::string>& values) override;

    int scheduleAppointment(const std::string& date, const std::string& time,
                            const std::string& reason) override;

private:
    std::string previousVisit_;
    std::string doctor_;
};

} // namespace model
} // namespace scapms
