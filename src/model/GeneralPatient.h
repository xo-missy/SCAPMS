#pragma once
#include "PatientRecord.h"

namespace scapms {
namespace model {

// Routine walk-in patient: normal priority, plain consultation.
class GeneralPatient : public PatientRecord {
public:
    GeneralPatient(int id, const std::string& name, int age, const std::string& contact,
                   const std::string& complaint = "General check-up");

    Category category() const override { return Category::General; }
    std::string categoryName() const override { return "General"; }
    std::string priorityLabel() const override { return "Normal"; }
    int priorityLevel() const override { return 2; }
    std::string describe() const override;
    std::vector<std::string> detailLabels() const override { return labelsFor(Category::General); }
    std::vector<std::string> details() const override { return {complaint_}; }
    void setDetails(const std::vector<std::string>& values) override;

    const std::string& complaint() const { return complaint_; }

private:
    std::string complaint_;
};

} // namespace model
} // namespace scapms
