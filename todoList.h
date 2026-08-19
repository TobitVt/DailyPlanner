#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "database.h"

class todoList {
private:
    std::string listName;
    std::vector<std::string> list_vect;

public:
    todoList(std::string LN);

    void createTodoList();
    std::vector<std::string>& getTodoList();

    void updateList();

    void addTask(QString task);
    void removeTask(int index);
    void completeTask(int index);

    void printList() const;
    bool isEmpty();

    void save(Database& db);

    ~todoList();

};
