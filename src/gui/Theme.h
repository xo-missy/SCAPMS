#pragma once
// Application-wide Qt style sheet (colours, spacing, rounded corners).
// Only appearance lives here - no behaviour - so the GUI logic stays untouched.
#include <QApplication>
#include <QFont>

namespace scapms {
namespace gui {

inline void applyTheme(QApplication& app) {
    app.setStyle("Fusion");
    QFont f = app.font();
    f.setPointSize(10);
    app.setFont(f);

    app.setStyleSheet(R"(
        QWidget { background-color: #F3F6F9; color: #1F2937; }
        QMainWindow, QDialog { background-color: #F3F6F9; }

        /* ---- menu bar / status bar ---- */
        QMenuBar { background: #0E7490; color: white; padding: 2px; }
        QMenuBar::item { background: transparent; padding: 5px 12px; border-radius: 4px; }
        QMenuBar::item:selected { background: #155E75; }
        QMenu { background: white; border: 1px solid #CBD5E1; padding: 4px; }
        QMenu::item { padding: 6px 24px; border-radius: 4px; }
        QMenu::item:selected { background: #CFFAFE; color: #164E63; }
        QStatusBar { background: #E2E8F0; color: #334155; }
        QStatusBar::item { border: none; }

        /* ---- tabs ---- */
        QTabWidget::pane { border: 1px solid #CBD5E1; border-radius: 8px; background: #F3F6F9; top: -1px; }
        QTabBar::tab { background: #E2E8F0; color: #475569; padding: 9px 20px; margin-right: 3px;
                       border-top-left-radius: 8px; border-top-right-radius: 8px; }
        QTabBar::tab:selected { background: #0E7490; color: white; font-weight: bold; }
        QTabBar::tab:hover:!selected { background: #CFFAFE; color: #164E63; }

        /* ---- group boxes ---- */
        QGroupBox { background: white; border: 1px solid #CBD5E1; border-radius: 8px;
                    margin-top: 14px; padding: 12px 8px 8px 8px; font-weight: bold; color: #0E7490; }
        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left;
                           left: 12px; padding: 0 6px; background: #F3F6F9; }
        QGroupBox QLabel { background: transparent; color: #1F2937; font-weight: normal; }
        QGroupBox QCheckBox, QGroupBox QRadioButton { background: transparent; font-weight: normal; }

        /* ---- inputs ---- */
        QLineEdit, QSpinBox, QDateEdit, QTimeEdit, QComboBox, QTextEdit, QPlainTextEdit {
            background: white; border: 1px solid #CBD5E1; border-radius: 6px; padding: 5px 8px;
            selection-background-color: #0E7490; selection-color: white; }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QTextEdit:focus { border: 2px solid #0E7490; padding: 4px 7px; }
        QLineEdit:disabled, QComboBox:disabled { background: #E2E8F0; color: #94A3B8; }
        QComboBox QAbstractItemView { background: white; border: 1px solid #CBD5E1;
                                      selection-background-color: #CFFAFE; selection-color: #164E63; }

        /* ---- buttons ---- */
        QPushButton { background: #0E7490; color: white; border: none; border-radius: 6px;
                      padding: 7px 16px; font-weight: bold; }
        QPushButton:hover { background: #155E75; }
        QPushButton:pressed { background: #164E63; }
        QPushButton:disabled { background: #CBD5E1; color: #94A3B8; }

        /* ---- tables ---- */
        QTableWidget { background: white; border: 1px solid #CBD5E1; border-radius: 8px;
                       gridline-color: #E2E8F0; selection-background-color: #0E7490; selection-color: white; }
        QTableWidget::item { padding: 5px 6px; }
        QHeaderView::section { background: #164E63; color: white; padding: 8px 6px; border: none;
                               border-right: 1px solid #0E7490; font-weight: bold; }
        QListWidget { background: white; border: 1px solid #CBD5E1; border-radius: 8px; padding: 4px; }
        QListWidget::item { padding: 5px 6px; border-radius: 4px; }
        QListWidget::item:selected { background: #0E7490; color: white; }

        /* ---- dashboard cards ---- */
        QFrame#statCard { background: white; border: 1px solid #CBD5E1; border-radius: 10px; }
        QFrame#statCard[accent="general"]   { border-left: 6px solid #16A34A; }
        QFrame#statCard[accent="emergency"] { border-left: 6px solid #DC2626; }
        QFrame#statCard[accent="followup"]  { border-left: 6px solid #2563EB; }
        QFrame#statCard[accent="total"]     { border-left: 6px solid #0E7490; }
        QFrame#statCard[accent="queue"]     { border-left: 6px solid #D97706; }
        QFrame#statCard[accent="pending"]   { border-left: 6px solid #7C3AED; }
        QLabel#statValue { background: transparent; color: #0F172A; }
        QLabel#statTitle { background: transparent; color: #64748B; }

        /* ---- scrollbars / tooltips ---- */
        QScrollBar:vertical { background: transparent; width: 12px; margin: 0; }
        QScrollBar::handle:vertical { background: #CBD5E1; border-radius: 6px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #94A3B8; }
        QScrollBar:horizontal { background: transparent; height: 12px; margin: 0; }
        QScrollBar::handle:horizontal { background: #CBD5E1; border-radius: 6px; min-width: 30px; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
        QToolTip { background: #164E63; color: white; border: none; padding: 5px 8px; }
        QMessageBox { background: white; }
    )");
}

} // namespace gui
} // namespace scapms
