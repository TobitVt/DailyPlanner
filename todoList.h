#pragma once

#include <iostream>
#include <fstream>
#include <vector>

class todoList {
private:
    std::string listName;
    std::vector<std::string> list_vect;

public:
    todoList(std::string LN);

    void createTodoList();
    std::vector<std::string>& getTodoList();
    void updateList();
    void printList() const;
    bool isEmpty();

    ~todoList();

};
