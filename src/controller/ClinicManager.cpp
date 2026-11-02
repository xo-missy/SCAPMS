#include "ClinicManager.h"
#include "../utils/Exceptions.h"
#include "../utils/FileHandler.h"
#include "../utils/IDGenerator.h"
#include "../utils/DateUtils.h"
#include <cctype>
#include <iomanip>
#include <sstream>

namespace scapms {
namespace controller {

using namespace model;
using namespace utils;

ClinicManager::ClinicManager() : appts_(db_), sorted_(false), sortedKey_(SortKey::Id) {
    clearCache();
}

// =============================== patients ===============================
int ClinicManager::addPatient(Category c, int id, const std::string& name, int age, const std::string& contact,
                              const std::vector<std::string>& details) {
    if (id < 0) throw InvalidPatientException("Patient ID must be a positive number.");
    if (id == 0) {
        do { id = IDGenerator::nextPatientId(); } while (db_.contains(id));
    } else if (db_.contains(id)) {
        throw DuplicateIdException();
    }
    db_.add(PatientRecord::create(c, id, name, age, contact, details));   // validates everything
    IDGenerator::seedPatient(id);
    sorted_ = false;
    return id;
}

PatientSnapshot ClinicManager::snapshotOf(const PatientRecord& p) const {
    PatientSnapshot s;
    s.name = p.name();
    s.age = p.age();
    s.contact = p.contact();
    s.details = p.details();
    return s;
}

void ClinicManager::applySnapshot(PatientRecord& p, const PatientSnapshot& s) const {
    p.setName(s.name);
    p.setAge(s.age);
    p.setContact(s.contact);
    p.setDetails(s.details);
}

void ClinicManager::editPatient(int id, const std::string& name, int age, const std::string& contact,
                                const std::vector<std::string>& details) {
    PatientRecord* p = db_.get(id);
    if (!p) throw NotFoundException("No patient with ID " + std::to_string(id) + " exists.");
    PatientSnapshot before = snapshotOf(*p);
    PatientSnapshot after{name, age, contact, details};
    try {
        applySnapshot(*p, after);
    } catch (...) {
        applySnapshot(*p, before);   // roll back a half-applied edit
        throw;
    }
    UndoAction a;
    a.type = UndoAction::Type::ModifyPatient;
    a.description = "Modify patient " + std::to_string(id) + " (" + before.name + ")";
    a.patientId = id;
    a.snapshot = before;
    undo_.push(std::move(a));
    sorted_ = false;
}

void ClinicManager::deletePatient(int id) {
    size_t idx = 0;
    std::unique_ptr<PatientRecord> removed = db_.removeById(id, &idx);   // throws if missing
    purgeCache(removed.get());
    UndoAction a;
    a.type = UndoAction::Type::DeletePatient;
    a.description = "Delete patient " + std::to_string(id) + " (" + removed->name() + ")";
    a.index = idx;
    a.patientId = id;
    a.removedPatient = std::move(removed);
    undo_.push(std::move(a));
    sorted_ = false;
}

std::string ClinicManager::processPatient(const PatientRecord& p) const {
    // Virtual calls: the same code handles General, Emergency and Follow-up patients.
    std::ostringstream os;
    os << "[" << p.id() << "] " << p.name() << " (" << p.categoryName() << ", " << p.priorityLabel() << ") - "
       << p.describe();
    return os.str();
}

// =============================== appointments ===============================
int ClinicManager::scheduleAppointment(int patientId, const std::string& date, const std::string& time,
                                       const std::string& reason) {
    return appts_.schedule(patientId, date, time, reason);
}
void ClinicManager::cancelAppointment(int id) { appts_.cancel(id); }
void ClinicManager::completeAppointment(int id) { appts_.complete(id); }

void ClinicManager::removeAppointment(int id) {
    PatientRecord* owner = db_.findByAppointment(id);
    int pid = owner ? owner->id() : 0;
    Appointment removed = appts_.remove(id);   // throws if missing
    UndoAction a;
    a.type = UndoAction::Type::RemoveAppointment;
    a.description = "Remove appointment " + std::to_string(id) + " of patient " + std::to_string(pid);
    a.patientId = pid;
    a.appointment = removed;
    undo_.push(std::move(a));
}

// =============================== queue ===============================
void ClinicManager::enqueue(int patientId) {
    if (!db_.contains(patientId))
        throw NotFoundException("No patient with ID " + std::to_string(patientId) + " exists.");
    std::vector<QueueEntry> now = queueSnapshot();
    for (size_t i = 0; i < now.size(); ++i)
        if (now[i].patientId == patientId) throw ClinicException("That patient is already in the waiting queue.");
    queue_.push(QueueEntry{patientId, std::time(nullptr)});
}

PatientRecord* ClinicManager::callNext() {
    while (!queue_.empty()) {
        QueueEntry e = queue_.front();
        queue_.pop();
        PatientRecord* p = db_.get(e.patientId);
        if (p) return p;               // skip patients who were deleted while waiting
    }
    throw ClinicException("The waiting queue is empty.");
}

std::vector<QueueEntry> ClinicManager::queueSnapshot() const {
    std::vector<QueueEntry> out;
    std::queue<QueueEntry> copy = queue_;
    while (!copy.empty()) {
        if (db_.contains(copy.front().patientId)) out.push_back(copy.front());
        copy.pop();
    }
    return out;
}

std::vector<QueueEntry> ClinicManager::longWaiting(int minutes) const {
    std::vector<QueueEntry> out, all = queueSnapshot();
    std::time_t now = std::time(nullptr);
    for (size_t i = 0; i < all.size(); ++i)
        if (std::difftime(now, all[i].joined) >= minutes * 60.0) out.push_back(all[i]);
    return out;
}

// =============================== sort / search ===============================
void ClinicManager::sortPatients(SortAlgorithm algo, SortKey key) {
    PatientList v = db_.view();
    SortEngine::sort(v, algo, key);
    db_.reorder(v);
    sorted_ = true;
    sortedKey_ = key;
}

SearchResult ClinicManager::search(SearchField field, const std::string& query, SearchAlgorithm algo) {
    std::string q = query;
    while (!q.empty() && std::isspace(static_cast<unsigned char>(q.front()))) q.erase(q.begin());
    while (!q.empty() && std::isspace(static_cast<unsigned char>(q.back()))) q.pop_back();
    if (q.empty()) throw InvalidSearchException("Please enter a search keyword.");

    PatientList all = db_.view();
    SearchResult r;

    if (field == SearchField::Id) {
        int id = 0;
        for (size_t i = 0; i < q.size(); ++i)
            if (!std::isdigit(static_cast<unsigned char>(q[i]))) throw InvalidSearchException("Patient ID must be a number.");
        if (q.size() > 9) throw InvalidSearchException("Patient ID is too long.");
        id = std::stoi(q);
        if (algo == SearchAlgorithm::Recursive)
            throw InvalidSearchException("Recursive search is only available for patient names.");
        bool sortedById = sorted_ && sortedKey_ == SortKey::Id;
        if (algo == SearchAlgorithm::Binary && !sortedById)
            throw InvalidSearchException("Binary search needs the records sorted by Patient ID. Sort by ID first.");
        bool useBinary = (algo == SearchAlgorithm::Binary) || (algo == SearchAlgorithm::Auto && sortedById);
        PatientRecord* hit = nullptr;
        if (useBinary) {
            hit = SearchEngine::binarySearchById(all, id);
            r.algorithm = "Binary Search";
            r.reason = "Records are sorted by ID, so halving the search range (O(log n)) is fastest.";
        } else {
            hit = SearchEngine::linearSearchById(all, id);
            r.algorithm = "Linear Search";
            r.reason = "Records are not sorted by ID, so every record must be checked in turn (O(n)).";
        }
        if (hit) r.matches.push_back(hit);
    } else if (field == SearchField::Name) {
        if (algo == SearchAlgorithm::Binary)
            throw InvalidSearchException("Binary search is only available for patient IDs.");
        if (algo == SearchAlgorithm::Linear) {
            r.matches = SearchEngine::linearSearchByName(all, q);
            r.algorithm = "Linear Search";
            r.reason = "Iterative scan of every name (substring match).";
        } else {
            SearchEngine::recursiveSearchByName(all, q, 0, r.matches);
            r.algorithm = "Recursive Search";
            r.reason = "Names are matched by substring, so the list is scanned recursively one record at a time.";
        }
    } else {
        if (algo == SearchAlgorithm::Binary || algo == SearchAlgorithm::Recursive)
            throw InvalidSearchException("Category search uses linear search.");
        r.matches = SearchEngine::linearSearchByCategory(all, PatientRecord::categoryFromString(q));
        r.algorithm = "Linear Search";
        r.reason = "Category is not a unique key, so every record is checked.";
    }
    for (size_t i = 0; i < r.matches.size() && i < 3; ++i) touchCache(r.matches[i]);
    return r;
}

// =============================== undo ===============================
std::string ClinicManager::lastOperation() const {
    return undo_.empty() ? std::string("(nothing to undo)") : undo_.top().description;
}

std::string ClinicManager::undo() {
    if (undo_.empty()) throw ClinicException("There is no administrative operation to undo.");
    UndoAction a = std::move(undo_.top());
    undo_.pop();
    switch (a.type) {
    case UndoAction::Type::DeletePatient:
        db_.insertAt(a.index, std::move(a.removedPatient));
        break;
    case UndoAction::Type::RemoveAppointment:
        appts_.restore(a.patientId, *a.appointment);
        break;
    case UndoAction::Type::ModifyPatient: {
        PatientRecord* p = db_.get(a.patientId);
        if (p) applySnapshot(*p, a.snapshot);
        break;
    }
    }
    sorted_ = false;
    return "Undone: " + a.description;
}

// =============================== cache (fixed-size array) ===============================
void ClinicManager::clearCache() {
    for (size_t i = 0; i < kCacheSize; ++i) recentPatients_[i] = nullptr;
}

void ClinicManager::touchCache(PatientRecord* p) {
    if (!p) return;
    size_t pos = kCacheSize - 1;                 // default: evict the oldest slot
    for (size_t i = 0; i < kCacheSize; ++i)
        if (recentPatients_[i] == p) { pos = i; break; }
    for (size_t i = pos; i > 0; --i) recentPatients_[i] = recentPatients_[i - 1];
    recentPatients_[0] = p;
}

void ClinicManager::purgeCache(PatientRecord* p) {
    size_t w = 0;
    for (size_t i = 0; i < kCacheSize; ++i)
        if (recentPatients_[i] != p) recentPatients_[w++] = recentPatients_[i];
    while (w < kCacheSize) recentPatients_[w++] = nullptr;
}

std::vector<PatientRecord*> ClinicManager::recentPatients() const {
    std::vector<PatientRecord*> v;
    for (size_t i = 0; i < kCacheSize; ++i)
        if (recentPatients_[i]) v.push_back(recentPatients_[i]);
    return v;
}

void ClinicManager::recordAccess(int patientId) { touchCache(db_.get(patientId)); }

// =============================== reports ===============================
std::string ClinicManager::reportMostFrequentlyAttended() const {
    PatientList v = db_.view();
    SortEngine::insertionSort(v, [](const PatientRecord* a, const PatientRecord* b) {
        int x = a->countWithStatus(AppointmentStatus::Completed), y = b->countWithStatus(AppointmentStatus::Completed);
        if (x != y) return x > y;
        return a->id() < b->id();
    });
    std::ostringstream os;
    os << "MOST FREQUENTLY ATTENDED PATIENTS\n--------------------------------\n";
    int shown = 0;
    for (size_t i = 0; i < v.size() && shown < 10; ++i) {
        int n = v[i]->countWithStatus(AppointmentStatus::Completed);
        if (n == 0) break;
        os << std::left << std::setw(7) << v[i]->id() << std::setw(24) << v[i]->name() << n
           << " completed appointment" << (n == 1 ? "" : "s") << "\n";
        ++shown;
    }
    if (shown == 0) os << "No completed appointments yet.\n";
    return os.str();
}

std::string ClinicManager::reportPendingAppointments() const {
    std::ostringstream os;
    os << "PATIENTS WITH PENDING APPOINTMENTS\n----------------------------------\n";
    PatientList v = db_.view();
    int count = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        int n = v[i]->countWithStatus(AppointmentStatus::Pending);
        if (n == 0) continue;
        ++count;
        os << std::left << std::setw(7) << v[i]->id() << std::setw(24) << v[i]->name() << n << " pending (next: "
           << v[i]->earliestPendingDate() << ")\n";
    }
    if (count == 0) os << "No patients have pending appointments.\n";
    return os.str();
}

std::string ClinicManager::reportCategoryDistribution() const {
    DashboardStats s = stats();
    std::ostringstream os;
    os << "PATIENT CATEGORY DISTRIBUTION\n-----------------------------\n"
       << std::left << std::setw(20) << "General Patients" << s.general << "\n"
       << std::setw(20) << "Emergency Patients" << s.emergency << "\n"
       << std::setw(20) << "Follow-Up Patients" << s.followUp << "\n"
       << std::setw(20) << "Total" << s.total << "\n";
    return os.str();
}

std::string ClinicManager::reportDailySummary(const std::string& date) const {
    if (!DateUtils::isValidDate(date))
        throw InvalidAppointmentException("Report date must be a valid date in the form YYYY-MM-DD.");
    DailySummary d = appts_.summaryFor(date);
    std::ostringstream os;
    os << "DAILY APPOINTMENT SUMMARY - " << date << "\n------------------------------------\n"
       << "Total Appointments: " << d.total << "\nCompleted: " << d.completed << "\nPending: " << d.pending
       << "\nCancelled: " << d.cancelled << "\n";
    return os.str();
}

DashboardStats ClinicManager::stats() const {
    DashboardStats s;
    PatientList v = db_.view();
    s.total = v.size();
    for (size_t i = 0; i < v.size(); ++i) {
        switch (v[i]->category()) {
        case Category::General: ++s.general; break;
        case Category::Emergency: ++s.emergency; break;
        case Category::FollowUp: ++s.followUp; break;
        }
        s.pendingAppointments += static_cast<size_t>(v[i]->countWithStatus(AppointmentStatus::Pending));
    }
    s.waiting = queueSnapshot().size();
    return s;
}

// =============================== persistence ===============================
void ClinicManager::save(const std::string& path) const {
    std::vector<Row> rows;
    PatientList v = db_.view();
    for (size_t i = 0; i < v.size(); ++i) rows.push_back(v[i]->toRow());
    for (size_t i = 0; i < v.size(); ++i)
        for (size_t j = 0; j < v[i]->appointments().size(); ++j) rows.push_back(v[i]->appointments()[j].toRow());
    std::vector<QueueEntry> q = queueSnapshot();
    for (size_t i = 0; i < q.size(); ++i) rows.push_back({"QUEUE", std::to_string(q[i].patientId), std::to_string(static_cast<long long>(q[i].joined))});
    FileHandler::writeCsv(path, rows);
}

void ClinicManager::load(const std::string& path) {
    std::vector<Row> rows = FileHandler::readCsv(path);
    ClinicDatabase tmp;
    std::vector<QueueEntry> q;
    int maxPatient = 0, maxAppt = 0;
    for (size_t i = 0; i < rows.size(); ++i) {
        const Row& r = rows[i];
        std::string where = "Line " + std::to_string(i + 1) + ": ";
        try {
            if (r[0] == "PATIENT") {
                std::unique_ptr<PatientRecord> p = PatientRecord::fromRow(r);
                if (p->id() > maxPatient) maxPatient = p->id();
                tmp.add(std::move(p));
            } else if (r[0] == "APPOINTMENT") {
                Appointment a = Appointment::fromRow(r);
                PatientRecord* owner = tmp.get(a.patientId());
                if (!owner) throw InvalidAppointmentException("appointment refers to unknown patient " + std::to_string(a.patientId()));
                if (a.id() > maxAppt) maxAppt = a.id();
                owner->addAppointment(a);
            } else if (r[0] == "QUEUE") {
                if (r.size() < 3) throw ClinicException("queue entry has missing fields");
                q.push_back(QueueEntry{std::stoi(r[1]), static_cast<std::time_t>(std::stoll(r[2]))});
            } else {
                throw ClinicException("unrecognised record type '" + r[0] + "'");
            }
        } catch (const FileException&) {
            throw;
        } catch (const std::logic_error&) {   // std::stoi / std::stoll failures
            throw FileException(where + "bad number in record");
        } catch (const std::exception& e) {
            throw FileException(where + e.what());
        }
    }
    // everything parsed -> commit
    db_ = std::move(tmp);
    queue_ = std::queue<QueueEntry>();
    for (size_t i = 0; i < q.size(); ++i) queue_.push(q[i]);
    while (!undo_.empty()) undo_.pop();
    clearCache();
    IDGenerator::seedPatient(maxPatient);
    IDGenerator::seedAppointment(maxAppt);
    sorted_ = false;
}

void ClinicManager::clearAll() {
    db_.clear();
    queue_ = std::queue<QueueEntry>();
    while (!undo_.empty()) undo_.pop();
    clearCache();
    sorted_ = false;
}

} // namespace controller
} // namespace scapms
