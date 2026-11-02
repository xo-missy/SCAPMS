#pragma once
// Generates unique patient / appointment identifiers.
namespace scapms {
namespace utils {

class IDGenerator {
public:
    static int nextPatientId() { return patientCounter()++; }
    static int nextAppointmentId() { return appointmentCounter()++; }

    // Make sure freshly generated ids never collide with ids loaded from a file.
    static void seedPatient(int used) { if (used >= patientCounter()) patientCounter() = used + 1; }
    static void seedAppointment(int used) { if (used >= appointmentCounter()) appointmentCounter() = used + 1; }

    static void reset() { patientCounter() = 1001; appointmentCounter() = 1; }

private:
    static int& patientCounter() { static int c = 1001; return c; }
    static int& appointmentCounter() { static int c = 1; return c; }
};

} // namespace utils
} // namespace scapms
