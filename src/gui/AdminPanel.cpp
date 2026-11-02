#include "AdminPanel.h"
#include "GuiUtils.h"
#include "../utils/DateUtils.h"
#include <QComboBox>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace scapms {
namespace gui {

using namespace model;

QWidget* AdminPanel::statCardWidget(const QString& title, QLabel** valueOut) {
    QFrame* card = new QFrame;
    card->setObjectName("statCard");
    card->setFrameShape(QFrame::StyledPanel);
    // accent colour strip (see Theme.h) - matches the category colours used in the tables
    if (title == "General") card->setProperty("accent", "general");
    else if (title == "Emergency") card->setProperty("accent", "emergency");
    else if (title == "Follow-Up") card->setProperty("accent", "followup");
    else if (title == "Waiting in queue") card->setProperty("accent", "queue");
    else if (title == "Pending appointments") card->setProperty("accent", "pending");
    else card->setProperty("accent", "total");
    QVBoxLayout* l = new QVBoxLayout(card);
    QLabel* t = new QLabel(title);
    t->setObjectName("statTitle");
    t->setAlignment(Qt::AlignCenter);
    QLabel* v = new QLabel("0");
    v->setObjectName("statValue");
    v->setAlignment(Qt::AlignCenter);
    QFont f = v->font();
    f.setPointSize(f.pointSize() + 12);
    f.setBold(true);
    v->setFont(f);
    l->addWidget(v);
    l->addWidget(t);
    *valueOut = v;
    return card;
}

AdminPanel::AdminPanel(controller::ClinicManager& mgr, QWidget* parent) : QWidget(parent), mgr_(mgr) {
    // ----- dashboard cards (QGridLayout) -----
    QGridLayout* grid = new QGridLayout;
    grid->addWidget(statCardWidget("Registered patients", &totalValue_), 0, 0);
    grid->addWidget(statCardWidget("General", &generalValue_), 0, 1);
    grid->addWidget(statCardWidget("Emergency", &emergencyValue_), 0, 2);
    grid->addWidget(statCardWidget("Follow-Up", &followUpValue_), 1, 0);
    grid->addWidget(statCardWidget("Waiting in queue", &waitingValue_), 1, 1);
    grid->addWidget(statCardWidget("Pending appointments", &pendingValue_), 1, 2);

    // ----- undo + recent patients -----
    QGroupBox* undoBox = new QGroupBox("Undo last administrative operation");
    QVBoxLayout* undoLayout = new QVBoxLayout(undoBox);
    lastOperation_ = new QLabel;
    lastOperation_->setWordWrap(true);
    undoBtn_ = new QPushButton("Undo");
    undoBtn_->setToolTip("Reverses the most recent delete-patient, modify-patient or remove-appointment (Ctrl+Z).");
    undoLayout->addWidget(new QLabel("Most recent operation:"));
    undoLayout->addWidget(lastOperation_);
    undoLayout->addWidget(undoBtn_);

    QGroupBox* recentBox = new QGroupBox("Frequently accessed patients");
    QVBoxLayout* recentLayout = new QVBoxLayout(recentBox);
    recentList_ = new QListWidget;
    recentList_->setToolTip("The 10 most recently opened or searched patients (fixed-size array cache).");
    recentLayout->addWidget(recentList_);

    QVBoxLayout* leftCol = new QVBoxLayout;
    leftCol->addWidget(undoBox);
    leftCol->addWidget(recentBox, 1);

    // ----- reports -----
    QGroupBox* reportBox = new QGroupBox("Reports");
    QVBoxLayout* reportLayout = new QVBoxLayout(reportBox);
    reportCombo_ = new QComboBox;
    reportCombo_->addItems({"Most Frequently Attended Patients", "Patients with Pending Appointments",
                            "Patient Category Distribution", "Daily Appointment Summary"});
    reportDateEdit_ = new QLineEdit(qs(utils::DateUtils::today()));
    reportDateEdit_->setToolTip("Date (YYYY-MM-DD) for the daily appointment summary.");
    QPushButton* generateBtn = new QPushButton("Generate");
    QHBoxLayout* reportControls = new QHBoxLayout;
    reportControls->addWidget(reportCombo_, 1);
    reportControls->addWidget(new QLabel("Date:"));
    reportControls->addWidget(reportDateEdit_);
    reportControls->addWidget(generateBtn);
    reportView_ = new QPlainTextEdit;
    reportView_->setReadOnly(true);
    reportView_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    reportLayout->addLayout(reportControls);
    reportLayout->addWidget(reportView_, 1);

    QHBoxLayout* lower = new QHBoxLayout;
    lower->addLayout(leftCol, 1);
    lower->addWidget(reportBox, 2);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->addLayout(grid);
    root->addLayout(lower, 1);

    connect(undoBtn_, &QPushButton::clicked, this, &AdminPanel::onUndo);
    connect(generateBtn, &QPushButton::clicked, this, &AdminPanel::onGenerateReport);
    connect(reportCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { onGenerateReport(); });
    connect(reportDateEdit_, &QLineEdit::returnPressed, this, &AdminPanel::onGenerateReport);

    refresh();
}

void AdminPanel::onUndo() {
    try {
        QString msg = qs(mgr_.undo());
        emit statusMessage(msg);
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void AdminPanel::onGenerateReport() {
    try {
        std::string text;
        switch (reportCombo_->currentIndex()) {
        case 0: text = mgr_.reportMostFrequentlyAttended(); break;
        case 1: text = mgr_.reportPendingAppointments(); break;
        case 2: text = mgr_.reportCategoryDistribution(); break;
        default: text = mgr_.reportDailySummary(ss(reportDateEdit_->text().trimmed())); break;
        }
        reportView_->setPlainText(qs(text));
    } catch (const std::exception& e) {
        reportView_->setPlainText("Error: " + qs(e.what()));
    }
}

void AdminPanel::refresh() {
    controller::DashboardStats s = mgr_.stats();
    totalValue_->setText(QString::number(s.total));
    generalValue_->setText(QString::number(s.general));
    emergencyValue_->setText(QString::number(s.emergency));
    followUpValue_->setText(QString::number(s.followUp));
    waitingValue_->setText(QString::number(s.waiting));
    pendingValue_->setText(QString::number(s.pendingAppointments));

    lastOperation_->setText(qs(mgr_.lastOperation()));
    undoBtn_->setEnabled(mgr_.canUndo());

    recentList_->clear();
    std::vector<PatientRecord*> recent = mgr_.recentPatients();
    for (size_t i = 0; i < recent.size(); ++i)
        recentList_->addItem(QString::number(recent[i]->id()) + "  " + qs(recent[i]->name()) + "  (" +
                             qs(recent[i]->categoryName()) + ")");
    onGenerateReport();
}

} // namespace gui
} // namespace scapms
