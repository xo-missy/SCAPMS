#include "AppointmentPanel.h"
#include "GuiUtils.h"
#include "../utils/DateUtils.h"
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace scapms {
namespace gui {

using namespace model;

AppointmentPanel::AppointmentPanel(controller::ClinicManager& mgr, QWidget* parent)
    : QWidget(parent), mgr_(mgr) {
    QGroupBox* box = new QGroupBox("Appointment details", this);
    QFormLayout* form = new QFormLayout(box);
    patientIdEdit_ = new QLineEdit;
    patientIdEdit_->setPlaceholderText("patient ID");
    patientIdEdit_->setToolTip("ID of the patient who needs the appointment.");
    dateEdit_ = new QLineEdit(qs(utils::DateUtils::today()));
    dateEdit_->setToolTip("Date in the form YYYY-MM-DD.");
    timeEdit_ = new QLineEdit("09:00");
    timeEdit_->setToolTip("24-hour time in the form HH:MM.");
    reasonEdit_ = new QLineEdit;
    reasonEdit_->setPlaceholderText("e.g. Blood pressure review");
    reasonEdit_->setToolTip("Reason for the visit.");
    form->addRow("Patient ID:", patientIdEdit_);
    form->addRow("Date:", dateEdit_);
    form->addRow("Time:", timeEdit_);
    form->addRow("Reason:", reasonEdit_);

    QPushButton* scheduleBtn = new QPushButton("Schedule Appointment");
    QPushButton* cancelBtn = new QPushButton("Cancel Appointment");
    QPushButton* completeBtn = new QPushButton("Mark Completed");
    QPushButton* removeBtn = new QPushButton("Remove (admin)");
    scheduleBtn->setToolTip("Book the appointment for the patient above.");
    cancelBtn->setToolTip("Cancel the selected pending appointment (it stays in the list as Cancelled).");
    completeBtn->setToolTip("Mark the selected pending appointment as attended.");
    removeBtn->setToolTip("Permanently remove the selected appointment (can be undone).");

    filterCombo_ = new QComboBox;
    filterCombo_->addItems({"All", "Pending", "Completed", "Cancelled"});
    filterCombo_->setToolTip("Show only appointments with this status.");

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(scheduleBtn);
    buttons->addWidget(cancelBtn);
    buttons->addWidget(completeBtn);
    buttons->addWidget(removeBtn);
    QHBoxLayout* filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel("Show:"));
    filterRow->addWidget(filterCombo_);
    filterRow->addStretch();

    table_ = makeTable({"Appt ID", "Patient ID", "Patient", "Date", "Time", "Reason", "Status"}, this);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->addWidget(box);
    root->addLayout(buttons);
    root->addLayout(filterRow);
    root->addWidget(table_, 1);

    connect(scheduleBtn, &QPushButton::clicked, this, &AppointmentPanel::onSchedule);
    connect(cancelBtn, &QPushButton::clicked, this, &AppointmentPanel::onCancel);
    connect(completeBtn, &QPushButton::clicked, this, &AppointmentPanel::onComplete);
    connect(removeBtn, &QPushButton::clicked, this, &AppointmentPanel::onRemove);
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { refresh(); });
    connect(table_, &QTableWidget::itemSelectionChanged, this, &AppointmentPanel::onSelectionChanged);
    connect(reasonEdit_, &QLineEdit::returnPressed, this, &AppointmentPanel::onSchedule);

    refresh();
}

void AppointmentPanel::onSchedule() {
    try {
        int pid = parseInt<utils::InvalidAppointmentException>(patientIdEdit_->text(),
                                                               "Patient ID must be a whole number.");
        int apptId = mgr_.scheduleAppointment(pid, ss(dateEdit_->text().trimmed()), ss(timeEdit_->text().trimmed()),
                                              ss(reasonEdit_->text().trimmed()));
        emit statusMessage("Appointment " + QString::number(apptId) + " scheduled.");
        reasonEdit_->clear();
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void AppointmentPanel::onCancel() {
    int id = selectedId(table_);
    if (id < 0) { QMessageBox::information(this, "Cancel appointment", "Select an appointment in the table first."); return; }
    try {
        mgr_.cancelAppointment(id);
        emit statusMessage("Appointment " + QString::number(id) + " cancelled.");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void AppointmentPanel::onComplete() {
    int id = selectedId(table_);
    if (id < 0) { QMessageBox::information(this, "Complete appointment", "Select an appointment in the table first."); return; }
    try {
        mgr_.completeAppointment(id);
        emit statusMessage("Appointment " + QString::number(id) + " marked as completed.");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void AppointmentPanel::onRemove() {
    int id = selectedId(table_);
    if (id < 0) { QMessageBox::information(this, "Remove appointment", "Select an appointment in the table first."); return; }
    if (QMessageBox::question(this, "Remove appointment",
                              "Permanently remove appointment " + QString::number(id) + "?\nThis can be undone.") !=
        QMessageBox::Yes)
        return;
    try {
        mgr_.removeAppointment(id);
        emit statusMessage("Appointment " + QString::number(id) + " removed.");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void AppointmentPanel::onSelectionChanged() {
    int pid = selectedId(table_, 1);
    if (pid > 0) patientIdEdit_->setText(QString::number(pid));   // convenience: reuse patient for next booking
}

void AppointmentPanel::refresh() {
    int keepId = selectedId(table_);
    std::vector<controller::AppointmentRow> rows = mgr_.appointments().all();
    QString filter = filterCombo_->currentText();
    table_->blockSignals(true);
    table_->setRowCount(0);
    int row = 0, keepRow = -1;
    for (size_t i = 0; i < rows.size(); ++i) {
        const Appointment& a = rows[i].appt;
        if (filter != "All" && qs(a.statusText()) != filter) continue;
        table_->insertRow(row);
        setTableRow(table_, row,
                    {QString::number(a.id()), QString::number(rows[i].patientId), qs(rows[i].patientName),
                     qs(a.date()), qs(a.time()), qs(a.reason()), qs(a.statusText())},
                    statusColor(a.status()));
        if (a.id() == keepId) keepRow = row;
        ++row;
    }
    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
    if (keepRow >= 0) table_->selectRow(keepRow);
    table_->blockSignals(false);
}

} // namespace gui
} // namespace scapms
