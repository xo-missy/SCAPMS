#pragma once
// Custom exception hierarchy for SCAPMS.
#include <stdexcept>
#include <string>

namespace scapms {
namespace utils {

// Base class of every error raised by the clinic system.
class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidPatientException : public ClinicException {
public:
    explicit InvalidPatientException(const std::string& msg) : ClinicException(msg) {}
};

class InvalidAgeException : public InvalidPatientException {
public:
    explicit InvalidAgeException(const std::string& msg = "Patient age must be between 0 and 120.")
        : InvalidPatientException(msg) {}
};

class DuplicateIdException : public InvalidPatientException {
public:
    explicit DuplicateIdException(const std::string& msg = "Patient ID already exists.")
        : InvalidPatientException(msg) {}
};

class NotFoundException : public ClinicException {
public:
    explicit NotFoundException(const std::string& msg) : ClinicException(msg) {}
};

class InvalidAppointmentException : public ClinicException {
public:
    explicit InvalidAppointmentException(const std::string& msg) : ClinicException(msg) {}
};

class InvalidSearchException : public ClinicException {
public:
    explicit InvalidSearchException(const std::string& msg) : ClinicException(msg) {}
};

class FileException : public ClinicException {
public:
    explicit FileException(const std::string& msg) : ClinicException(msg) {}
};

} // namespace utils
} // namespace scapms
