#include "PatientPanel.h"
#include "GuiUtils.h"
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
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

PatientPanel::PatientPanel(controller::ClinicManager& mgr, QWidget* parent)
    : QWidget(parent), mgr_(mgr), editingId_(0) {
    // ---------- form (QFormLayout: label/field pairs) ----------
    QGroupBox* box = new QGroupBox("Patient details", this);
    form_ = new QFormLayout(box);
    idEdit_ = new QLineEdit;
    idEdit_->setPlaceholderText("leave blank for automatic ID");
    idEdit_->setToolTip("Unique patient ID (whole number). Leave blank to generate one automatically.");
    nameEdit_ = new QLineEdit;
    nameEdit_->setToolTip("Full name of the patient.");
    ageEdit_ = new QLineEdit;
    ageEdit_->setPlaceholderText("0 - 120");
    ageEdit_->setToolTip("Age in years (0 to 120).");
    contactEdit_ = new QLineEdit;
    contactEdit_->setToolTip("Phone number or e-mail address.");
    categoryCombo_ = new QComboBox;
    categoryCombo_->addItem("General", static_cast<int>(Category::General));
    categoryCombo_->addItem("Emergency", static_cast<int>(Category::Emergency));
    categoryCombo_->addItem("Follow-Up", static_cast<int>(Category::FollowUp));
    categoryCombo_->setToolTip("Changing the type changes which extra fields are shown.");
    form_->addRow("Patient ID:", idEdit_);
    form_->addRow("Name:", nameEdit_);
    form_->addRow("Age:", ageEdit_);
    form_->addRow("Contact:", contactEdit_);
    form_->addRow("Patient type:", categoryCombo_);
    fixedRows_ = form_->rowCount();
    rebuildDetailFields(Category::General);

    // ---------- buttons (QGridLayout) ----------
    addBtn_ = new QPushButton("Add Patient");
    updateBtn_ = new QPushButton("Update Selected");
    deleteBtn_ = new QPushButton("Delete Patient");
    clearBtn_ = new QPushButton("Clear Form");
    addBtn_->setToolTip("Register a new patient with the details above.");
    updateBtn_->setToolTip("Save the edited details of the patient selected in the table.");
    deleteBtn_->setToolTip("Delete the selected patient (can be undone from the Dashboard / Edit menu).");
    clearBtn_->setToolTip("Empty the form so a new patient can be entered.");
    QGridLayout* buttons = new QGridLayout;
    buttons->addWidget(addBtn_, 0, 0);
    buttons->addWidget(updateBtn_, 0, 1);
    buttons->addWidget(deleteBtn_, 1, 0);
    buttons->addWidget(clearBtn_, 1, 1);

    QVBoxLayout* left = new QVBoxLayout;
    left->addWidget(box);
    left->addLayout(buttons);
    left->addStretch();

    // ---------- table ----------
    table_ = makeTable({"ID", "Name", "Age", "Contact", "Type", "Priority", "Details"}, this);
    summaryLabel_ = new QLabel("Select a patient to see a summary.");
    summaryLabel_->setWordWrap(true);
    QVBoxLayout* right = new QVBoxLayout;
    right->addWidget(table_);
    right->addWidget(summaryLabel_);

    QHBoxLayout* root = new QHBoxLayout(this);
    root->addLayout(left, 0);
    root->addLayout(right, 1);

    // ---------- events: signals & slots ----------
    connect(addBtn_, &QPushButton::clicked, this, &PatientPanel::onAdd);
    connect(updateBtn_, &QPushButton::clicked, this, &PatientPanel::onUpdate);
    connect(deleteBtn_, &QPushButton::clicked, this, &PatientPanel::onDelete);
    connect(clearBtn_, &QPushButton::clicked, this, &PatientPanel::onClear);
    connect(categoryCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { onCategoryChanged(); });
    connect(table_, &QTableWidget::itemSelectionChanged, this, &PatientPanel::onSelectionChanged);
    connect(idEdit_, &QLineEdit::textChanged, this, &PatientPanel::onIdTextChanged);
    // pressing Enter in the last common field adds the patient
    connect(contactEdit_, &QLineEdit::returnPressed, this, [this]() { if (editingId_ == 0) onAdd(); });

    setEditing(0);
    refresh();
}

Category PatientPanel::currentCategory() const {
    return static_cast<Category>(categoryCombo_->currentData().toInt());
}

void PatientPanel::rebuildDetailFields(Category c) {
    while (form_->rowCount() > fixedRows_) form_->removeRow(fixedRows_);   // remove old dynamic rows
    detailEdits_.clear();
    std::vector<std::string> labels = PatientRecord::labelsFor(c);
    for (size_t i = 0; i < labels.size(); ++i) {
        QLineEdit* e = new QLineEdit;
        e->setToolTip("Extra information for " + categoryCombo_->currentText() + " patients.");
        form_->addRow(qs(labels[i]) + ":", e);
        detailEdits_.append(e);
    }
}

std::vector<std::string> PatientPanel::detailValues() const {
    std::vector<std::string> v;
    for (int i = 0; i < detailEdits_.size(); ++i) v.push_back(ss(detailEdits_[i]->text()));
    return v;
}

void PatientPanel::setEditing(int patientId) {
    editingId_ = patientId;
    bool editing = patientId != 0;
    idEdit_->setReadOnly(editing);          // the ID of an existing patient cannot change
    addBtn_->setEnabled(!editing);
    updateBtn_->setEnabled(editing);
    deleteBtn_->setEnabled(editing);
    categoryCombo_->setEnabled(!editing);
}

void PatientPanel::onCategoryChanged() { rebuildDetailFields(currentCategory()); }

void PatientPanel::onIdTextChanged(const QString& text) {
    // text-field event: live duplicate-ID feedback while typing
    bool ok = false;
    int id = text.trimmed().toInt(&ok);
    bool duplicate = editingId_ == 0 && ok && id > 0 && mgr_.patient(id) != nullptr;
    idEdit_->setStyleSheet(duplicate ? "background-color:#ffd6d6; color:black;" : "");
    idEdit_->setToolTip(duplicate ? "Error: Patient ID already exists."
                                  : "Unique patient ID (whole number). Leave blank to generate one automatically.");
}

void PatientPanel::onAdd() {
    try {
        int id = idEdit_->text().trimmed().isEmpty()
                     ? 0
                     : parseInt<utils::InvalidPatientException>(idEdit_->text(), "Patient ID must be a whole number.");
        int age = parseInt<utils::InvalidAgeException>(ageEdit_->text(),
                                                       "Patient age must be a whole number between 0 and 120.");
        int newId = mgr_.addPatient(currentCategory(), id, ss(nameEdit_->text()), age, ss(contactEdit_->text()),
                                    detailValues());
        emit statusMessage("Patient " + QString::number(newId) + " registered.");
        onClear();
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void PatientPanel::onUpdate() {
    if (editingId_ == 0) return;
    try {
        int age = parseInt<utils::InvalidAgeException>(ageEdit_->text(),
                                                       "Patient age must be a whole number between 0 and 120.");
        mgr_.editPatient(editingId_, ss(nameEdit_->text()), age, ss(contactEdit_->text()), detailValues());
        emit statusMessage("Patient " + QString::number(editingId_) + " updated.");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void PatientPanel::onDelete() {
    if (editingId_ == 0) return;
    PatientRecord* p = mgr_.patient(editingId_);
    if (!p) return;
    QMessageBox::StandardButton answer = QMessageBox::question(
        this, "Delete patient",
        "Delete " + qs(p->name()) + " (ID " + QString::number(p->id()) + ") and all their appointments?\n"
        "You can undo this from the Dashboard or the Edit menu.");
    if (answer != QMessageBox::Yes) return;
    try {
        int id = editingId_;
        mgr_.deletePatient(id);
        emit statusMessage("Patient " + QString::number(id) + " deleted.");
        onClear();
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void PatientPanel::onClear() {
    table_->blockSignals(true);
    table_->clearSelection();
    table_->blockSignals(false);
    setEditing(0);
    idEdit_->clear();
    nameEdit_->clear();
    ageEdit_->clear();
    contactEdit_->clear();
    categoryCombo_->setCurrentIndex(0);
    rebuildDetailFields(currentCategory());
    summaryLabel_->setText("Select a patient to see a summary.");
}

void PatientPanel::onSelectionChanged() {
    int id = selectedId(table_);
    if (id <= 0) return;
    PatientRecord* p = mgr_.patient(id);
    if (!p) return;
    loadIntoForm(*p);
    mgr_.recordAccess(id);   // feeds the "frequently accessed" cache
}

void PatientPanel::loadIntoForm(const PatientRecord& p) {
    setEditing(p.id());
    idEdit_->setText(QString::number(p.id()));
    nameEdit_->setText(qs(p.name()));
    ageEdit_->setText(QString::number(p.age()));
    contactEdit_->setText(qs(p.contact()));
    categoryCombo_->blockSignals(true);
    categoryCombo_->setCurrentIndex(categoryCombo_->findData(static_cast<int>(p.category())));
    categoryCombo_->blockSignals(false);
    rebuildDetailFields(p.category());
    std::vector<std::string> d = p.details();
    for (int i = 0; i < detailEdits_.size() && i < static_cast<int>(d.size()); ++i)
        detailEdits_[i]->setText(qs(d[static_cast<size_t>(i)]));
    summaryLabel_->setText(qs(mgr_.processPatient(p)));   // polymorphic description
}

void PatientPanel::refresh() {
    controller::PatientList list = mgr_.patients();
    table_->blockSignals(true);
    table_->setRowCount(0);
    table_->setRowCount(static_cast<int>(list.size()));
    int keepRow = -1;
    for (size_t i = 0; i < list.size(); ++i) {
        const PatientRecord* p = list[i];
        std::vector<std::string> d = p->details();
        QString details;
        for (size_t k = 0; k < d.size(); ++k) details += (k ? " | " : "") + qs(d[k]);
        setTableRow(table_, static_cast<int>(i),
                    {QString::number(p->id()), qs(p->name()), QString::number(p->age()), qs(p->contact()),
                     qs(p->categoryName()), friendlyPriority(*p), details},
                    categoryColor(p->category()), p->category() == Category::Emergency);
        if (p->id() == editingId_) keepRow = static_cast<int>(i);
    }
    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
    if (keepRow >= 0) table_->selectRow(keepRow);
    table_->blockSignals(false);
    if (editingId_ != 0 && keepRow < 0) onClear();   // the patient was removed elsewhere (e.g. Undo/Load)
}

} // namespace gui
} // namespace scapms
