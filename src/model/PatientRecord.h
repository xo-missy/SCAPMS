#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Appointable.h"
#include "Appointment.h"
#include "../utils/FileHandler.h"

namespace scapms {
namespace model {

enum class Category { General, Emergency, FollowUp };

// Abstract base class of all patient types.
class PatientRecord : public Appointable {
public:
    PatientRecord(int id, const std::string& name, int age, const std::string& contact);
    ~PatientRecord() override = default;

    // ---- pure virtual functions: every patient type must provide these ----
    virtual Category category() const = 0;
    virtual std::string categoryName() const = 0;
    virtual std::string priorityLabel() const = 0;          // High Priority / Normal / Scheduled
    virtual int priorityLevel() const = 0;                  // 1 = most urgent
    virtual std::string describe() const = 0;               // one-line summary (polymorphic)
    virtual std::vector<std::string> detailLabels() const = 0;
    virtual std::vector<std::string> details() const = 0;
    virtual void setDetails(const std::vector<std::string>& values) = 0;   // validates

    // ---- Appointable (default behaviour, specialised by subclasses) ----
    int scheduleAppointment(const std::string& date, const std::string& time,
                            const std::string& reason) override;
    bool cancelAppointment(int appointmentId) override;

    // ---- common data (encapsulated, validated setters) ----
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    int age() const { return age_; }
    const std::string& contact() const { return contact_; }
    void setName(const std::string& name);
    void setAge(int age);
    void setContact(const std::string& contact);

    // ---- composition: a patient owns one or more appointments ----
    const std::vector<Appointment>& appointments() const { return appointments_; }
    std::vector<Appointment>& appointments() { return appointments_; }
    void addAppointment(const Appointment& a) { appointments_.push_back(a); }
    Appointment* findAppointment(int appointmentId);
    bool removeAppointment(int appointmentId);
    int countWithStatus(AppointmentStatus s) const;
    // Earliest pending appointment date (used for sorting); "9999-99-99" when none.
    std::string earliestPendingDate() const;

    // ---- persistence ----
    utils::Row toRow() const;     // PATIENT,category,id,name,age,contact,detail1,detail2...

    // ---- factory ----
    static std::unique_ptr<PatientRecord> create(Category c, int id, const std::string& name, int age,
                                                 const std::string& contact,
                                                 const std::vector<std::string>& details);
    static std::unique_ptr<PatientRecord> fromRow(const utils::Row& row);
    static std::vector<std::string> labelsFor(Category c);
    static std::string categoryToString(Category c);
    static Category categoryFromString(const std::string& s);   // throws InvalidPatientException

protected:
    static std::string trim(const std::string& s);

private:
    int id_;
    std::string name_;
    int age_;
    std::string contact_;
    std::vector<Appointment> appointments_;
};

} // namespace model
} // namespace scapms
