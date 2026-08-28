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
    ui->tasksListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

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
    if (user.username.isEmpty() || user.password != password) {
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

void MainWindow::updateDashboardUser() {
    ui->userNameLabel->setText(currentUser.username);
    ui->userEmailLabel->setText("Home: " + currentUser.homeCity + " | Work: " + currentUser.work);
    updateGreeting();
    ui->summaryTasksLabel->setText(QString::number(currentUser.productivity.todoList.count()) +
                                   " tasks");
    ui->summaryEventsLabel->setText(QString::number(currentUser.productivity.calendar.events().size()) +
                                    " events");
    ui->summaryReminderLabel->setText("No reminders loaded");
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