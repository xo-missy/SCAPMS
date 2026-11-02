#pragma once
#include <QWidget>
#include "../controller/ClinicManager.h"

class QLabel;
class QLineEdit;
class QTableWidget;

namespace scapms {
namespace gui {

// "Waiting Queue" tab: first come, first served (std::queue in the ClinicManager).
class QueuePanel : public QWidget {
    Q_OBJECT
public:
    explicit QueuePanel(controller::ClinicManager& mgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();
    void statusMessage(const QString& message);

private slots:
    void onAdd();
    void onCallNext();

private:
    controller::ClinicManager& mgr_;
    QLineEdit* patientIdEdit_;
    QLabel* nowServing_;
    QTableWidget* table_;
};

} // namespace gui
} // namespace scapms
