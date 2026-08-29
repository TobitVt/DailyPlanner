#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_welcome.h"
#include "ui_login.h"
#include "ui_signup.h"

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

MainWindow::MainWindow(Database& database, QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), welcomeUi(nullptr), loginUi(nullptr),
            signupUi(nullptr), screenStack(new QStackedWidget(this)), database(database) {
    ui->setupUi(this);

        QWidget* dashboard = ui->centralwidget;
        screenStack->addWidget(dashboard);

        QWidget* welcomePage = new QWidget(screenStack);
        welcomeUi = new Ui::WelcomeScreen;
        welcomeUi->setupUi(welcomePage);
        screenStack->addWidget(welcomePage);

        QWidget* loginPage = new QWidget(screenStack);
        loginUi = new Ui::LoginScreen;
        loginUi->setupUi(loginPage);
        screenStack->addWidget(loginPage);

        QWidget* signupPage = new QWidget(screenStack);
        signupUi = new Ui::SignUpScreen;
        signupUi->setupUi(signupPage);
        screenStack->addWidget(signupPage);
        setCentralWidget(screenStack);

    connect(ui->addTaskButton, &QPushButton::clicked, this, &MainWindow::onAddTaskClicked);
    connect(ui->tasksListWidget, &QListWidget::itemChanged, this, &MainWindow::onTaskItemChanged);
    connect(ui->tasksListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onTaskListContextMenuRequested);
    connect(ui->tasksListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onTaskDoubleClicked);
    ui->tasksListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(ui->scheduleListWidget, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QListWidgetItem* item = ui->scheduleListWidget->itemAt(pos);
        if (!item) return;
        QMenu menu;
        QAction* removeAction = menu.addAction("Remove");
        if (menu.exec(ui->scheduleListWidget->viewport()->mapToGlobal(pos)) == removeAction) {
            int row = ui->scheduleListWidget->row(item);
            int hour = row + 6;
            currentUser.productivity.hourly.removeTask(hour);
            saveCalendarAndSchedule();
            loadScheduleDisplay();
        }
    });
    ui->scheduleListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    
    // Connect schedule and event buttons
    if (ui->viewAllTasksButton) {
        connect(ui->viewAllTasksButton, &QPushButton::clicked, this, [this]() {
            onAddScheduleItemClicked();
        });
    }
    if (ui->viewCalendarButton) {
        connect(ui->viewCalendarButton, &QPushButton::clicked, this, [this]() {
            onAddEventClicked();
        });
    }

    connect(welcomeUi->loginButton, &QPushButton::clicked, this, &MainWindow::showLoginScreen);
    connect(welcomeUi->signupButton, &QPushButton::clicked, this, &MainWindow::showSignupScreen);
    connect(loginUi->submitButton, &QPushButton::clicked, this, &MainWindow::onLoginSubmitted);
    connect(loginUi->backButton, &QPushButton::clicked, this, &MainWindow::showWelcomeScreen);
    connect(signupUi->submitButton, &QPushButton::clicked, this, &MainWindow::onSignupSubmitted);
    connect(signupUi->backButton, &QPushButton::clicked, this, &MainWindow::showWelcomeScreen);

    showWelcomeScreen();
}

MainWindow::~MainWindow() {
    delete welcomeUi;
    delete loginUi;
    delete signupUi;
    delete ui;
}

void MainWindow::showLoginScreen() {
    screenStack->setCurrentIndex(2);
    loginUi->usernameEdit->setFocus();
}

void MainWindow::showSignupScreen() {
    screenStack->setCurrentIndex(3);
    signupUi->usernameEdit->setFocus();
}

void MainWindow::showWelcomeScreen() {
    screenStack->setCurrentIndex(1);
}

void MainWindow::onLoginSubmitted() {
    const QString username = loginUi->usernameEdit->text().trimmed();
    const QString password = loginUi->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login", "Enter both a username and password.");
        return;
    }

    userInfo user = database.getAllInfo(username);
    if (user.username.isEmpty() || !PasswordHash::verify(password, user.password)) {
        QMessageBox::warning(this, "Login", "Incorrect username or password.");
        return;
    }

    currentUser = user;
    loadCurrentUser();
    updateDashboardUser();
    screenStack->setCurrentIndex(0);
    loginUi->passwordEdit->clear();
}

void MainWindow::onSignupSubmitted() {
    userInfo newUser;
    newUser.username = signupUi->usernameEdit->text().trimmed();
    newUser.password = signupUi->passwordEdit->text();
    newUser.email = signupUi->emailEdit->text().trimmed();
    newUser.homeCity = signupUi->homeCityEdit->text().trimmed();
    newUser.work = signupUi->workCityEdit->text().trimmed();

    if (newUser.username.isEmpty() || newUser.password.isEmpty() ||
        newUser.homeCity.isEmpty() || newUser.work.isEmpty()) {
        QMessageBox::warning(this, "Create account",
                             "Username, password, home city, and work city are required.");
        return;
    }

    if (!database.createUser(newUser)) {
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

void MainWindow::onLogoutClicked() {
    currentUser = userInfo();
    ui->tasksListWidget->clear();
    ui->scheduleListWidget->clear();
    ui->userNameLabel->setText("Guest");
    ui->userEmailLabel->setText("Not signed in");
    ui->greetingLabel->setText("Welcome!");
    ui->summaryTasksLabel->setText("No tasks loaded");
    ui->summaryEventsLabel->setText("No events loaded");
    showWelcomeScreen();
}

void MainWindow::loadCurrentUser() {
    QSignalBlocker blocker(ui->tasksListWidget);
    ui->tasksListWidget->clear();
    for (const Task& task : currentUser.productivity.todoList.tasks()) {
        addTaskToList(task.description, task.done);
    }
    loadScheduleDisplay();
    loadCalendarDisplay();
}

void MainWindow::loadScheduleDisplay() {
    ui->scheduleListWidget->clear();
    const HourlySchedules& schedule = currentUser.productivity.hourly;
    for (int hour = 6; hour < 23; ++hour) {
        if (schedule.hasTask(hour)) {
            const HourlyTask& task = schedule.taskAt(hour);
            QString text = QString::number(hour).rightJustified(2, '0') + ":00 - " + task.description;
            if (task.done) {
                text = "✓ " + text;
            }
            ui->scheduleListWidget->addItem(text);
        }
    }
    if (ui->scheduleListWidget->count() == 0) {
        ui->scheduleListWidget->addItem("No schedule items for today");
    }
}

void MainWindow::loadCalendarDisplay() {
    const QDate today = QDate::currentDate();
    const QVector<CalendarEvent> todayEvents = currentUser.productivity.calendar.eventsOnDate(today);
    int eventCount = todayEvents.size();
    ui->summaryEventsLabel->setText(QString::number(eventCount) + " event" + (eventCount != 1 ? "s" : "") + " today");
}

bool MainWindow::saveCurrentTasks() {
    if (currentUser.username.isEmpty()) {
        return false;
    }
    if (!database.saveTodoList(QStringList(), currentUser)) {
        QMessageBox::warning(this, "Save error", "Your task changes could not be saved.");
        return false;
    }
    return true;
}

bool MainWindow::saveCalendarAndSchedule() {
    if (currentUser.username.isEmpty()) {
        return false;
    }
    if (!database.saveHourlySchedules(QMap<int, QString>(), currentUser)) {
        QMessageBox::warning(this, "Save error", "Your schedule changes could not be saved.");
        return false;
    }
    if (!database.saveCalendar(currentUser.productivity.calendar.toJson(), currentUser)) {
        QMessageBox::warning(this, "Save error", "Your calendar changes could not be saved.");
        return false;
    }
    return true;
}

void MainWindow::updateDashboardUser() {
    ui->userNameLabel->setText(currentUser.username);
    ui->userEmailLabel->setText("Home: " + currentUser.homeCity + " | Work: " + currentUser.work);
    updateGreeting();
    ui->summaryTasksLabel->setText(QString::number(currentUser.productivity.todoList.count()) +
                                   " tasks");
    ui->summaryEventsLabel->setText(QString::number(currentUser.productivity.calendar.events().size()) +
                                    " events");
    if (ui->summaryReminderLabel) {
        ui->summaryReminderLabel->setText("No reminders loaded");
    }
}

void MainWindow::updateGreeting() {
    const int hour = QTime::currentTime().hour();
    QString greeting = hour < 12 ? "Good morning" : hour < 18 ? "Good afternoon" : "Good evening";
    ui->greetingLabel->setText(greeting + ", " + currentUser.username + "!");
    ui->dateLabel->setText(QDate::currentDate().toString("dddd, d MMMM yyyy"));
}

void MainWindow::addTaskToList(const QString& text, bool done) {
    auto* item = new QListWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(done ? Qt::Checked : Qt::Unchecked);
    ui->tasksListWidget->addItem(item);
}

void MainWindow::onAddTaskClicked() {
    bool accepted = false;
    const QString text = QInputDialog::getText(
        this, "Add task", "Task description:", QLineEdit::Normal, QString(), &accepted).trimmed();
    if (!accepted || text.isEmpty()) {
        return;
    }

    currentUser.productivity.todoList.addTask(text);
    addTaskToList(text);
    saveCurrentTasks();
}

void MainWindow::onTaskListContextMenuRequested(const QPoint& position) {
    QListWidgetItem* selected = ui->tasksListWidget->itemAt(position);
    if (!selected) {
        return;
    }

    QMenu menu(this);
    QAction* removeAction = menu.addAction("Remove task");
    if (menu.exec(ui->tasksListWidget->viewport()->mapToGlobal(position)) == removeAction) {
        if (QMessageBox::question(this, "Remove task", "Remove the selected task?") ==
            QMessageBox::Yes) {
            const int row = ui->tasksListWidget->row(selected);
            currentUser.productivity.todoList.removeTask(row);
            delete ui->tasksListWidget->takeItem(row);
            saveCurrentTasks();
        }
    }
}

void MainWindow::onTaskItemChanged(QListWidgetItem* item) {
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

void MainWindow::onTaskDoubleClicked(QListWidgetItem* item) {
    const int row = ui->tasksListWidget->row(item);
    if (row < 0 || row >= currentUser.productivity.todoList.tasks().size()) {
        return;
    }

    const Task& task = currentUser.productivity.todoList.tasks()[row];

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

    if (dialog.exec() == QDialog::Accepted) {
        currentUser.productivity.todoList.setPriority(row, static_cast<TaskPriority>(priorityCombo.currentData().toInt()));
        currentUser.productivity.todoList.setDueDate(row, dueDateEdit.dateTime());
        saveCurrentTasks();
        item->setText(descEdit.text());
    }
}

void MainWindow::onAddScheduleItemClicked() {
    bool accepted = false;
    const int hour = QInputDialog::getInt(
        this, "Add schedule item", "Hour (6-22):", 9, 6, 22, 1, &accepted);
    if (!accepted) return;

    const QString description = QInputDialog::getText(
        this, "Add schedule item", "Task description:", QLineEdit::Normal, QString(), &accepted).trimmed();
    if (!accepted || description.isEmpty()) return;

    currentUser.productivity.hourly.setTask(hour, description);
    saveCalendarAndSchedule();
    loadScheduleDisplay();
}

void MainWindow::onAddEventClicked() {
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

    if (dialog.exec() == QDialog::Accepted) {
        if (summaryEdit.text().trimmed().isEmpty()) {
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
        QMessageBox::information(this, "Event added", "Calendar event has been added.");
    }
}
