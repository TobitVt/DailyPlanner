#include "todoList.h"

todoList::todoList(std::string LN)
{
    listName = LN;
};

void todoList::createTodoList()
{
    std::ofstream myList(listName + ".txt");
    std::string todoItem;
    std::string temp;
    int count = 1;

    //create todo list
    std::cout << std::endl << "add things to your todo list(type done when finished): ";
    std::cout << std::endl << "item 1: ";
    while (std::getline(std::cin, todoItem))
    {
        if(todoItem =="done" || todoItem =="Done")
            break;  

        temp = std::to_string(count) + " " + todoItem;
        myList << temp << std::endl;
        list_vect.push_back(temp);
        count ++;
        std::cout <<  "item " << count << ": ";
        
    }
    myList.close();
}

std::vector<std::string>& todoList::getTodoList()
{
    return list_vect;
}

void todoList::updateList()
{
    std::ofstream myList(listName + ".txt");

    for (size_t i = 0; i < list_vect.size(); i++)
    {
        size_t spacePos = list_vect[i].find(' ');
        std::string task = list_vect[i].substr(spacePos + 1);

        list_vect[i] = std::to_string(i + 1) + " " + task;

        myList << list_vect[i] << std::endl;
    }

    myList.close();
}

void todoList::printList() const
{

    for (size_t i = 0; i < list_vect.size(); i++)
    {
        std::cout << std::endl << list_vect[i];
    }
    std::cout << std::endl;

}

bool todoList::isEmpty()
{
    if (list_vect.size() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


todoList::~todoList(){}