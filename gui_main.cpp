#include <QApplication>
#include <QMessageBox>
#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyleSheet(
    "QMainWindow, QWidget#contentWidget { background: #f0f1f4; }"
    "QFrame#sidebarFrame { background: white; border-right: 1px solid #e5e5e5; }"
    "QPushButton { text-align: left; padding: 8px 12px; border: none; border-radius: 6px; background: transparent; }"
    "QPushButton:checked { background: #2d6cdf; color: white; }"
    "QPushButton:hover:!checked { background: #f0f1f4; }"
    "QListWidget { border: none; }"
    );

    Database plannerDB("planner.db");
    if (!plannerDB.isOpen()) {
        QMessageBox::critical(nullptr, "Database error",
                              "The planner database could not be opened or initialized.");
        return 1;
    }

    MainWindow window(plannerDB);
    window.show();
    return app.exec();
}