#include "MainWindow.h"
#include "AdminPanel.h"
#include "AppointmentPanel.h"
#include "GuiUtils.h"
#include "PatientPanel.h"
#include "QueuePanel.h"
#include "SearchPanel.h"
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>

namespace scapms {
namespace gui {

namespace {
const int kReminderWindowMinutes = 15;    // remind this long before an appointment
const int kWaitAlertMinutes = 10;         // alert staff when a patient has waited this long
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Smart Clinic Appointment & Patient Management System (SCAPMS)");
    resize(1100, 700);

    // ----- multiple panels in tabs (dashboard-style main window) -----
    tabs_ = new QTabWidget(this);
    adminPanel_ = new AdminPanel(mgr_);
    patientPanel_ = new PatientPanel(mgr_);
    appointmentPanel_ = new AppointmentPanel(mgr_);
    queuePanel_ = new QueuePanel(mgr_);
    searchPanel_ = new SearchPanel(mgr_);
    tabs_->addTab(adminPanel_, "Dashboard && Reports");
    tabs_->addTab(patientPanel_, "Patient Records");
    tabs_->addTab(appointmentPanel_, "Appointments");
    tabs_->addTab(queuePanel_, "Waiting Queue");
    tabs_->addTab(searchPanel_, "Search && Sort");
    setCentralWidget(tabs_);

    statusLabel_ = new QLabel;
    statusBar()->addPermanentWidget(statusLabel_);
    createMenus();

    // ----- wire every panel: signals & slots -----
    connect(adminPanel_, &AdminPanel::dataChanged, this, &MainWindow::refreshAll);
    connect(patientPanel_, &PatientPanel::dataChanged, this, &MainWindow::refreshAll);
    connect(appointmentPanel_, &AppointmentPanel::dataChanged, this, &MainWindow::refreshAll);
    connect(queuePanel_, &QueuePanel::dataChanged, this, &MainWindow::refreshAll);
    connect(searchPanel_, &SearchPanel::dataChanged, this, &MainWindow::refreshAll);
    connect(adminPanel_, &AdminPanel::statusMessage, this, [this](const QString& m) { statusBar()->showMessage(m, 6000); });
    connect(patientPanel_, &PatientPanel::statusMessage, this, [this](const QString& m) { statusBar()->showMessage(m, 6000); });
    connect(appointmentPanel_, &AppointmentPanel::statusMessage, this, [this](const QString& m) { statusBar()->showMessage(m, 6000); });
    connect(queuePanel_, &QueuePanel::statusMessage, this, [this](const QString& m) { statusBar()->showMessage(m, 6000); });
    connect(searchPanel_, &SearchPanel::statusMessage, this, [this](const QString& m) { statusBar()->showMessage(m, 6000); });
    // switching tabs shows fresh data (e.g. the recently-accessed list on the dashboard)
    connect(tabs_, &QTabWidget::currentChanged, this, [this](int) { refreshAll(); });

    // ----- timer events -----
    dashboardTimer_ = new QTimer(this);
    connect(dashboardTimer_, &QTimer::timeout, this, &MainWindow::refreshDashboard);
    dashboardTimer_->start(5000);                    // refresh dashboard statistics every 5 s
    reminderTimer_ = new QTimer(this);
    connect(reminderTimer_, &QTimer::timeout, this, &MainWindow::checkReminders);
    reminderTimer_->start(30000);                    // look for reminders every 30 s
    QTimer::singleShot(1500, this, [this]() { checkReminders(); });

    updateStatusLabel();
    statusBar()->showMessage("Welcome to SCAPMS. Use File > Load to open saved data.", 8000);
}

void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* newAct = new QAction("&New", this);
    newAct->setShortcut(QKeySequence("Ctrl+N"));
    newAct->setStatusTip("Clear all data and start a new clinic file");
    QAction* saveAct = new QAction("&Save", this);
    saveAct->setShortcut(QKeySequence("Ctrl+S"));
    saveAct->setStatusTip("Save patients, appointments and the queue to a file");
    QAction* saveAsAct = new QAction("Save &As...", this);
    saveAsAct->setShortcut(QKeySequence("Ctrl+Shift+S"));
    QAction* loadAct = new QAction("&Load...", this);
    loadAct->setShortcut(QKeySequence("Ctrl+O"));
    loadAct->setStatusTip("Load patients and appointments from a file");
    QAction* exitAct = new QAction("E&xit", this);
    exitAct->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(loadAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* undoAct = new QAction("&Undo last administrative operation", this);
    undoAct->setShortcut(QKeySequence("Ctrl+Z"));
    QAction* findAct = new QAction("&Find patient...", this);
    findAct->setShortcut(QKeySequence("Ctrl+F"));
    editMenu->addAction(undoAct);
    editMenu->addAction(findAct);

    connect(newAct, &QAction::triggered, this, &MainWindow::newData);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveData);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveDataAs);
    connect(loadAct, &QAction::triggered, this, &MainWindow::loadData);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
    connect(undoAct, &QAction::triggered, this, &MainWindow::undoLast);
    connect(findAct, &QAction::triggered, this, &MainWindow::focusSearch);
}

void MainWindow::newData() {
    if (mgr_.patientCount() > 0 &&
        QMessageBox::question(this, "New clinic file",
                              "This clears all patients, appointments and the waiting queue. Continue?") !=
            QMessageBox::Yes)
        return;
    mgr_.clearAll();
    currentFile_.clear();
    remindedAppointments_.clear();
    alertedWaiting_.clear();
    refreshAll();
    statusBar()->showMessage("Started a new, empty clinic file.", 5000);
}

bool MainWindow::saveTo(const QString& path) {
    try {
        mgr_.save(ss(path));
        currentFile_ = path;
        statusBar()->showMessage("Saved to " + path, 6000);
        return true;
    } catch (const std::exception& e) {
        showError(this, e);
        return false;
    }
}

void MainWindow::saveData() {
    if (currentFile_.isEmpty()) { saveDataAs(); return; }
    saveTo(currentFile_);
}

void MainWindow::saveDataAs() {
    QString path = QFileDialog::getSaveFileName(this, "Save clinic data", QDir::homePath() + "/clinic_data.csv",
                                                "CSV files (*.csv);;All files (*)");
    if (path.isEmpty()) return;
    if (QFileInfo(path).suffix().isEmpty()) path += ".csv";
    saveTo(path);
}

void MainWindow::loadData() {
    QString path = QFileDialog::getOpenFileName(this, "Load clinic data", QDir::homePath(),
                                                "CSV files (*.csv);;All files (*)");
    if (path.isEmpty()) return;
    try {
        mgr_.load(ss(path));
        currentFile_ = path;
        remindedAppointments_.clear();
        alertedWaiting_.clear();
        refreshAll();
        statusBar()->showMessage("Loaded " + QString::number(mgr_.patientCount()) + " patients from " + path, 6000);
    } catch (const std::exception& e) {
        showError(this, e);   // meaningful message instead of crashing; existing data is untouched
    }
}

void MainWindow::undoLast() {
    try {
        statusBar()->showMessage(qs(mgr_.undo()), 6000);
        refreshAll();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void MainWindow::focusSearch() {
    tabs_->setCurrentWidget(searchPanel_);
    searchPanel_->focusSearchField();
}

void MainWindow::refreshAll() {
    adminPanel_->refresh();
    patientPanel_->refresh();
    appointmentPanel_->refresh();
    queuePanel_->refresh();
    searchPanel_->refresh();
    updateStatusLabel();
}

void MainWindow::refreshDashboard() {
    adminPanel_->refresh();
    queuePanel_->refresh();
    updateStatusLabel();
}

void MainWindow::updateStatusLabel() {
    controller::DashboardStats s = mgr_.stats();
    statusLabel_->setText(QString("Patients: %1   Waiting: %2   Pending appointments: %3")
                              .arg(s.total).arg(s.waiting).arg(s.pendingAppointments));
}

void MainWindow::showReminder(const QString& title, const QString& text) {
    QMessageBox* box = new QMessageBox(QMessageBox::Information, title, text, QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowModality(Qt::NonModal);   // does not block the staff member's work
    box->show();
    statusBar()->showMessage(text, 15000);
}

void MainWindow::checkReminders() {
    std::vector<controller::AppointmentRow> soon = mgr_.appointments().upcoming(kReminderWindowMinutes);
    for (size_t i = 0; i < soon.size(); ++i) {
        int id = soon[i].appt.id();
        if (remindedAppointments_.count(id)) continue;
        remindedAppointments_.insert(id);
        showReminder("Appointment reminder",
                     "Reminder: " + qs(soon[i].patientName) + " has an appointment at " + qs(soon[i].appt.time()) +
                         " (" + qs(soon[i].appt.reason()) + ").");
    }
    std::vector<controller::QueueEntry> waiting = mgr_.longWaiting(kWaitAlertMinutes);
    for (size_t i = 0; i < waiting.size(); ++i) {
        int pid = waiting[i].patientId;
        if (alertedWaiting_.count(pid)) continue;
        alertedWaiting_.insert(pid);
        const model::PatientRecord* p = mgr_.patient(pid);
        if (p)
            showReminder("Waiting patient",
                         qs(p->name()) + " has been waiting for more than " + QString::number(kWaitAlertMinutes) +
                             " minutes.");
    }
}

void MainWindow::tryLoadDefaultData() {
    QStringList candidates;
    candidates << QDir::current().filePath("data/sample_clinic.csv")
               << QCoreApplication::applicationDirPath() + "/data/sample_clinic.csv"
               << QCoreApplication::applicationDirPath() + "/../data/sample_clinic.csv"
               << QCoreApplication::applicationDirPath() + "/../../data/sample_clinic.csv";
    for (int i = 0; i < candidates.size(); ++i) {
        if (!QFileInfo::exists(candidates[i])) continue;
        try {
            mgr_.load(ss(candidates[i]));
            refreshAll();
            statusBar()->showMessage("Loaded sample data from " + candidates[i], 8000);
        } catch (const std::exception&) {
            // sample data is optional - ignore problems
        }
        return;
    }
}

} // namespace gui
} // namespace scapms
