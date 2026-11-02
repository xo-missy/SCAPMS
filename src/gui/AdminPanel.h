#pragma once
#include <QWidget>
#include "../controller/ClinicManager.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

namespace scapms {
namespace gui {

// "Dashboard" tab: statistics, Undo, recently accessed patients and reports.
class AdminPanel : public QWidget {
    Q_OBJECT
public:
    explicit AdminPanel(controller::ClinicManager& mgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();
    void statusMessage(const QString& message);

private slots:
    void onUndo();
    void onGenerateReport();

private:
    QWidget* statCardWidget(const QString& title, QLabel** valueOut);

    controller::ClinicManager& mgr_;
    QLabel* totalValue_;
    QLabel* generalValue_;
    QLabel* emergencyValue_;
    QLabel* followUpValue_;
    QLabel* waitingValue_;
    QLabel* pendingValue_;
    QLabel* lastOperation_;
    QPushButton* undoBtn_;
    QListWidget* recentList_;
    QComboBox* reportCombo_;
    QLineEdit* reportDateEdit_;
    QPlainTextEdit* reportView_;
};

} // namespace gui
} // namespace scapms
