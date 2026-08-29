#pragma once

#include <QMainWindow>
#include <QPoint>

#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class WelcomeScreen; }
namespace Ui { class LoginScreen; }
namespace Ui { class SignUpScreen; }
QT_END_NAMESPACE

class QListWidgetItem;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Database& database, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showLoginScreen();
    void showSignupScreen();
    void showWelcomeScreen();
    void onLoginSubmitted();
    void onSignupSubmitted();
    void onLogoutClicked();
    void onAddTaskClicked();
    void onTaskItemChanged(QListWidgetItem* item);
    void onTaskListContextMenuRequested(const QPoint& position);

private:
    Ui::MainWindow *ui;
    Ui::WelcomeScreen *welcomeUi;
    Ui::LoginScreen *loginUi;
    Ui::SignUpScreen *signupUi;
    QStackedWidget *screenStack;
    Database& database;
    userInfo currentUser;

    void loadCurrentUser();
    bool saveCurrentTasks();
    bool saveCalendarAndSchedule();
    void updateDashboardUser();
    void updateGreeting();
    void loadScheduleDisplay();
    void loadCalendarDisplay();
    void addTaskToList(const QString& text, bool done = false);
    void onTaskDoubleClicked(QListWidgetItem* item);
    void onAddScheduleItemClicked();
    void onScheduleItemDoubleClicked(QListWidgetItem* item);
    void onAddEventClicked();
};