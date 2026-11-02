#include "SearchPanel.h"
#include "GuiUtils.h"
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
using namespace controller;

SearchPanel::SearchPanel(ClinicManager& mgr, QWidget* parent) : QWidget(parent), mgr_(mgr), showAll_(true) {
    // ----- search group (QFormLayout) -----
    QGroupBox* searchBox = new QGroupBox("Search", this);
    QFormLayout* searchForm = new QFormLayout(searchBox);
    fieldCombo_ = new QComboBox;
    fieldCombo_->addItem("Patient ID", static_cast<int>(SearchField::Id));
    fieldCombo_->addItem("Patient Name", static_cast<int>(SearchField::Name));
    fieldCombo_->addItem("Category", static_cast<int>(SearchField::Category));
    queryEdit_ = new QLineEdit;
    searchAlgoCombo_ = new QComboBox;
    searchAlgoCombo_->addItem("Automatic (best for the data)", static_cast<int>(SearchAlgorithm::Auto));
    searchAlgoCombo_->addItem("Linear Search", static_cast<int>(SearchAlgorithm::Linear));
    searchAlgoCombo_->addItem("Binary Search (ID, sorted data)", static_cast<int>(SearchAlgorithm::Binary));
    searchAlgoCombo_->addItem("Recursive Search (name)", static_cast<int>(SearchAlgorithm::Recursive));
    searchAlgoCombo_->setToolTip("Automatic picks binary search when the records are sorted by ID, otherwise linear.");
    QPushButton* searchBtn = new QPushButton("Search");
    QPushButton* showAllBtn = new QPushButton("Show All");
    QHBoxLayout* searchButtons = new QHBoxLayout;
    searchButtons->addWidget(searchBtn);
    searchButtons->addWidget(showAllBtn);
    searchForm->addRow("Search by:", fieldCombo_);
    searchForm->addRow("Keyword:", queryEdit_);
    searchForm->addRow("Algorithm:", searchAlgoCombo_);
    searchForm->addRow(searchButtons);

    // ----- sort group (QFormLayout) -----
    QGroupBox* sortBox = new QGroupBox("Sort", this);
    QFormLayout* sortForm = new QFormLayout(sortBox);
    sortAlgoCombo_ = new QComboBox;
    sortAlgoCombo_->addItem("Selection Sort", static_cast<int>(SortAlgorithm::Selection));
    sortAlgoCombo_->addItem("Insertion Sort", static_cast<int>(SortAlgorithm::Insertion));
    sortAlgoCombo_->addItem("Merge Sort", static_cast<int>(SortAlgorithm::Merge));
    sortAlgoCombo_->addItem("Quick Sort", static_cast<int>(SortAlgorithm::Quick));
    sortAlgoCombo_->setToolTip("The sorting algorithm that will be used (all four are implemented by hand).");
    sortKeyCombo_ = new QComboBox;
    sortKeyCombo_->addItem("Patient Name", static_cast<int>(SortKey::Name));
    sortKeyCombo_->addItem("Patient ID", static_cast<int>(SortKey::Id));
    sortKeyCombo_->addItem("Age", static_cast<int>(SortKey::Age));
    sortKeyCombo_->addItem("Appointment Date", static_cast<int>(SortKey::AppointmentDate));
    QPushButton* sortBtn = new QPushButton("SORT");
    sortBtn->setToolTip("Sort all patient records using the chosen algorithm and field.");
    sortForm->addRow("Sort Algorithm:", sortAlgoCombo_);
    sortForm->addRow("Sort By:", sortKeyCombo_);
    sortForm->addRow(sortBtn);

    QHBoxLayout* controls = new QHBoxLayout;
    controls->addWidget(searchBox, 1);
    controls->addWidget(sortBox, 1);

    explanation_ = new QLabel;
    explanation_->setWordWrap(true);
    resultInfo_ = new QLabel;
    resultInfo_->setWordWrap(true);
    table_ = makeTable({"ID", "Name", "Age", "Type", "Priority", "Next pending appointment"}, this);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->addLayout(controls);
    root->addWidget(explanation_);
    root->addWidget(resultInfo_);
    root->addWidget(table_, 1);

    connect(searchBtn, &QPushButton::clicked, this, &SearchPanel::onSearch);
    connect(queryEdit_, &QLineEdit::returnPressed, this, &SearchPanel::onSearch);   // text-field event
    connect(showAllBtn, &QPushButton::clicked, this, &SearchPanel::onShowAll);
    connect(sortBtn, &QPushButton::clicked, this, &SearchPanel::onSort);
    connect(fieldCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { onFieldChanged(); });
    connect(sortAlgoCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { onSortAlgorithmChanged(); });

    onFieldChanged();
    onSortAlgorithmChanged();
    refresh();
}

void SearchPanel::focusSearchField() {
    queryEdit_->setFocus();
    queryEdit_->selectAll();
}

void SearchPanel::onFieldChanged() {
    SearchField f = static_cast<SearchField>(fieldCombo_->currentData().toInt());
    if (f == SearchField::Id) queryEdit_->setPlaceholderText("e.g. 1003");
    else if (f == SearchField::Name) queryEdit_->setPlaceholderText("part of a name, e.g. ade");
    else queryEdit_->setPlaceholderText("General, Emergency or Follow-Up");
    queryEdit_->setToolTip(queryEdit_->placeholderText());
}

void SearchPanel::onSortAlgorithmChanged() {
    SortAlgorithm a = static_cast<SortAlgorithm>(sortAlgoCombo_->currentData().toInt());
    explanation_->setText("Why this algorithm: " + qs(SortEngine::explain(a, mgr_.patientCount())));
}

void SearchPanel::onSearch() {
    try {
        SearchResult r = mgr_.search(static_cast<SearchField>(fieldCombo_->currentData().toInt()),
                                     ss(queryEdit_->text()),
                                     static_cast<SearchAlgorithm>(searchAlgoCombo_->currentData().toInt()));
        showAll_ = false;
        resultIds_.clear();
        for (size_t i = 0; i < r.matches.size(); ++i) resultIds_.push_back(r.matches[i]->id());
        resultInfo_->setText(QString::number(r.matches.size()) + " match(es) using " + qs(r.algorithm) + ". " +
                             qs(r.reason));
        emit statusMessage("Search finished: " + QString::number(r.matches.size()) + " match(es).");
        refresh();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void SearchPanel::onShowAll() {
    showAll_ = true;
    resultIds_.clear();
    refresh();
}

void SearchPanel::onSort() {
    try {
        SortAlgorithm a = static_cast<SortAlgorithm>(sortAlgoCombo_->currentData().toInt());
        SortKey k = static_cast<SortKey>(sortKeyCombo_->currentData().toInt());
        mgr_.sortPatients(a, k);
        showAll_ = true;
        resultIds_.clear();
        onSortAlgorithmChanged();
        emit statusMessage("Sorted by " + qs(SortEngine::keyName(k)) + " using " + qs(SortEngine::algorithmName(a)) + ".");
        emit dataChanged();
    } catch (const std::exception& e) {
        showError(this, e);
    }
}

void SearchPanel::refresh() {
    PatientList list;
    if (showAll_) {
        list = mgr_.patients();
        if (mgr_.isSorted())
            resultInfo_->setText("Showing all " + QString::number(list.size()) + " patients, sorted by " +
                                 qs(SortEngine::keyName(mgr_.sortedKey())) + ".");
        else
            resultInfo_->setText("Showing all " + QString::number(list.size()) + " patients (unsorted).");
    } else {
        for (size_t i = 0; i < resultIds_.size(); ++i)
            if (PatientRecord* p = mgr_.patient(resultIds_[i])) list.push_back(p);
    }
    table_->setRowCount(static_cast<int>(list.size()));
    for (size_t i = 0; i < list.size(); ++i) {
        const PatientRecord* p = list[i];
        std::string next = p->earliestPendingDate();
        setTableRow(table_, static_cast<int>(i),
                    {QString::number(p->id()), qs(p->name()), QString::number(p->age()), qs(p->categoryName()),
                     friendlyPriority(*p), next == "9999-99-99" ? QString("-") : qs(next)},
                    categoryColor(p->category()));
    }
    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setStretchLastSection(true);
    onSortAlgorithmChanged();
}

} // namespace gui
} // namespace scapms
