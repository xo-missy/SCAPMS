// Console tests for the Qt-free layers (model, controller, utils).
#include <cassert>
#include <cstdio>
#include <iostream>
#include "../src/controller/ClinicManager.h"
#include "../src/utils/Exceptions.h"
#include "../src/utils/DateUtils.h"

using namespace scapms;
using namespace scapms::controller;
using model::Category;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cout << "FAIL line " << __LINE__ << ": " #c "\n"; ++failures; } } while (0)
#define THROWS(ex, stmt) do { bool ok = false; try { stmt; } catch (const ex&) { ok = true; } CHECK(ok); } while (0)

static std::vector<int> ids(const PatientList& v) { std::vector<int> r; for (auto* p : v) r.push_back(p->id()); return r; }

int main() {
    ClinicManager m;
    m.addPatient(Category::General, 30, "Zainab Bello", 40, "0801", {"Headache"});
    m.addPatient(Category::Emergency, 10, "adam Yusuf", 25, "0802", {"4", "Fracture"});
    m.addPatient(Category::FollowUp, 20, "Chinedu Okoro", 60, "0803", {"2026-09-01", "Dr. Ade"});
    m.addPatient(Category::General, 0, "Bola Ige", 12, "0804", {});
    CHECK(m.patientCount() == 4);

    // validation + custom exceptions
    THROWS(utils::DuplicateIdException, m.addPatient(Category::General, 10, "X", 1, "1", {}));
    THROWS(utils::InvalidAgeException, m.addPatient(Category::General, 99, "X", 121, "1", {}));
    THROWS(utils::InvalidAgeException, m.addPatient(Category::General, 99, "X", -1, "1", {}));
    THROWS(utils::InvalidPatientException, m.addPatient(Category::Emergency, 99, "X", 5, "1", {"9", "t"}));
    THROWS(utils::InvalidPatientException, m.addPatient(Category::General, 99, "  ", 5, "1", {}));
    CHECK(m.patientCount() == 4);

    // all sorts, all keys, agree with each other
    SortAlgorithm algos[] = {SortAlgorithm::Selection, SortAlgorithm::Insertion, SortAlgorithm::Merge, SortAlgorithm::Quick};
    SortKey keys[] = {SortKey::Name, SortKey::Id, SortKey::Age, SortKey::AppointmentDate};
    for (SortKey k : keys) {
        m.sortPatients(SortAlgorithm::Merge, k);
        std::vector<int> ref = ids(m.patients());
        for (SortAlgorithm a : algos) {
            m.sortPatients(SortAlgorithm::Selection, SortKey::Age);   // scramble
            m.sortPatients(a, k);
            CHECK(ids(m.patients()) == ref);
        }
    }
    m.sortPatients(SortAlgorithm::Quick, SortKey::Name);
    CHECK(m.patients().front()->name() == "adam Yusuf");   // case-insensitive

    // bigger randomised sort check
    {
        ClinicManager big;
        unsigned s = 12345;
        for (int i = 1; i <= 300; ++i) { s = s * 1103515245u + 12345u; big.addPatient(Category::General, i, "P" + std::to_string(s % 1000), int(s % 100), "c", {}); }
        for (SortAlgorithm a : algos) {
            big.sortPatients(a, SortKey::Age);
            auto v = big.patients();
            for (size_t i = 1; i < v.size(); ++i) CHECK(v[i-1]->age() <= v[i]->age());
        }
    }

    // searching
    SearchResult r = m.search(SearchField::Id, "20");
    CHECK(r.matches.size() == 1 && r.algorithm == "Linear Search");   // sorted by name, not id
    THROWS(utils::InvalidSearchException, m.search(SearchField::Id, "20", SearchAlgorithm::Binary));
    m.sortPatients(SortAlgorithm::Insertion, SortKey::Id);
    r = m.search(SearchField::Id, "30");
    CHECK(r.matches.size() == 1 && r.algorithm == "Binary Search" && r.matches[0]->name() == "Zainab Bello");
    CHECK(m.search(SearchField::Id, "777").matches.empty());
    THROWS(utils::InvalidSearchException, m.search(SearchField::Id, "abc"));
    THROWS(utils::InvalidSearchException, m.search(SearchField::Name, "   "));
    r = m.search(SearchField::Name, "o");
    CHECK(r.algorithm == "Recursive Search" && r.matches.size() == 3);   // Bello, Okoro, Bola
    r = m.search(SearchField::Category, "emergency");
    CHECK(r.matches.size() == 1);
    THROWS(utils::ClinicException, m.search(SearchField::Category, "nonsense"));
    CHECK(m.recentPatients().size() >= 1);

    // appointments + polymorphic scheduling
    int a1 = m.scheduleAppointment(10, "2026-11-01", "09:00", "Cast check");
    int a2 = m.scheduleAppointment(20, "2026-11-01", "10:00", "Review");
    m.scheduleAppointment(30, "2026-11-02", "10:00", "Headache");
    CHECK(m.patient(10)->appointments()[0].reason().rfind("[URGENT] ", 0) == 0);
    CHECK(m.patient(20)->appointments()[0].reason().rfind("Follow-up: ", 0) == 0);
    THROWS(utils::InvalidAppointmentException, m.scheduleAppointment(30, "2026-11-01", "10:00", "clash"));
    m.scheduleAppointment(10, "2026-11-01", "10:00", "emergency double-book ok");
    THROWS(utils::InvalidAppointmentException, m.scheduleAppointment(30, "2026-13-01", "10:00", "x"));
    THROWS(utils::InvalidAppointmentException, m.scheduleAppointment(30, "2026-11-05", "25:00", "x"));
    THROWS(utils::NotFoundException, m.scheduleAppointment(555, "2026-11-05", "10:00", "x"));
    m.completeAppointment(a1);
    m.cancelAppointment(a2);
    THROWS(utils::InvalidAppointmentException, m.cancelAppointment(a2));
    CHECK(m.reportDailySummary("2026-11-01").find("Total Appointments: 3") != std::string::npos);
    CHECK(m.reportMostFrequentlyAttended().find("adam Yusuf") != std::string::npos);
    CHECK(m.reportCategoryDistribution().find("Emergency Patients") != std::string::npos);

    // queue is FIFO and skips deleted patients
    m.enqueue(30); m.enqueue(10); m.enqueue(20);
    THROWS(utils::ClinicException, m.enqueue(30));
    CHECK(m.callNext()->id() == 30);
    m.deletePatient(10);
    CHECK(m.callNext()->id() == 20);
    THROWS(utils::ClinicException, m.callNext());

    // undo stack: delete / modify / remove appointment
    CHECK(m.patientCount() == 3);
    std::string desc = m.undo();                 // undo delete 10
    CHECK(m.patientCount() == 4 && m.patient(10) && m.patient(10)->appointments().size() == 2);
    m.editPatient(30, "Zainab B.", 41, "0999", {"Cough"});
    CHECK(m.patient(30)->name() == "Zainab B.");
    THROWS(utils::InvalidAgeException, m.editPatient(30, "Zainab B.", 500, "0999", {"Cough"}));
    CHECK(m.patient(30)->age() == 41);
    m.undo();
    CHECK(m.patient(30)->name() == "Zainab Bello" && m.patient(30)->age() == 40);
    m.removeAppointment(a1);
    CHECK(m.patient(10)->appointments().size() == 1);
    m.undo();
    CHECK(m.patient(10)->appointments().size() == 2);
    while (m.canUndo()) m.undo();
    THROWS(utils::ClinicException, m.undo());

    // save / load round trip + failures
    m.enqueue(30);
    const char* path = "/tmp/scapms_test.csv";
    m.save(path);
    ClinicManager m2;
    m2.load(path);
    CHECK(m2.patientCount() == 4);
    CHECK(m2.patient(10)->appointments().size() == 2);
    CHECK(m2.queueSnapshot().size() == 1);
    CHECK(m2.patient(20)->details()[1] == "Dr. Ade");
    THROWS(utils::FileException, m2.load("/nonexistent/file.csv"));
    { FILE* f = std::fopen("/tmp/scapms_bad.csv", "w"); std::fputs("PATIENT,General,1,Bob,200,x,y\n", f); std::fclose(f); }
    THROWS(utils::FileException, m2.load("/tmp/scapms_bad.csv"));
    CHECK(m2.patientCount() == 4);               // failed load leaves state untouched

    // polymorphism
    for (auto* p : m.patients()) CHECK(!m.processPatient(*p).empty());

    // cache is a bounded array
    ClinicManager c;
    for (int i = 1; i <= 15; ++i) { c.addPatient(Category::General, i, "N" + std::to_string(i), 20, "c", {}); c.recordAccess(i); }
    CHECK(c.recentPatients().size() == 10 && c.recentPatients()[0]->id() == 15);

    std::cout << (failures ? "TESTS FAILED\n" : "ALL TESTS PASSED\n");
    return failures ? 1 : 0;
}
