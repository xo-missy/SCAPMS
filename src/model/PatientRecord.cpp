#include "PatientRecord.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include "../utils/Exceptions.h"
#include "../utils/IDGenerator.h"
#include <algorithm>
#include <cctype>

namespace scapms {
namespace model {

std::string PatientRecord::trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

PatientRecord::PatientRecord(int id, const std::string& name, int age, const std::string& contact)
    : id_(id), age_(0) {
    if (id <= 0) throw utils::InvalidPatientException("Patient ID must be a positive number.");
    setName(name);
    setAge(age);
    setContact(contact);
}

void PatientRecord::setName(const std::string& name) {
    std::string n = trim(name);
    if (n.empty()) throw utils::InvalidPatientException("Patient name cannot be empty.");
    name_ = n;
}

void PatientRecord::setAge(int age) {
    if (age < 0 || age > 120) throw utils::InvalidAgeException();
    age_ = age;
}

void PatientRecord::setContact(const std::string& contact) {
    std::string c = trim(contact);
    if (c.empty()) throw utils::InvalidPatientException("Contact information cannot be empty.");
    contact_ = c;
}

int PatientRecord::scheduleAppointment(const std::string& date, const std::string& time,
                                       const std::string& reason) {
    int apptId = utils::IDGenerator::nextAppointmentId();
    appointments_.push_back(Appointment(apptId, id_, date, time, reason));   // validates
    return apptId;
}

bool PatientRecord::cancelAppointment(int appointmentId) {
    Appointment* a = findAppointment(appointmentId);
    if (!a || a->status() != AppointmentStatus::Pending) return false;
    a->setStatus(AppointmentStatus::Cancelled);
    return true;
}

Appointment* PatientRecord::findAppointment(int appointmentId) {
    for (size_t i = 0; i < appointments_.size(); ++i)
        if (appointments_[i].id() == appointmentId) return &appointments_[i];
    return nullptr;
}

bool PatientRecord::removeAppointment(int appointmentId) {
    for (size_t i = 0; i < appointments_.size(); ++i) {
        if (appointments_[i].id() == appointmentId) {
            appointments_.erase(appointments_.begin() + static_cast<long>(i));
            return true;
        }
    }
    return false;
}

int PatientRecord::countWithStatus(AppointmentStatus s) const {
    int n = 0;
    for (size_t i = 0; i < appointments_.size(); ++i)
        if (appointments_[i].status() == s) ++n;
    return n;
}

std::string PatientRecord::earliestPendingDate() const {
    std::string best = "9999-99-99";
    for (size_t i = 0; i < appointments_.size(); ++i)
        if (appointments_[i].status() == AppointmentStatus::Pending && appointments_[i].date() < best)
            best = appointments_[i].date();
    return best;
}

utils::Row PatientRecord::toRow() const {
    utils::Row r = {"PATIENT", categoryToString(category()), std::to_string(id_), name_,
                    std::to_string(age_), contact_};
    std::vector<std::string> d = details();
    r.insert(r.end(), d.begin(), d.end());
    return r;
}

std::string PatientRecord::categoryToString(Category c) {
    switch (c) {
    case Category::General: return "General";
    case Category::Emergency: return "Emergency";
    case Category::FollowUp: return "FollowUp";
    }
    return "General";
}

Category PatientRecord::categoryFromString(const std::string& s) {
    std::string t;
    for (char ch : s)
        if (std::isalnum(static_cast<unsigned char>(ch))) t += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (t == "general") return Category::General;
    if (t == "emergency") return Category::Emergency;
    if (t == "followup") return Category::FollowUp;
    throw utils::InvalidPatientException("Unknown patient category: '" + s + "'.");
}

std::vector<std::string> PatientRecord::labelsFor(Category c) {
    switch (c) {
    case Category::General: return {"Complaint"};
    case Category::Emergency: return {"Severity (1-5)", "Emergency type"};
    case Category::FollowUp: return {"Previous visit (YYYY-MM-DD)", "Doctor"};
    }
    return {};
}

std::unique_ptr<PatientRecord> PatientRecord::create(Category c, int id, const std::string& name, int age,
                                                     const std::string& contact,
                                                     const std::vector<std::string>& details) {
    std::unique_ptr<PatientRecord> p;
    switch (c) {
    case Category::General: p.reset(new GeneralPatient(id, name, age, contact)); break;
    case Category::Emergency: p.reset(new EmergencyPatient(id, name, age, contact)); break;
    case Category::FollowUp: p.reset(new FollowUpPatient(id, name, age, contact)); break;
    }
    p->setDetails(details);
    return p;
}

std::unique_ptr<PatientRecord> PatientRecord::fromRow(const utils::Row& r) {
    if (r.size() < 6) throw utils::InvalidPatientException("Patient record has missing fields.");
    int id = 0, age = 0;
    try {
        id = std::stoi(r[2]);
        age = std::stoi(r[4]);
    } catch (const std::logic_error&) {
        throw utils::InvalidPatientException("Patient record contains a bad number.");
    }
    std::vector<std::string> details(r.begin() + 6, r.end());
    return create(categoryFromString(r[1]), id, r[3], age, r[5], details);
}

} // namespace model
} // namespace scapms
