#pragma once
#include <QList>
#include <QWidget>
#include <vector>
#include "../controller/ClinicManager.h"

class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scapms {
namespace gui {

// "Patient Records" tab: view, add, edit and delete patients.
class PatientPanel : public QWidget {
    Q_OBJECT
public:
    explicit PatientPanel(controller::ClinicManager& mgr, QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();
    void statusMessage(const QString& message);

private slots:
    void onAdd();
    void onUpdate();
    void onDelete();
    void onClear();
    void onCategoryChanged();
    void onSelectionChanged();
    void onIdTextChanged(const QString& text);

private:
    void rebuildDetailFields(model::Category c);    // dynamic components
    model::Category currentCategory() const;
    std::vector<std::string> detailValues() const;
    void loadIntoForm(const model::PatientRecord& p);
    void setEditing(int patientId);                 // 0 = entering a new patient

    controller::ClinicManager& mgr_;
    QLineEdit* idEdit_;
    QLineEdit* nameEdit_;
    QLineEdit* ageEdit_;
    QLineEdit* contactEdit_;
    QComboBox* categoryCombo_;
    QList<QLineEdit*> detailEdits_;
    QFormLayout* form_;
    int fixedRows_;
    QPushButton* addBtn_;
    QPushButton* updateBtn_;
    QPushButton* deleteBtn_;
    QPushButton* clearBtn_;
    QTableWidget* table_;
    QLabel* summaryLabel_;
    int editingId_;
};

} // namespace gui
} // namespace scapms
