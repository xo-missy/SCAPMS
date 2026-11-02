#pragma once
#include <ctime>
#include <memory>
#include <optional>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include "AppointmentController.h"
#include "SearchEngine.h"
#include "SortEngine.h"
#include "../model/ClinicDatabase.h"

namespace scapms {
namespace controller {

enum class SearchField { Id, Name, Category };
enum class SearchAlgorithm { Auto, Linear, Binary, Recursive };

struct SearchResult {
    PatientList matches;
    std::string algorithm;   // which algorithm actually ran
    std::string reason;      // why it was chosen
};

struct QueueEntry {
    int patientId;
    std::time_t joined;
};

struct DashboardStats {
    size_t total = 0, general = 0, emergency = 0, followUp = 0, waiting = 0, pendingAppointments = 0;
};

// Snapshot of the editable fields of a patient (used by Undo of "modify").
struct PatientSnapshot {
    std::string name;
    int age = 0;
    std::string contact;
    std::vector<std::string> details;
};

// One reversible administrative operation, stored on the undo stack.
struct UndoAction {
    enum class Type { DeletePatient, RemoveAppointment, ModifyPatient };
    Type type = Type::DeletePatient;
    std::string description;
    std::unique_ptr<model::PatientRecord> removedPatient;   // DeletePatient
    size_t index = 0;                                       // DeletePatient: old position
    int patientId = 0;                                      // RemoveAppointment / ModifyPatient
    std::optional<model::Appointment> appointment;          // RemoveAppointment
    PatientSnapshot snapshot;                               // ModifyPatient (old values)
};

// Central "business logic" object used by the GUI. Contains no GUI code.
class ClinicManager {
public:
    static const size_t kCacheSize = 10;

    ClinicManager();
    ClinicManager(const ClinicManager&) = delete;
    ClinicManager& operator=(const ClinicManager&) = delete;

    // ---- patients ----
    // id == 0 asks for an automatically generated id. Returns the id used.
    int addPatient(model::Category c, int id, const std::string& name, int age, const std::string& contact,
                   const std::vector<std::string>& details);
    void editPatient(int id, const std::string& name, int age, const std::string& contact,
                     const std::vector<std::string>& details);
    void deletePatient(int id);
    model::PatientRecord* patient(int id) const { return db_.get(id); }
    PatientList patients() const { return db_.view(); }
    size_t patientCount() const { return db_.size(); }
    // Polymorphism demo: works on ANY PatientRecord through the base class.
    std::string processPatient(const model::PatientRecord& p) const;

    // ---- appointments ----
    int scheduleAppointment(int patientId, const std::string& date, const std::string& time, const std::string& reason);
    void cancelAppointment(int appointmentId);
    void completeAppointment(int appointmentId);
    void removeAppointment(int appointmentId);        // undoable
    AppointmentController& appointments() { return appts_; }
    const AppointmentController& appointments() const { return appts_; }

    // ---- waiting queue (std::queue, FIFO) ----
    void enqueue(int patientId);
    model::PatientRecord* callNext();
    std::vector<QueueEntry> queueSnapshot() const;    // only patients that still exist
    std::vector<QueueEntry> longWaiting(int minutes) const;

    // ---- sorting / searching ----
    void sortPatients(SortAlgorithm algo, SortKey key);
    SearchResult search(SearchField field, const std::string& query, SearchAlgorithm algo = SearchAlgorithm::Auto);
    bool isSorted() const { return sorted_; }
    SortKey sortedKey() const { return sortedKey_; }

    // ---- undo (std::stack) ----
    bool canUndo() const { return !undo_.empty(); }
    std::string lastOperation() const;
    std::string undo();                               // returns a description of what was undone

    // ---- recent-patient cache (fixed-size array, most recent first) ----
    std::vector<model::PatientRecord*> recentPatients() const;
    void recordAccess(int patientId);

    // ---- reports ----
    std::string reportMostFrequentlyAttended() const;
    std::string reportPendingAppointments() const;
    std::string reportCategoryDistribution() const;
    std::string reportDailySummary(const std::string& date) const;
    DashboardStats stats() const;

    // ---- persistence ----
    void save(const std::string& path) const;
    void load(const std::string& path);               // throws FileException; state unchanged on failure
    void clearAll();

private:
    void touchCache(model::PatientRecord* p);
    void purgeCache(model::PatientRecord* p);
    void clearCache();
    PatientSnapshot snapshotOf(const model::PatientRecord& p) const;
    void applySnapshot(model::PatientRecord& p, const PatientSnapshot& s) const;

    model::ClinicDatabase db_;
    AppointmentController appts_;
    std::queue<QueueEntry> queue_;
    std::stack<UndoAction> undo_;
    model::PatientRecord* recentPatients_[kCacheSize];
    bool sorted_;
    SortKey sortedKey_;
};

} // namespace controller
} // namespace scapms
