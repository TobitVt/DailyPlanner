// To-Do List Application: A command-line list where users can append tasks, 
// view current tasks, and mark them as done. Teaches dynamic arrays (std::vector) 
// and data persistence using file streams (std::ofstream/std::ifstream)


#include <iostream>
#include <vector>

#include "todoList.h"

int main() {

    todoList myTodoList("todoList");

    myTodoList.createTodoList();
 
    while (true)
    {
        std::vector<std::string>& todo_vect = myTodoList.getTodoList();

        std::cout << std::endl << "please select the tasks you have completed(1 - " << todo_vect.size() <<"): ";
        myTodoList.printList();

        int done;
        std::cin >> done;

        bool found = false;

        for (size_t i = 0; i < todo_vect.size(); i++)
        {
            int taskNumber = stoi(todo_vect[i]);

            if (done == taskNumber)
            {
                todo_vect.erase(todo_vect.begin() + i);
                myTodoList.updateList();
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << "\nInvalid task number.\n";
        }


        if (myTodoList.isEmpty()) {break;}

    }

    return 0;
}