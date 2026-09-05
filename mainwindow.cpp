#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_welcome.h"
#include "ui_login.h"
#include "ui_signup.h"
#include "weather.h"

#include <QDate>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMenu>
#include <QDateTime>
#include <QMessageBox>
#include <QStackedWidget>
#include <QSignalBlocker>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QTimer>
#include <QDateTime>
#include <QCheckBox>
#include <QFormLayout>
#include <QTextEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCalendarWidget>
#include <QSettings>
#include <functional>

MainWindow::MainWindow(Database &database, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), welcomeUi(nullptr), loginUi(nullptr),
      signupUi(nullptr), screenStack(new QStackedWidget(this)), database(database)
{
    ui->setupUi(this);

    QWidget *dashboard = ui->centralwidget;
    screenStack->addWidget(dashboard);

    QWidget *welcomePage = new QWidget(screenStack);
    welcomeUi = new Ui::WelcomeScreen;
    welcomeUi->setupUi(welcomePage);
    screenStack->addWidget(welcomePage);

    QWidget *loginPage = new QWidget(screenStack);
    loginUi = new Ui::LoginScreen;
    loginUi->setupUi(loginPage);
    screenStack->addWidget(loginPage);

    QWidget *signupPage = new QWidget(screenStack);
    signupUi = new Ui::SignUpScreen;
    signupUi->setupUi(signupPage);
    screenStack->addWidget(signupPage);
    setCentralWidget(screenStack);

    auto createPage = [this](const QString &title, QListWidget **list, const QString &actionText,
                             std::function<void()> action)
    {
        QWidget *page = new QWidget(screenStack);
        auto *layout = new QVBoxLayout(page);
        auto *heading = new QLabel(title, page);
        heading->setStyleSheet("font-size: 22px; font-weight: 700;");
        layout->addWidget(heading);
        auto *buttons = new QHBoxLayout();
        auto *back = new QPushButton("Back to dashboard", page);
        auto *actionButton = new QPushButton(actionText, page);
        buttons->addWidget(back);
        buttons->addWidget(actionButton);
        buttons->addStretch();
        layout->addLayout(buttons);
        *list = new QListWidget(page);
        layout->addWidget(*list);
        connect(back, &QPushButton::clicked, this, &MainWindow::showDashboardPage);
        connect(actionButton, &QPushButton::clicked, this, action);
        screenStack->addWidget(page);
        return page;
    };
    calendarPage = createPage("Calendar", &calendarList, "Import .ics", [this]()
                              { importCalendar(); });
    auto *calendarButtons = qobject_cast<QHBoxLayout *>(calendarPage->layout()->itemAt(1)->layout());
    auto *addEventButton = new QPushButton("Add event", calendarPage);
    calendarButtons->insertWidget(1, addEventButton);
    connect(addEventButton, &QPushButton::clicked, this, &MainWindow::onAddEventClicked);
    auto *monthButton = new QPushButton("Month view", calendarPage);
    auto *weekButton = new QPushButton("Week view", calendarPage);
    calendarButtons->insertWidget(2, monthButton);
    calendarButtons->insertWidget(3, weekButton);
    calendarWidget = new QCalendarWidget(calendarPage);
    calendarWidget->setSelectedDate(QDate::currentDate());
    calendarPage->layout()->addWidget(calendarWidget);
    connect(calendarWidget, &QCalendarWidget::selectionChanged, this, [this]() { loadCalendarPage(); });
    connect(monthButton, &QPushButton::clicked, this, [this]() {
        calendarWidget->setVisible(true);
        loadCalendarPage();
    });
    connect(weekButton, &QPushButton::clicked, this, [this]() {
        calendarWidget->setVisible(false);
        loadCalendarPage();
    });
    schedulePage = createPage("Hourly schedule", &fullScheduleList, "Add schedule item", [this]()
                              { onAddScheduleItemClicked(); });
    remindersPage = createPage("Reminders", &remindersList, "Add reminder", [this]()
                               { addReminder(); });
    tasksPage = createPage("Tasks", &tasksPageList, "Add task", [this]()
                           { onAddTaskClicked(); });
    weatherPage = new QWidget(screenStack);
    auto *weatherLayout = new QVBoxLayout(weatherPage);
    weatherLayout->addWidget(new QLabel("Weather and recommendations", weatherPage));
    weatherPageLabel = new QLabel("Log in to load weather.", weatherPage);
    weatherPageLabel->setWordWrap(true);
    weatherLayout->addWidget(weatherPageLabel);
    auto *weatherBack = new QPushButton("Back to dashboard", weatherPage);
    weatherLayout->addWidget(weatherBack);
    connect(weatherBack, &QPushButton::clicked, this, &MainWindow::showDashboardPage);
    screenStack->addWidget(weatherPage);
    analysisPage = new QWidget(screenStack);
    auto *analysisLayout = new QVBoxLayout(analysisPage);
    analysisLayout->addWidget(new QLabel("Daily analysis", analysisPage));
    analysisLabel = new QLabel(analysisPage);
    analysisLabel->setWordWrap(true);
    analysisLayout->addWidget(analysisLabel);
    auto *analysisBack = new QPushButton("Back to dashboard", analysisPage);
    analysisLayout->addWidget(analysisBack);
    connect(analysisBack, &QPushButton::clicked, this, &MainWindow::showDashboardPage);
    screenStack->addWidget(analysisPage);
    settingsPage = new QWidget(screenStack);
    auto *settingsLayout = new QVBoxLayout(settingsPage);
    settingsLayout->addWidget(new QLabel("Settings", settingsPage));
    auto *form = new QFormLayout();
    settingsEmailEdit = new QLineEdit(settingsPage);
    settingsHomeEdit = new QLineEdit(settingsPage);
    settingsWorkEdit = new QLineEdit(settingsPage);
    form->addRow("Email", settingsEmailEdit);
    form->addRow("Home city", settingsHomeEdit);
    form->addRow("Work city", settingsWorkEdit);
    settingsLayout->addLayout(form);
    auto *saveSettingsButton = new QPushButton("Save settings", settingsPage);
    auto *detectButton = new QPushButton("Detect current location", settingsPage);
    auto *exportButton = new QPushButton("Export my data", settingsPage);
    auto *settingsBack = new QPushButton("Back to dashboard", settingsPage);
    settingsLayout->addWidget(saveSettingsButton);
    settingsLayout->addWidget(detectButton);
    settingsLayout->addWidget(exportButton);
    auto *importButton = new QPushButton("Import my data", settingsPage);
    settingsLayout->addWidget(importButton);
    darkModeCheck = new QCheckBox("Dark mode", settingsPage);
    darkModeCheck->setChecked(QSettings().value("darkMode", false).toBool());
    settingsLayout->addWidget(darkModeCheck);
    settingsLayout->addWidget(settingsBack);
    connect(saveSettingsButton, &QPushButton::clicked, this, &MainWindow::saveSettings);
    connect(detectButton, &QPushButton::clicked, this, &MainWindow::detectLocation);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importData);
    connect(darkModeCheck, &QCheckBox::toggled, this, [this](bool enabled) {
        QSettings().setValue("darkMode", enabled);
        if (enabled) {
            qApp->setStyleSheet("QWidget { background: #202124; color: #eeeeee; } QPushButton { background: #303134; color: #eeeeee; padding: 8px; } QListWidget, QLineEdit, QDateTimeEdit { background: #303134; color: #eeeeee; } QFrame { background: #282a2d; }");
        } else {
            qApp->setStyleSheet("QMainWindow, QWidget#contentWidget { background: #f0f1f4; } QFrame#sidebarFrame { background: white; border-right: 1px solid #e5e5e5; } QPushButton { text-align: left; padding: 8px 12px; border: none; border-radius: 6px; background: transparent; } QPushButton:checked { background: #2d6cdf; color: white; } QPushButton:hover:!checked { background: #f0f1f4; } QListWidget { border: none; }");
        }
    });
    if (darkModeCheck->isChecked()) darkModeCheck->toggled(true);
    connect(settingsBack, &QPushButton::clicked, this, &MainWindow::showDashboardPage);
    screenStack->addWidget(settingsPage);
    weatherManager = new QNetworkAccessManager(this);
    reminderTimer = new QTimer(this);
    reminderTimer->setInterval(30000);
    connect(reminderTimer, &QTimer::timeout, this, &MainWindow::checkReminders);
    reminderTimer->start();

    connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::onAddTaskClicked);
    connect(ui->tasksListWidget, &QListWidget::itemChanged, this, &MainWindow::onTaskItemChanged);
    connect(ui->tasksListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onTaskListContextMenuRequested);
    connect(ui->tasksListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onTaskDoubleClicked);
    ui->tasksListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->scheduleListWidget, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos)
            {
        QListWidgetItem* item = ui->scheduleListWidget->itemAt(pos);
        if (!item) return;
        QMenu menu;
        QAction* removeAction = menu.addAction("Remove");
        if (menu.exec(ui->scheduleListWidget->viewport()->mapToGlobal(pos)) == removeAction) {
            const int hour = item->data(Qt::UserRole).toInt();
            if (hour < 0 || hour > 23) return;
            currentUser.productivity.hourly.removeTask(hour);
            saveCalendarAndSchedule();
            loadScheduleDisplay();
        } });
    ui->scheduleListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

    connect(ui->navDashboardButton, &QPushButton::clicked, this, &MainWindow::showDashboardPage);
    connect(ui->navCalendarButton, &QPushButton::clicked, this, &MainWindow::showCalendarPage);
    connect(ui->navRemindersButton, &QPushButton::clicked, this, &MainWindow::showRemindersPage);
    connect(ui->navTasksButton, &QPushButton::clicked, this, &MainWindow::showTasksPage);
    connect(ui->navWeatherButton, &QPushButton::clicked, this, &MainWindow::showWeatherPage);
    connect(ui->navSettingsButton, &QPushButton::clicked, this, &MainWindow::showSettingsPage);
    connect(ui->viewAllTasksButton, &QPushButton::clicked, this, &MainWindow::showTasksPage);
    connect(ui->viewCalendarButton, &QPushButton::clicked, this, &MainWindow::showCalendarPage);
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::showTasksPage);
    connect(ui->notificationsButton, &QPushButton::clicked, this, &MainWindow::showRemindersPage);
    auto *analysisButton = new QPushButton("Daily analysis", ui->centralwidget);
    auto *topBar = qobject_cast<QHBoxLayout *>(ui->topBarLayout);
    topBar->insertWidget(2, analysisButton);
    connect(analysisButton, &QPushButton::clicked, this, &MainWindow::showAnalysisPage);
    auto *scheduleHeader = qobject_cast<QHBoxLayout *>(ui->scheduleCard->layout()->itemAt(0)->layout());
    auto *scheduleButton = new QPushButton("View schedule", ui->scheduleCard);
    scheduleHeader->insertWidget(1, scheduleButton);
    connect(scheduleButton, &QPushButton::clicked, this, &MainWindow::showSchedulePage);

    connect(welcomeUi->loginButton, &QPushButton::clicked, this, &MainWindow::showLoginScreen);
    connect(welcomeUi->signupButton, &QPushButton::clicked, this, &MainWindow::showSignupScreen);
    connect(loginUi->submitButton, &QPushButton::clicked, this, &MainWindow::onLoginSubmitted);
    connect(loginUi->backButton, &QPushButton::clicked, this, &MainWindow::showWelcomeScreen);
    connect(signupUi->submitButton, &QPushButton::clicked, this, &MainWindow::onSignupSubmitted);
    connect(signupUi->backButton, &QPushButton::clicked, this, &MainWindow::showWelcomeScreen);

    showWelcomeScreen();
}

MainWindow::~MainWindow()
{
    delete welcomeUi;
    delete loginUi;
    delete signupUi;
    delete ui;
}

void MainWindow::showLoginScreen()
{
    screenStack->setCurrentIndex(2);
    loginUi->usernameEdit->setFocus();
}

void MainWindow::showSignupScreen()
{
    screenStack->setCurrentIndex(3);
    signupUi->usernameEdit->setFocus();
}

void MainWindow::showWelcomeScreen()
{
    screenStack->setCurrentIndex(1);
}

void MainWindow::onLoginSubmitted()
{
    const QString username = loginUi->usernameEdit->text().trimmed();
    const QString password = loginUi->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Login", "Enter both a username and password.");
        return;
    }

    userInfo user = database.getAllInfo(username);
    if (user.username.isEmpty() || !PasswordHash::verify(password, user.password))
    {
        QMessageBox::warning(this, "Login", "Incorrect username or password.");
        return;
    }

    currentUser = user;
    loadCurrentUser();
    updateDashboardUser();
    requestWeather(currentUser.homeCity, true);
    requestWeather(currentUser.work, false);
    loadRemindersPage();
    screenStack->setCurrentIndex(0);
    loginUi->passwordEdit->clear();
}

void MainWindow::onSignupSubmitted()
{
    userInfo newUser;
    newUser.username = signupUi->usernameEdit->text().trimmed();
    newUser.password = signupUi->passwordEdit->text();
    newUser.email = signupUi->emailEdit->text().trimmed();
    newUser.homeCity = signupUi->homeCityEdit->text().trimmed();
    newUser.work = signupUi->workCityEdit->text().trimmed();

    if (newUser.username.isEmpty() || newUser.password.isEmpty() ||
        newUser.homeCity.isEmpty() || newUser.work.isEmpty())
    {
        QMessageBox::warning(this, "Create account",
                             "Username, password, home city, and work city are required.");
        return;
    }

    if (!database.createUser(newUser))
    {
        QMessageBox::warning(this, "Create account",
                             "The account could not be created. The username may already exist.");
        return;
    }

    signupUi->usernameEdit->clear();
    signupUi->emailEdit->clear();
    signupUi->passwordEdit->clear();
    signupUi->homeCityEdit->clear();
    signupUi->workCityEdit->clear();
    QMessageBox::information(this, "Create account", "Account created. Please log in.");
    showLoginScreen();
}

void MainWindow::onLogoutClicked()
{
    currentUser = userInfo();
    ui->tasksListWidget->clear();
    ui->scheduleListWidget->clear();
    ui->reminderTimeLabel->setText("--:--");
    ui->reminderTextLabel->setText("No reminders loaded");
    ui->userNameLabel->setText("Guest");
    ui->userEmailLabel->setText("Not signed in");
    ui->greetingLabel->setText("Welcome!");
    ui->summaryTasksLabel->setText("No tasks loaded");
    ui->summaryEventsLabel->setText("No events loaded");
    showWelcomeScreen();
}

void MainWindow::loadCurrentUser()
{
    QSignalBlocker blocker(ui->tasksListWidget);
    ui->tasksListWidget->clear();
    for (const Task &task : currentUser.productivity.todoList.tasks())
    {
        addTaskToList(task.description, task.done);
    }
    loadScheduleDisplay();
    loadCalendarDisplay();
}

void MainWindow::loadScheduleDisplay()
{
    ui->scheduleListWidget->clear();
    const HourlySchedules &schedule = currentUser.productivity.hourly;
    for (int hour = 6; hour < 23; ++hour)
    {
        if (schedule.hasTask(hour))
        {
            const HourlyTask &task = schedule.taskAt(hour);
            QString text = QString::number(hour).rightJustified(2, '0') + ":00 - " + task.description;
            if (task.done)
            {
                text = "✓ " + text;
            }
            auto *item = new QListWidgetItem(text);
            item->setData(Qt::UserRole, hour);
            ui->scheduleListWidget->addItem(item);
        }
    }
    if (ui->scheduleListWidget->count() == 0)
    {
        ui->scheduleListWidget->addItem("No schedule items for today");
    }
    const QVector<CalendarEvent> todayEvents = currentUser.productivity.calendar.eventsOnDate(QDate::currentDate());
    for (const CalendarEvent &event : todayEvents)
    {
        const QString time = event.allDay ? "All day" : event.start.time().toString("hh:mm");
        auto *eventItem = new QListWidgetItem(time + " - " + event.summary);
        eventItem->setData(Qt::UserRole, -1);
        ui->scheduleListWidget->addItem(eventItem);
    }
    loadCalendarDisplay();
}

void MainWindow::loadCalendarDisplay()
{
    const QDate today = QDate::currentDate();
    const QVector<CalendarEvent> todayEvents = currentUser.productivity.calendar.eventsOnDate(today);
    int eventCount = todayEvents.size();
    ui->summaryEventsLabel->setText(QString::number(eventCount) + " event" + (eventCount != 1 ? "s" : "") + " today");
}

bool MainWindow::saveCurrentTasks()
{
    if (currentUser.username.isEmpty())
    {
        return false;
    }
    if (!database.saveTodoList(QStringList(), currentUser))
    {
        QMessageBox::warning(this, "Save error", "Your task changes could not be saved.");
        return false;
    }
    return true;
}

bool MainWindow::saveCalendarAndSchedule()
{
    if (currentUser.username.isEmpty())
    {
        return false;
    }
    if (!database.saveHourlySchedules(QMap<int, QString>(), currentUser))
    {
        QMessageBox::warning(this, "Save error", "Your schedule changes could not be saved.");
        return false;
    }
    if (!database.saveCalendar(currentUser.productivity.calendar.toJson(), currentUser))
    {
        QMessageBox::warning(this, "Save error", "Your calendar changes could not be saved.");
        return false;
    }
    return true;
}

void MainWindow::updateDashboardUser()
{
    ui->userNameLabel->setText(currentUser.username);
    ui->userEmailLabel->setText("Home: " + currentUser.homeCity + " | Work: " + currentUser.work);
    updateGreeting();
    ui->summaryTasksLabel->setText(QString::number(currentUser.productivity.todoList.count()) +
                                   " tasks");
    ui->summaryEventsLabel->setText(QString::number(currentUser.productivity.calendar.events().size()) +
                                    " events");
    if (ui->summaryReminderLabel)
    {
        ui->summaryReminderLabel->setText("No reminders loaded");
    }
}

void MainWindow::updateGreeting()
{
    const int hour = QTime::currentTime().hour();
    QString greeting = hour < 12 ? "Good morning" : hour < 18 ? "Good afternoon"
                                                              : "Good evening";
    ui->greetingLabel->setText(greeting + ", " + currentUser.username + "!");
    ui->dateLabel->setText(QDate::currentDate().toString("dddd, d MMMM yyyy"));
}

void MainWindow::addTaskToList(const QString &text, bool done)
{
    auto *item = new QListWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(done ? Qt::Checked : Qt::Unchecked);
    ui->tasksListWidget->addItem(item);
}

void MainWindow::onAddTaskClicked()
{
    bool accepted = false;
    const QString text = QInputDialog::getText(
                             this, "Add task", "Task description:", QLineEdit::Normal, QString(), &accepted)
                             .trimmed();
    if (!accepted || text.isEmpty())
    {
        return;
    }

    currentUser.productivity.todoList.addTask(text);
    addTaskToList(text);
    saveCurrentTasks();
}

void MainWindow::onTaskListContextMenuRequested(const QPoint &position)
{
    QListWidgetItem *selected = ui->tasksListWidget->itemAt(position);
    if (!selected)
    {
        return;
    }

    QMenu menu(this);
    QAction *removeAction = menu.addAction("Remove task");
    if (menu.exec(ui->tasksListWidget->viewport()->mapToGlobal(position)) == removeAction)
    {
        if (QMessageBox::question(this, "Remove task", "Remove the selected task?") ==
            QMessageBox::Yes)
        {
            const int row = ui->tasksListWidget->row(selected);
            currentUser.productivity.todoList.removeTask(row);
            delete ui->tasksListWidget->takeItem(row);
            saveCurrentTasks();
        }
    }
}

void MainWindow::onTaskItemChanged(QListWidgetItem *item)
{
    bool isDone = (item->checkState() == Qt::Checked);

    // Visually distinguish completed tasks.
    QFont font = item->font();
    font.setStrikeOut(isDone);
    {
        QSignalBlocker blocker(item->listWidget());
        item->setFont(font);
    }
    const int row = ui->tasksListWidget->row(item);
    currentUser.productivity.todoList.setDone(row, isDone);
    saveCurrentTasks();
}

void MainWindow::onTaskDoubleClicked(QListWidgetItem *item)
{
    const int row = ui->tasksListWidget->row(item);
    if (row < 0 || row >= currentUser.productivity.todoList.tasks().size())
    {
        return;
    }

    const Task &task = currentUser.productivity.todoList.tasks()[row];

    QDialog dialog(this);
    dialog.setWindowTitle("Edit task");
    dialog.setMinimumWidth(400);

    QVBoxLayout layout(&dialog);

    layout.addWidget(new QLabel("Description:"));
    QLineEdit descEdit;
    descEdit.setText(task.description);
    layout.addWidget(&descEdit);

    layout.addWidget(new QLabel("Priority:"));
    QComboBox priorityCombo;
    priorityCombo.addItem("Low", static_cast<int>(TaskPriority::Low));
    priorityCombo.addItem("Medium", static_cast<int>(TaskPriority::Medium));
    priorityCombo.addItem("High", static_cast<int>(TaskPriority::High));
    priorityCombo.setCurrentIndex(static_cast<int>(task.priority));
    layout.addWidget(&priorityCombo);

    layout.addWidget(new QLabel("Due date (optional):"));
    QDateTimeEdit dueDateEdit;
    dueDateEdit.setDateTime(task.dueDate.isValid() ? task.dueDate : QDateTime::currentDateTime());
    dueDateEdit.setCalendarPopup(true);
    layout.addWidget(&dueDateEdit);

    QHBoxLayout buttonLayout;
    QPushButton saveBtn("Save");
    QPushButton cancelBtn("Cancel");
    buttonLayout.addWidget(&saveBtn);
    buttonLayout.addWidget(&cancelBtn);
    layout.addLayout(&buttonLayout);

    connect(&saveBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        if (descEdit.text().trimmed().isEmpty())
            return;
        currentUser.productivity.todoList.setDescription(row, descEdit.text().trimmed());
        currentUser.productivity.todoList.setPriority(row, static_cast<TaskPriority>(priorityCombo.currentData().toInt()));
        currentUser.productivity.todoList.setDueDate(row, dueDateEdit.dateTime());
        saveCurrentTasks();
        item->setText(descEdit.text().trimmed());
    }
}

void MainWindow::onAddScheduleItemClicked()
{
    bool accepted = false;
    const int hour = QInputDialog::getInt(
        this, "Add schedule item", "Hour (6-22):", 9, 6, 22, 1, &accepted);
    if (!accepted)
        return;

    const QString description = QInputDialog::getText(
                                    this, "Add schedule item", "Task description:", QLineEdit::Normal, QString(), &accepted)
                                    .trimmed();
    if (!accepted || description.isEmpty())
        return;

    currentUser.productivity.hourly.setTask(hour, description);
    saveCalendarAndSchedule();
    loadScheduleDisplay();
    loadFullSchedulePage();
}

void MainWindow::onAddEventClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add calendar event");
    dialog.setMinimumWidth(400);

    QVBoxLayout layout(&dialog);

    layout.addWidget(new QLabel("Event summary:"));
    QLineEdit summaryEdit;
    layout.addWidget(&summaryEdit);

    layout.addWidget(new QLabel("Description:"));
    QLineEdit descEdit;
    layout.addWidget(&descEdit);

    layout.addWidget(new QLabel("Start date and time:"));
    QDateTimeEdit startEdit;
    startEdit.setDateTime(QDateTime::currentDateTime());
    startEdit.setCalendarPopup(true);
    layout.addWidget(&startEdit);

    layout.addWidget(new QLabel("End date and time:"));
    QDateTimeEdit endEdit;
    endEdit.setDateTime(QDateTime::currentDateTime().addSecs(3600));
    endEdit.setCalendarPopup(true);
    layout.addWidget(&endEdit);

    layout.addWidget(new QLabel("All day event:"));
    QComboBox allDayCombo;
    allDayCombo.addItem("No");
    allDayCombo.addItem("Yes");
    layout.addWidget(&allDayCombo);

    QHBoxLayout buttonLayout;
    QPushButton saveBtn("Create");
    QPushButton cancelBtn("Cancel");
    buttonLayout.addWidget(&saveBtn);
    buttonLayout.addWidget(&cancelBtn);
    layout.addLayout(&buttonLayout);

    connect(&saveBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        if (summaryEdit.text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, "Add event", "Event summary is required.");
            return;
        }

        CalendarEvent event;
        event.summary = summaryEdit.text().trimmed();
        event.description = descEdit.text().trimmed();
        event.start = startEdit.dateTime();
        event.end = endEdit.dateTime();
        event.allDay = (allDayCombo.currentIndex() == 1);

        currentUser.productivity.calendar.addEvent(event);
        saveCalendarAndSchedule();
        loadCalendarDisplay();
        loadCalendarPage();
        QMessageBox::information(this, "Event added", "Calendar event has been added.");
    }
}

void MainWindow::showDashboardPage()
{
    updateDashboardUser();
    screenStack->setCurrentWidget(ui->centralwidget);
}

void MainWindow::showTasksPage()
{
    loadTasksPage();
    screenStack->setCurrentWidget(tasksPage);
}

void MainWindow::showWeatherPage()
{
    weatherPageLabel->setText("Home: " + ui->weatherHomeTempLabel->text() + " - " +
                              ui->weatherHomeDescLabel->text() + "\nWork: " +
                              ui->weatherWorkTempLabel->text() + " - " +
                              ui->weatherWorkDescLabel->text());
    screenStack->setCurrentWidget(weatherPage);
}

void MainWindow::showAnalysisPage()
{
    buildDailyAnalysis();
    screenStack->setCurrentWidget(analysisPage);
}

void MainWindow::showSettingsPage()
{
    settingsEmailEdit->setText(currentUser.email);
    settingsHomeEdit->setText(currentUser.homeCity);
    settingsWorkEdit->setText(currentUser.work);
    screenStack->setCurrentWidget(settingsPage);
}

void MainWindow::showCalendarPage()
{
    loadCalendarPage();
    screenStack->setCurrentWidget(calendarPage);
}

void MainWindow::showSchedulePage()
{
    loadFullSchedulePage();
    screenStack->setCurrentWidget(schedulePage);
}

void MainWindow::showRemindersPage()
{
    loadRemindersPage();
    screenStack->setCurrentWidget(remindersPage);
}

void MainWindow::loadCalendarPage()
{
    if (!calendarList)
        return;
    calendarList->clear();
    QVector<CalendarEvent> visibleEvents;
    if (calendarWidget && !calendarWidget->isVisible()) {
        const QDate selected = calendarWidget->selectedDate();
        const QDate weekStart = selected.addDays(1 - selected.dayOfWeek());
        visibleEvents = currentUser.productivity.calendar.eventsBetween(
            QDateTime(weekStart, QTime(0, 0)), QDateTime(weekStart.addDays(6), QTime(23, 59, 59)));
    } else {
        visibleEvents = currentUser.productivity.calendar.events();
    }
    for (int index = 0; index < visibleEvents.size(); ++index)
    {
        const CalendarEvent &event = visibleEvents.at(index);
        const QString start = event.allDay ? event.start.date().toString("yyyy-MM-dd")
                                           : event.start.toString("yyyy-MM-dd hh:mm");
        auto *item = new QListWidgetItem(start + " - " + event.summary, calendarList);
        int sourceIndex = 0;
        for (; sourceIndex < currentUser.productivity.calendar.events().size(); ++sourceIndex) {
            const CalendarEvent& source = currentUser.productivity.calendar.events().at(sourceIndex);
            if (source.uid == event.uid && source.start == event.start && source.summary == event.summary) break;
        }
        item->setData(Qt::UserRole, sourceIndex);
    }
    if (calendarList->count() == 0)
        calendarList->addItem("No calendar events");
    connect(calendarList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onCalendarItemDoubleClicked, Qt::UniqueConnection);
    calendarList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(calendarList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &position)
            {
        QListWidgetItem* item = calendarList->itemAt(position);
        if (!item) return;
        QMenu menu;
        QAction* remove = menu.addAction("Remove event");
        if (menu.exec(calendarList->viewport()->mapToGlobal(position)) == remove) {
            const int index = item->data(Qt::UserRole).toInt();
            currentUser.productivity.calendar.removeEvent(index);
            saveCalendarAndSchedule();
            loadCalendarPage();
            loadCurrentUser();
            updateDashboardUser();
        } }, Qt::UniqueConnection);
}

void MainWindow::loadTasksPage()
{
    if (!tasksPageList)
        return;
    tasksPageList->clear();
    for (int index = 0; index < currentUser.productivity.todoList.tasks().size(); ++index)
    {
        const Task &task = currentUser.productivity.todoList.tasks().at(index);
        auto *item = new QListWidgetItem(task.description, tasksPageList);
        item->setData(Qt::UserRole, index);
        item->setCheckState(task.done ? Qt::Checked : Qt::Unchecked);
    }
    if (tasksPageList->count() == 0)
        tasksPageList->addItem("No tasks");
    connect(tasksPageList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item)
            {
        const int index = item->data(Qt::UserRole).toInt();
        if (index < 0 || index >= currentUser.productivity.todoList.tasks().size()) return;
        onTaskDoubleClicked(ui->tasksListWidget->item(index));
        loadTasksPage(); }, Qt::UniqueConnection);
}

void MainWindow::loadFullSchedulePage()
{
    if (!fullScheduleList)
        return;
    fullScheduleList->clear();
    for (auto it = currentUser.productivity.hourly.allTasks().cbegin();
         it != currentUser.productivity.hourly.allTasks().cend(); ++it)
    {
        auto *item = new QListWidgetItem(QString::number(it.key()).rightJustified(2, '0') +
                                             ":00 - " + it.value().description,
                                         fullScheduleList);
        item->setData(Qt::UserRole, it.key());
        if (it.value().done)
            item->setText("[done] " + item->text());
    }
    if (fullScheduleList->count() == 0)
        fullScheduleList->addItem("No schedule items");
    connect(fullScheduleList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onScheduleItemDoubleClicked, Qt::UniqueConnection);
}

void MainWindow::loadRemindersPage()
{
    if (!remindersList)
        return;
    remindersList->clear();
    for (int index = 0; index < currentUser.productivity.reminders.all().size(); ++index)
    {
        const Reminder &reminder = currentUser.productivity.reminders.all().at(index);
        auto *item = new QListWidgetItem(reminder.due.toString("yyyy-MM-dd hh:mm") +
                                             " - " + reminder.text,
                                         remindersList);
        item->setData(Qt::UserRole, index);
        item->setCheckState(reminder.completed ? Qt::Checked : Qt::Unchecked);
    }
    if (remindersList->count() == 0)
        remindersList->addItem("No reminders");
    remindersList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(remindersList, &QListWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        QListWidgetItem* item = remindersList->itemAt(position);
        if (!item) return;
        QMenu menu;
        QAction* snooze = menu.addAction("Snooze 15 minutes");
        QAction* remove = menu.addAction("Remove");
        QAction* selected = menu.exec(remindersList->viewport()->mapToGlobal(position));
        const int index = item->data(Qt::UserRole).toInt();
        if (selected == snooze) currentUser.productivity.reminders.snooze(index, 15);
        if (selected == remove) currentUser.productivity.reminders.remove(index);
        if (selected) {
            database.saveReminders(currentUser);
            loadRemindersPage();
        }
    }, Qt::UniqueConnection);
    connect(remindersList, &QListWidget::itemChanged, this, [this](QListWidgetItem *item)
            {
        const int index = item->data(Qt::UserRole).toInt();
        if (index >= 0 && index < currentUser.productivity.reminders.all().size()) {
            currentUser.productivity.reminders.setCompleted(index, item->checkState() == Qt::Checked);
            database.saveReminders(currentUser);
            loadRemindersPage();
        } }, Qt::UniqueConnection);
}

void MainWindow::importCalendar()
{
    const QString path = QFileDialog::getOpenFileName(this, "Import calendar", QString(), "Calendar files (*.ics)");
    if (path.isEmpty())
        return;
    if (!currentUser.productivity.calendar.importFromFile(path))
    {
        QMessageBox::warning(this, "Import calendar", "No valid calendar events were found.");
        return;
    }
    if (saveCalendarAndSchedule())
    {
        loadCalendarDisplay();
        loadCalendarPage();
        QMessageBox::information(this, "Import calendar", "Calendar events imported.");
    }
}

void MainWindow::addReminder()
{
    bool accepted = false;
    const QString text = QInputDialog::getText(this, "Add reminder", "Reminder text:",
                                               QLineEdit::Normal, QString(), &accepted)
                             .trimmed();
    if (!accepted || text.isEmpty())
        return;
    const QDateTime due = QDateTime::currentDateTime().addSecs(
        QInputDialog::getInt(this, "Reminder time", "Minutes from now:", 60, 1, 10080, 1, &accepted) * 60);
    if (!accepted)
        return;
    const int recurrenceDays = QInputDialog::getInt(this, "Repeat reminder",
                                                     "Repeat every N days (0 = once):",
                                                     0, 0, 365, 1, &accepted);
    if (!accepted) return;
    currentUser.productivity.reminders.add({text, due, false, recurrenceDays, {}});
    database.saveReminders(currentUser);
    loadRemindersPage();
    updateDashboardUser();
}

void MainWindow::saveSettings()
{
    const QString home = settingsHomeEdit->text().trimmed();
    const QString work = settingsWorkEdit->text().trimmed();
    if (home.isEmpty())
    {
        QMessageBox::warning(this, "Settings", "Home city is required.");
        return;
    }
    currentUser.email = settingsEmailEdit->text().trimmed();
    currentUser.homeCity = home;
    currentUser.work = work;
    if (!database.updateUserProfile(currentUser))
    {
        QMessageBox::warning(this, "Settings", "Settings could not be saved.");
        return;
    }
    requestWeather(currentUser.homeCity, true);
    requestWeather(currentUser.work, false);
    updateDashboardUser();
    QMessageBox::information(this, "Settings", "Settings saved.");
}

void MainWindow::exportData()
{
    if (currentUser.username.isEmpty())
        return;
    const QString path = QFileDialog::getSaveFileName(this, "Export data", "dailyplanner.json", "JSON files (*.json)");
    if (path.isEmpty())
        return;
    QJsonObject profile;
    profile["username"] = currentUser.username;
    profile["email"] = currentUser.email;
    profile["homeCity"] = currentUser.homeCity;
    profile["workCity"] = currentUser.work;
    QJsonObject data;
    data["profile"] = profile;
    data["tasks"] = QJsonDocument::fromJson(currentUser.productivity.todoList.toJson().toUtf8()).array();
    data["calendar"] = QJsonDocument::fromJson(currentUser.productivity.calendar.toJson()).array();
    data["schedule"] = QJsonDocument::fromJson(currentUser.productivity.hourly.toJson().toUtf8()).object();
    data["reminders"] = QJsonDocument::fromJson(currentUser.productivity.reminders.toJson().toUtf8()).array();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) ||
        file.write(QJsonDocument(data).toJson(QJsonDocument::Indented)) < 0)
    {
        QMessageBox::warning(this, "Export data", "The data file could not be written.");
        return;
    }
    file.close();
    QMessageBox::information(this, "Export data", "Your data was exported.");
}

void MainWindow::importData()
{
    const QString path = QFileDialog::getOpenFileName(this, "Import data", QString(), "JSON files (*.json)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import data", "The data file could not be opened.");
        return;
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        QMessageBox::warning(this, "Import data", "The file is not a valid planner export.");
        return;
    }
    const QJsonObject data = document.object();
    if (!data.contains("tasks") || !data.contains("calendar") || !data.contains("schedule") ||
        !data.contains("reminders")) {
        QMessageBox::warning(this, "Import data", "The export is missing planner data.");
        return;
    }
    if (QMessageBox::question(this, "Import data", "Replace your current planner data?") != QMessageBox::Yes) return;
    currentUser.productivity.todoList = TodoList::fromJson(QJsonDocument(data["tasks"].toArray()).toJson(QJsonDocument::Compact));
    currentUser.productivity.calendar = Calendar::fromJson(QJsonDocument(data["calendar"].toArray()).toJson(QJsonDocument::Compact));
    currentUser.productivity.hourly = HourlySchedules::fromJson(
        QString::fromUtf8(QJsonDocument(data["schedule"].toObject()).toJson(QJsonDocument::Compact)), QDate::currentDate());
    currentUser.productivity.reminders = Reminders::fromJson(
        QString::fromUtf8(QJsonDocument(data["reminders"].toArray()).toJson(QJsonDocument::Compact)));
    if (!saveCurrentTasks() || !saveCalendarAndSchedule() || !database.saveReminders(currentUser)) {
        QMessageBox::warning(this, "Import data", "Some planner data could not be saved.");
        return;
    }
    loadCurrentUser();
    updateDashboardUser();
    loadRemindersPage();
    QMessageBox::information(this, "Import data", "Planner data imported.");
}

void MainWindow::detectLocation()
{
    QNetworkReply *reply = weatherManager->get(QNetworkRequest(QUrl("https://ipapi.co/json/")));
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Location", "Current location could not be detected.");
        } else {
            const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
            const QString city = object.value("city").toString().trimmed();
            if (city.isEmpty()) {
                QMessageBox::warning(this, "Location", "Current city was not returned.");
            } else {
                const QMessageBox::StandardButton choice = QMessageBox::question(
                    this, "Location detected", "Use " + city + " as your home city?",
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                if (choice == QMessageBox::Yes) settingsHomeEdit->setText(city);
                else if (choice == QMessageBox::No) settingsWorkEdit->setText(city);
            }
        }
        reply->deleteLater(); });
}

void MainWindow::buildDailyAnalysis()
{
    const auto &tasks = currentUser.productivity.todoList.tasks();
    const int completed = currentUser.productivity.todoList.completedTasks().size();
    const int events = currentUser.productivity.calendar.eventsOnDate(QDate::currentDate()).size();
    const int reminders = currentUser.productivity.reminders.upcoming().size();
    int scheduled = currentUser.productivity.hourly.allTasks().size();
    QString result = QString("Today\n\nTasks: %1 total, %2 completed\nEvents: %3\nScheduled hours: %4\nUpcoming reminders: %5")
                         .arg(tasks.size())
                         .arg(completed)
                         .arg(events)
                         .arg(scheduled)
                         .arg(reminders);
    int overdue = 0;
    for (const Task &task : tasks)
    {
        if (!task.done && task.dueDate.isValid() && task.dueDate < QDateTime::currentDateTime())
            ++overdue;
    }
    result += "\nOverdue tasks: " + QString::number(overdue);
    if (overdue > 0)
        result += "\nRecommendation: clear an overdue task before adding new work.";
    else if (completed < tasks.size() && events > 0)
        result += "\nRecommendation: reserve time around today's events for unfinished tasks.";
    else
        result += "\nRecommendation: keep the current plan and review it this evening.";
    analysisLabel->setText(result);
}

QString MainWindow::weatherRecommendation(const WeatherData &data) const
{
    if (!data.success || data.forecast.isEmpty())
        return "No weather recommendation available.";
    const ForecastEntry &current = data.forecast.first();
    bool rainExpected = false;
    bool strongWindExpected = false;
    double highestTemperature = current.temp;
    for (int index = 0; index < qMin(8, data.forecast.size()); ++index) {
        const ForecastEntry &entry = data.forecast.at(index);
        rainExpected = rainExpected || entry.pop >= 0.5 || entry.rainVolume > 0.0;
        strongWindExpected = strongWindExpected || entry.windSpeed >= 10.0;
        highestTemperature = qMax(highestTemperature, entry.temp);
    }
    if (rainExpected)
        return "Take an umbrella and allow extra travel time.";
    if (highestTemperature >= 28.0)
        return "Stay hydrated and avoid the hottest part of the day.";
    if (current.temp <= 5.0)
        return "Dress warmly before heading outside.";
    if (strongWindExpected)
        return "Expect strong wind; secure loose items and allow extra travel time.";
    return "Conditions look suitable for your planned activities.";
}

void MainWindow::checkReminders()
{
    if (currentUser.username.isEmpty())
        return;
    const QDateTime now = QDateTime::currentDateTime();
    for (int index = 0; index < currentUser.productivity.reminders.all().size(); ++index)
    {
        const Reminder &reminder = currentUser.productivity.reminders.all().at(index);
        if (!reminder.completed && reminder.due.isValid() && reminder.due <= now &&
            (!reminder.snoozedUntil.isValid() || reminder.snoozedUntil <= now))
        {
            QMessageBox::information(this, "Reminder", reminder.text);
            const Reminder& current = currentUser.productivity.reminders.all().at(index);
            if (current.recurrenceDays == 0) {
                currentUser.productivity.reminders.setCompleted(index, true);
            } else {
                Reminder next = current;
                do {
                    next.due = next.due.addDays(next.recurrenceDays);
                } while (next.due <= now);
                currentUser.productivity.reminders.remove(index);
                currentUser.productivity.reminders.add(next);
                --index;
            }
            database.saveReminders(currentUser);
        }
    }
    loadRemindersPage();
    updateDashboardUser();
}

void MainWindow::requestWeather(const QString &city, bool home)
{
    const QString apiKey = qEnvironmentVariable("OPENWEATHER_API_KEY").trimmed();
    QLabel *tempLabel = home ? ui->weatherHomeTempLabel : ui->weatherWorkTempLabel;
    QLabel *descLabel = home ? ui->weatherHomeDescLabel : ui->weatherWorkDescLabel;
    QLabel *detailLabel = home ? ui->weatherHomeDetailLabel : ui->weatherWorkDetailLabel;
    if (apiKey.isEmpty() || city.isEmpty())
    {
        tempLabel->setText("Unavailable");
        descLabel->setText("Set OPENWEATHER_API_KEY to load weather");
        detailLabel->clear();
        return;
    }
    tempLabel->setText("Loading...");
    const QUrl url(QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2")
                       .arg(QString::fromUtf8(QUrl::toPercentEncoding(city)), apiKey));
    QNetworkReply *reply = weatherManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, home]()
            {
        QLabel* tempLabel = home ? ui->weatherHomeTempLabel : ui->weatherWorkTempLabel;
        QLabel* descLabel = home ? ui->weatherHomeDescLabel : ui->weatherWorkDescLabel;
        QLabel* detailLabel = home ? ui->weatherHomeDetailLabel : ui->weatherWorkDetailLabel;
        if (reply->error() != QNetworkReply::NoError) {
            tempLabel->setText("Unavailable");
            descLabel->setText(reply->errorString());
            detailLabel->clear();
        } else {
            const WeatherData data = parseWeatherData(reply->readAll());
            if (data.success && !data.forecast.isEmpty()) {
                const ForecastEntry& current = data.forecast.first();
                tempLabel->setText(QString::number(current.temp, 'f', 1) + " C");
                descLabel->setText(current.description + ", feels like " +
                                   QString::number(current.feelsLike, 'f', 1) + " C");
                detailLabel->setText(QString::number(current.humidity) + "% humidity | " +
                                     QString::number(current.windSpeed * 3.6, 'f', 1) + " km/h wind | " +
                                     QString::number(current.pop * 100, 'f', 0) + "% rain chance | " +
                                     weatherRecommendation(data));
            } else {
                tempLabel->setText("Unavailable");
                descLabel->setText(data.errorMessage.isEmpty() ? "Weather data unavailable" : data.errorMessage);
                detailLabel->clear();
            }
        }
        reply->deleteLater(); });
}

void MainWindow::onScheduleItemDoubleClicked(QListWidgetItem *item)
{
    const int hour = item->data(Qt::UserRole).toInt();
    if (hour < 0 || hour > 23 || !currentUser.productivity.hourly.hasTask(hour))
        return;
    bool accepted = false;
    const QString text = QInputDialog::getText(this, "Edit schedule item", "Description:",
                                               QLineEdit::Normal,
                                               currentUser.productivity.hourly.taskAt(hour).description,
                                               &accepted)
                             .trimmed();
    if (accepted && !text.isEmpty())
    {
        currentUser.productivity.hourly.setTask(hour, text);
        saveCalendarAndSchedule();
        loadFullSchedulePage();
        loadScheduleDisplay();
    }
}

void MainWindow::onCalendarItemDoubleClicked(QListWidgetItem *item)
{
    const int index = item->data(Qt::UserRole).toInt();
    if (index < 0 || index >= currentUser.productivity.calendar.events().size())
        return;
    CalendarEvent event = currentUser.productivity.calendar.events().at(index);
    QDialog dialog(this);
    dialog.setWindowTitle("Edit event");
    QVBoxLayout layout(&dialog);
    QLineEdit summaryEdit(event.summary);
    QDateTimeEdit startEdit(event.start);
    QDateTimeEdit endEdit(event.end);
    QCheckBox allDayEdit("All day", &dialog);
    allDayEdit.setChecked(event.allDay);
    layout.addWidget(new QLabel("Summary:"));
    layout.addWidget(&summaryEdit);
    layout.addWidget(new QLabel("Start:"));
    startEdit.setCalendarPopup(true);
    layout.addWidget(&startEdit);
    layout.addWidget(new QLabel("End:"));
    endEdit.setCalendarPopup(true);
    layout.addWidget(&endEdit);
    layout.addWidget(&allDayEdit);
    QPushButton save("Save", &dialog);
    QPushButton cancel("Cancel", &dialog);
    layout.addWidget(&save);
    layout.addWidget(&cancel);
    connect(&save, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || summaryEdit.text().trimmed().isEmpty())
        return;
    currentUser.productivity.calendar.removeEvent(index);
    event.summary = summaryEdit.text().trimmed();
    event.start = startEdit.dateTime();
    event.end = endEdit.dateTime();
    event.allDay = allDayEdit.isChecked();
    currentUser.productivity.calendar.addEvent(event);
    saveCalendarAndSchedule();
    loadCalendarPage();
    loadCalendarDisplay();
}
