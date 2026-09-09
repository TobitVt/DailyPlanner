#include <QApplication>
#include <QMessageBox>
#include "database.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyleSheet(
    "QMainWindow, QWidget#contentWidget { background: #202124; color: #f2f4f8; }"
    "QFrame#sidebarFrame { background: #18191c; color: #f2f4f8; border-right: 1px solid #34373d; }"
    "QFrame#userCardFrame, QFrame#weatherHomeCard, QFrame#weatherWorkCard, QFrame#summaryCard, QFrame#tasksCard, QFrame#scheduleCard, QFrame#reminderCard, QFrame#quickNoteCard { background: #2a2d33; color: #f2f4f8; border: none; border-radius: 10px; }"
    "QLabel { color: #f2f4f8; }"
    "QLabel#userEmailLabel, QLabel#dateLabel, QLabel#weatherHomeDetailLabel, QLabel#weatherWorkDetailLabel { color: #b8c0ce; }"
    "QLabel#weatherHomeDescLabel, QLabel#weatherWorkDescLabel { color: #f2f4f8; font-weight: 700; }"
    "QLabel#userAvatarLabel { background: #4f8cff; color: #ffffff; border-radius: 16px; padding: 4px; qproperty-alignment: AlignCenter; }"
    "QPushButton { background: #2f333a; color: #f2f4f8; text-align: left; padding: 8px 12px; border: 1px solid #464b55; border-radius: 6px; }"
    "QPushButton:checked { background: #4f8cff; color: #ffffff; }"
    "QPushButton:hover:!checked { background: #3a404a; }"
    "QPushButton#searchButton, QPushButton#notificationsButton { background: #3a404a; border: none; border-radius: 16px; min-width: 36px; min-height: 32px; padding: 4px; text-align: center; }"
    "QListWidget, QTextEdit, QLineEdit, QDateTimeEdit, QComboBox { background: #23262b; color: #f2f4f8; border: 1px solid #4a515d; padding: 5px; }"
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