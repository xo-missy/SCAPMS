# SCAPMS – Smart Clinic Appointment & Patient Management System
COS202 Programming II – Lab Assessment Project (C++17 + Qt Widgets)

## Build & run
Requires a C++17 compiler and **Qt 6 or Qt 5.15** (Widgets module).

**Qt Creator:** open `CMakeLists.txt` (or `SCAPMS.pro`), configure a kit, press Run.

**Command line (CMake):**
```
cmake -S . -B build
cmake --build build
./build/SCAPMS          # GUI (auto-loads data/sample_clinic.csv)
./build/test_core       # console tests for the non-GUI layers
```
The sample file `data/sample_clinic.csv` holds 10 patients and 15 appointments.

## Folder layout (header/source pairs, one namespace per layer)
| Folder | Namespace | Contents |
|---|---|---|
| `src/model` | `scapms::model` | `PatientRecord` (abstract), `GeneralPatient`, `EmergencyPatient`, `FollowUpPatient`, `Appointable` (interface), `Appointment`, `ClinicDatabase` |
| `src/controller` | `scapms::controller` | `ClinicManager`, `SearchEngine`, `SortEngine`, `AppointmentController` |
| `src/gui` | `scapms::gui` | `MainWindow`, `PatientPanel`, `AppointmentPanel`, `QueuePanel`, `SearchPanel`, `AdminPanel` (dashboard/reports/undo), `GuiUtils.h` |
| `src/utils` | `scapms::utils` | `IDGenerator`, `FileHandler`, `DateUtils`, `Exceptions` |

The model, controller and utils layers contain **no Qt code**, so GUI, business logic and data handling are separate.

## Requirement map
* **OOP** – abstract `PatientRecord` (pure virtuals: `category`, `priorityLabel`, `describe`, `details`, `setDetails` …); three subclasses with type-specific behaviour (Emergency tags appointments `[URGENT]` and may double-book, Follow-Up tags `Follow-up:`); `Appointable` interface; runtime polymorphism in `ClinicManager::processPatient(const PatientRecord&)` and `p->scheduleAppointment(...)`; composition: `ClinicDatabase` owns patients, each patient owns its `Appointment`s.
* **Data structures** – `std::vector` (patients), `std::queue` (waiting queue), `std::stack` (undo), fixed array `PatientRecord* recentPatients_[10]` (frequently-accessed cache).
* **Searching (hand-written)** – linear search by ID, binary search by ID (only when data is sorted by ID – "Automatic" mode picks the right one), recursive search by name, linear category search.
* **Sorting (hand-written)** – selection, insertion, merge (recursive), quick (recursive); by name, ID, age or appointment date; chosen in a combo box; the GUI explains why the chosen algorithm suits the situation.
* **Events** – button clicks, menu actions, text-field `textChanged`/`returnPressed`, timers.
* **Layouts** – `QVBoxLayout`, `QHBoxLayout`, `QGridLayout`, `QFormLayout` (form layouts for labelled input, grid for the dashboard cards and button blocks, box layouts for panel composition – so everything resizes with the window).
* **Advanced GUI** – colour-coded rows (emergency red / general green / follow-up blue; appointment status colours), dynamic form fields per patient type, file dialogs (Save/Load), timers (reminders 15 min ahead, waiting-time alerts after 10 min, dashboard refresh every 5 s), input validation dialogs, shortcuts (Ctrl+S save, Ctrl+F search, Ctrl+Q exit, Ctrl+N new, Ctrl+O load, Ctrl+Z undo), tooltips.
* **Exceptions** – custom hierarchy rooted at `ClinicException` (`InvalidAgeException`, `DuplicateIdException`, `InvalidAppointmentException`, `FileException`, `InvalidSearchException`, …); every GUI action is wrapped in try/catch and shows `Error: …` dialogs instead of crashing. A failed Load leaves the current data untouched.
* **Persistence** – CSV (`PATIENT`, `APPOINTMENT`, `QUEUE` rows).
* **Reports** – most frequently attended, pending appointments, category distribution, daily summary.

## Notes
* Dates are `YYYY-MM-DD`, times `HH:MM` (24 h).
* Undo covers: delete patient, modify patient, remove appointment (use "Remove (admin)" on the Appointments tab; "Cancel" just marks it cancelled).
* `std::sort`, `std::find`, `std::binary_search` etc. are deliberately **not** used anywhere.
