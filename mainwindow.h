#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidgetItem;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTaskClicked();
    void onRemoveTaskClicked();
    void onTaskItemChanged(QListWidgetItem* item);

private:
    Ui::MainWindow *ui;
    void addTaskToList(const QString& text, bool done = false);
};