#pragma once
// Small helpers shared by all panels.
#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <exception>
#include <string>
#include "../model/PatientRecord.h"
#include "../utils/Exceptions.h"

namespace scapms {
namespace gui {

inline QString qs(const std::string& s) { return QString::fromStdString(s); }
inline std::string ss(const QString& s) { return s.toStdString(); }

// Input-validation dialog: shown for every ClinicException (and any other std::exception).
inline void showError(QWidget* parent, const std::exception& e) {
    QMessageBox::warning(parent, "SCAPMS - Error", "Error: " + qs(e.what()));
}

// Parses a whole number; throws 'ExceptionType' with 'message' on failure.
template <typename ExceptionType>
int parseInt(const QString& text, const QString& message) {
    bool ok = false;
    int v = text.trimmed().toInt(&ok);
    if (!ok) throw ExceptionType(ss(message));
    return v;
}

inline QTableWidget* makeTable(const QStringList& headers, QWidget* parent = nullptr) {
    QTableWidget* t = new QTableWidget(parent);
    t->setColumnCount(headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setVisible(false);
    t->horizontalHeader()->setStretchLastSection(true);
    t->setWordWrap(false);
    return t;
}

// Custom row presentation: colour tells the patient category / appointment status at a glance.
inline QColor categoryColor(model::Category c) {
    switch (c) {
    case model::Category::Emergency: return QColor(255, 205, 205);   // red   -> high priority
    case model::Category::General: return QColor(220, 244, 220);     // green -> normal
    case model::Category::FollowUp: return QColor(212, 228, 255);    // blue  -> scheduled
    }
    return QColor(255, 255, 255);
}

inline QColor statusColor(model::AppointmentStatus s) {
    switch (s) {
    case model::AppointmentStatus::Pending: return QColor(255, 244, 200);
    case model::AppointmentStatus::Completed: return QColor(220, 244, 220);
    case model::AppointmentStatus::Cancelled: return QColor(228, 228, 228);
    }
    return QColor(255, 255, 255);
}

inline void setTableRow(QTableWidget* t, int row, const QStringList& cells, const QColor& background,
                        bool boldFirstCells = false) {
    for (int c = 0; c < cells.size(); ++c) {
        QTableWidgetItem* item = new QTableWidgetItem(cells[c]);
        item->setBackground(QBrush(background));
        item->setForeground(QBrush(Qt::black));
        if (boldFirstCells && c < 2) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
        t->setItem(row, c, item);
    }
}

// Value in column 'col' of the selected row, or -1 when nothing is selected.
inline int selectedId(QTableWidget* t, int col = 0) {
    QList<QTableWidgetItem*> items = t->selectedItems();
    if (items.isEmpty()) return -1;
    QTableWidgetItem* it = t->item(items.first()->row(), col);
    return it ? it->text().toInt() : -1;
}

inline QString friendlyPriority(const model::PatientRecord& p) { return qs(p.priorityLabel()); }

} // namespace gui
} // namespace scapms
