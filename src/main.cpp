#include <QApplication>
#include "gui/MainWindow.h"
#include "gui/Theme.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    scapms::gui::applyTheme(app);
    app.setApplicationName("SCAPMS");

    scapms::gui::MainWindow window;
    window.tryLoadDefaultData();
    window.show();
    return app.exec();
}
