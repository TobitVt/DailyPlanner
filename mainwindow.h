#pragma once

#include <QMainWindow>
#include <QPoint>

#include "database.h"
#include "weather.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
namespace Ui
{
    class WelcomeScreen;
}
namespace Ui
{
    class LoginScreen;
}
namespace Ui
{
    class SignUpScreen;
}
QT_END_NAMESPACE

class QListWidgetItem;
class QStackedWidget;
class QListWidget;
class QNetworkAccessManager;
class QTimer;
class QLabel;
class QLineEdit;
class QCalendarWidget;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Database &database, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showLoginScreen();
    void showSignupScreen();
    void showWelcomeScreen();
    void onLoginSubmitted();
    void onSignupSubmitted();
    void onLogoutClicked();
    void onAddTaskClicked();
    void onTaskItemChanged(QListWidgetItem *item);
    void onTaskListContextMenuRequested(const QPoint &position);
    void onScheduleItemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    Ui::WelcomeScreen *welcomeUi;
    Ui::LoginScreen *loginUi;
    Ui::SignUpScreen *signupUi;
    QStackedWidget *screenStack;
    QWidget *calendarPage = nullptr;
    QWidget *schedulePage = nullptr;
    QWidget *remindersPage = nullptr;
    QWidget *tasksPage = nullptr;
    QWidget *weatherPage = nullptr;
    QWidget *analysisPage = nullptr;
    QWidget *settingsPage = nullptr;
    QListWidget *calendarList = nullptr;
    QListWidget *fullScheduleList = nullptr;
    QListWidget *remindersList = nullptr;
    QListWidget *tasksPageList = nullptr;
    QCalendarWidget *calendarWidget = nullptr;
    QCheckBox *darkModeCheck = nullptr;
    QLabel *weatherPageLabel = nullptr;
    QLabel *analysisLabel = nullptr;
    QLineEdit *settingsEmailEdit = nullptr;
    QLineEdit *settingsHomeEdit = nullptr;
    QLineEdit *settingsWorkEdit = nullptr;
    QNetworkAccessManager *weatherManager = nullptr;
    QTimer *reminderTimer = nullptr;
    Database &database;
    userInfo currentUser;

    void loadCurrentUser();
    bool saveCurrentTasks();
    bool saveCalendarAndSchedule();
    void updateDashboardUser();
    void updateGreeting();
    void loadScheduleDisplay();
    void loadCalendarDisplay();
    void addTaskToList(const QString &text, bool done = false);
    void onTaskDoubleClicked(QListWidgetItem *item);
    void onAddScheduleItemClicked();
    void onAddEventClicked();
    void showCalendarPage();
    void showSchedulePage();
    void showRemindersPage();
    void showDashboardPage();
    void showTasksPage();
    void showWeatherPage();
    void showAnalysisPage();
    void showSettingsPage();
    void importCalendar();
    void addReminder();
    void loadCalendarPage();
    void loadFullSchedulePage();
    void loadRemindersPage();
    void requestWeather(const QString &city, bool home);
    void checkReminders();
    void onCalendarItemDoubleClicked(QListWidgetItem *item);
    void exportData();
    void importData();
    void saveSettings();
    void detectLocation();
    void loadTasksPage();
    void buildDailyAnalysis();
    QString weatherRecommendation(const WeatherData &data) const;
};