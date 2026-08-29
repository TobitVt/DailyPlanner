
#include <iostream>
#include <QVector>
#include <QDebug>
#include <cstdlib>
#include <QCoreApplication>
#include <QTime>
#include <QDate>
#include <cmath>
#include <QLocale>

#include "todoList.h"
#include "weather.h"
#include "database.h"
#include "verse.h"


//set up log in/ sign up
//set up user auth
//set up GUI

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    Database plannerDB("planner.db"); 

    QTextStream in(stdin);
    QTextStream out(stdout);

    QString home, work;


    bool authenticated = false;

    while (!authenticated)
    {
        out << getDailyVerse() << Qt::flush;
        out << "\n--- WEATHER APP ---\n1. Sign Up\n2. Log In\n3. Exit\nChoose: " << Qt::flush;
        QString choice = in.readLine().trimmed();

        if (choice == "1")
        {
            userInfo newUser;

            out << "Create Username: " << Qt::flush;   newUser.username = in.readLine().trimmed();
            out << "Create Password: " << Qt::flush;   newUser.password = in.readLine().trimmed();
            out << "Home City location: " << Qt::flush;  newUser.homeCity = in.readLine().trimmed();
            out << "Work City location: " << Qt::flush;  newUser.work = in.readLine().trimmed();

            home = newUser.homeCity;
            work = newUser.work;

            if (plannerDB.createUser(newUser))
            {
                out << "Account registered!\n";
                authenticated = true;
            }
            else
            {
                out << "Unable to register account. The username may already exist.\n";
            }
        } 
        else if (choice == "2") 
        {
            out << "Enter Username: " << Qt::flush; QString inputUser = in.readLine().trimmed();
            out << "Enter Password: " << Qt::flush; QString inputPass = in.readLine().trimmed();

            userInfo loggedUser = plannerDB.getAllInfo(inputUser);

            if (loggedUser.username.isEmpty())
            {
                out << "No accounts exist yet. Please Sign Up first.\n";
                continue;
            }

            if (PasswordHash::verify(inputPass, loggedUser.password)) {
                out << "\n====================================\n"
                    << "               DASHBOARD              \n"
                    << "====================================\n"
                    << " Monitoring Weather Conditions For:\n"
                    << " Home: " << loggedUser.homeCity << "\n"
                    << " Work: " << loggedUser.work << "\n"
                    << "====================================\n";

                home = loggedUser.homeCity;   
                work = loggedUser.work; 

                authenticated = true;
            } else {
                out << "Access Denied: Incorrect login details.\n";
            }

        } else if (choice == "3") 
        {
            return 0;
        } else 
        {
            out << "Invalid option entry.\n";
        }
    }

    QString hAns, wAns;

    //home:

    out << "Current conditions at home " << home << ": \n" << Qt::flush;
    getWeatherSummary(home);

    out << "get forecast for the next couple days at home?(Y/N): " << Qt::flush;   hAns = in.readLine().trimmed();

    if (hAns == "y" || hAns == "Y")
    {
        getForecast(home);
    }

    //work:

    out << "Current conditions at work " << work << ": \n" << Qt::flush;
    getWeatherSummary(work);

    out << "get forecast for the next couple days at work?(Y/N): " << Qt::flush;   wAns = in.readLine().trimmed();

    if (wAns == "y" || wAns == "Y")
    {
        getForecast(work);
    }


    out << "\nGoodbye!\n" << Qt::flush;



    return 0;
}  

