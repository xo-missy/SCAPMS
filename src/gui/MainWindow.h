#pragma once
#include <QMainWindow>
#include <QString>
#include <set>
#include "../controller/ClinicManager.h"

class QLabel;
class QTabWidget;
class QTimer;

namespace scapms {
namespace gui {

class PatientPanel;
class AppointmentPanel;
class QueuePanel;
class SearchPanel;
class AdminPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void tryLoadDefaultData();      // silently loads data/sample_clinic.csv when it can be found

private slots:
    void newData();
    void saveData();
    void saveDataAs();
    void loadData();
    void undoLast();
    void focusSearch();
    void refreshAll();
    void refreshDashboard();        // timer: dashboard refresh
    void checkReminders();          // timer: appointment reminders + waiting-time alerts

private:
    void createMenus();
    void updateStatusLabel();
    bool saveTo(const QString& path);
    void showReminder(const QString& title, const QString& text);

    controller::ClinicManager mgr_;
    QTabWidget* tabs_;
    PatientPanel* patientPanel_;
    AppointmentPanel* appointmentPanel_;
    QueuePanel* queuePanel_;
    SearchPanel* searchPanel_;
    AdminPanel* adminPanel_;
    QLabel* statusLabel_;
    QTimer* dashboardTimer_;
    QTimer* reminderTimer_;
    QString currentFile_;
    std::set<int> remindedAppointments_;
    std::set<int> alertedWaiting_;
};

} // namespace gui
} // namespace scapms
