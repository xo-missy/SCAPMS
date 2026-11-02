#include "QueuePanel.h"
#include "GuiUtils.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <ctime>

namespace scapms {
namespace gui {

using namespace model;

QueuePanel::QueuePanel(controller::ClinicManager& mgr, QWidget* parent) : QWidget(parent), mgr_(mgr) {
    patientIdEdit_ = new QLineEdit;
    patientIdEdit_->setPlaceholderText("patient ID");
    patientIdEdit_->setToolTip("ID of the patient who has arrived and is waiting.");
    QPushButton* addBtn = new QPushButton("Add to Queue");
    QPushButton* nextBtn = new QPushButton("Call Next Patient");
    addBtn->setToolTip("Put the patient at the back of the waiting queue.");
    nextBtn->setToolTip("Serve the patient who has been waiting the longest.");
    nowServing_ = new QLabel("Now serving: nobody yet");
    QFont f = nowServing_->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 2);
    nowServing_->setFont(f);

    QHBoxLayout* top = new QHBoxLayout;
    top->addWidget(new QLabel("Patient ID:"));
    top->addWidget(patientIdEdit_);
    top->addWidget(addBtn);
    top->addStretch();
    top->addWidget(nextBtn);

    table_ = makeTable({"Position", "ID", "Name", "Type", "Priority", "Waiting (min)"}, this);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->addLayout(top);
    root->addWidget(nowServing_);
    root->addWidget(table_, 1);

    connect(addBtn, &QPushButton::clicked, this, &QueuePanel::onAdd);
    connect(patientIdEdit_, &QLineEdit::returnPressed, this, &QueuePanel::onAdd);
    connect(nextBtn, &QPushButton::clicked, this, &QueuePanel::onCallNext);
    refresh();
}

void QueuePanel::onAdd() {
    try {
        int id = parseInt<utils::ClinicException>(patientIdEdit_->text(), "Patient ID must be a whole number.");
        mgr_.enqueue(id);
        patientIdEdit_->clear();
        emit statusMessage("Patient " + QString::number(id) + " added to the waiting queue.");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void QueuePanel::onCallNext() {
    try {
        PatientRecord* p = mgr_.callNext();
        QString text = qs(mgr_.processPatient(*p));   // polymorphic summary
        nowServing_->setText("Now serving: " + qs(p->name()) + " (ID " + QString::number(p->id()) + ")");
        emit statusMessage("Calling " + qs(p->name()) + ".");
        emit dataChanged();
        QMessageBox::information(this, "Next patient", text);
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void QueuePanel::refresh() {
    std::vector<controller::QueueEntry> q = mgr_.queueSnapshot();
    table_->setRowCount(static_cast<int>(q.size()));
    std::time_t now = std::time(nullptr);
    for (size_t i = 0; i < q.size(); ++i) {
        const PatientRecord* p = mgr_.patient(q[i].patientId);
        if (!p) continue;
        long minutes = static_cast<long>(std::difftime(now, q[i].joined) / 60.0);
        setTableRow(table_, static_cast<int>(i),
                    {QString::number(i + 1), QString::number(p->id()), qs(p->name()), qs(p->categoryName()),
                     friendlyPriority(*p), QString::number(minutes)},
                    categoryColor(p->category()));
    }
    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
}

} // namespace gui
} // namespace scapms
