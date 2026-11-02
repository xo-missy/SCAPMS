#pragma once
#include <QWidget>
#include <vector>
#include "../controller/ClinicManager.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;

namespace scapms {
namespace gui {

// "Search & Sort" tab.
class SearchPanel : public QWidget {
    Q_OBJECT
public:
    explicit SearchPanel(controller::ClinicManager& mgr, QWidget* parent = nullptr);
    void refresh();
    void focusSearchField();

signals:
    void dataChanged();
    void statusMessage(const QString& message);

private slots:
    void onSearch();
    void onShowAll();
    void onSort();
    void onFieldChanged();
    void onSortAlgorithmChanged();

private:
    controller::ClinicManager& mgr_;
    QComboBox* fieldCombo_;
    QLineEdit* queryEdit_;
    QComboBox* searchAlgoCombo_;
    QComboBox* sortAlgoCombo_;
    QComboBox* sortKeyCombo_;
    QLabel* resultInfo_;
    QLabel* explanation_;
    QTableWidget* table_;
    bool showAll_;
    std::vector<int> resultIds_;
};

} // namespace gui
} // namespace scapms
