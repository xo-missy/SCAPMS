#pragma once
#include <QWidget>
#include "../controller/ClinicManager.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scapms {
namespace gui {

// "Appointments" tab: schedule, cancel, complete, remove and view appointments.
class AppointmentPanel : public QWidget {
    Q_OBJECT
public:
    explicit AppointmentPanel(controller::ClinicManager& mgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();
    void statusMessage(const QString& message);

private slots:
    void onSchedule();
    void onCancel();
    void onComplete();
    void onRemove();
    void onSelectionChanged();

private:
    controller::ClinicManager& mgr_;
    QLineEdit* patientIdEdit_;
    QLineEdit* dateEdit_;
    QLineEdit* timeEdit_;
    QLineEdit* reasonEdit_;
    QComboBox* filterCombo_;
    QTableWidget* table_;
};

} // namespace gui
} // namespace scapms
