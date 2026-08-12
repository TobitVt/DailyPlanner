#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidgetItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddTaskClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveTaskClicked);
    connect(ui->taskList, &QListWidget::itemChanged, this, &MainWindow::onTaskItemChanged);

    // Pressing Enter in the input field also adds the task.
    connect(ui->taskInput, &QLineEdit::returnPressed, this, &MainWindow::onAddTaskClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::addTaskToList(const QString& text, bool done) {
    auto* item = new QListWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(done ? Qt::Checked : Qt::Unchecked);
    ui->taskList->addItem(item);
}

void MainWindow::onAddTaskClicked() {
    QString text = ui->taskInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    addTaskToList(text);
    ui->taskInput->clear();

    // TODO: persist the new task (e.g. via a SQLite-backed task store)
}

void MainWindow::onRemoveTaskClicked() {
    QListWidgetItem* selected = ui->taskList->currentItem();
    if (!selected) {
        QMessageBox::information(this, "No selection", "Select a task to remove first.");
        return;
    }

    delete ui->taskList->takeItem(ui->taskList->row(selected));

    // TODO: persist the removal
}

void MainWindow::onTaskItemChanged(QListWidgetItem* item) {
    bool isDone = (item->checkState() == Qt::Checked);

    // Visually distinguish completed tasks.
    QFont font = item->font();
    font.setStrikeOut(isDone);
    item->setFont(font);

    // TODO: persist the completion state change
}