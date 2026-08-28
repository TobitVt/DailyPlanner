# DailyPlanner

A personalized desktop daily planner built with C++ and Qt. The application combines task management, calendar scheduling, reminders, user accounts, and weather integration to provide a centralized dashboard for planning and organizing daily activities.

## Features

The project is currently an early working prototype. The CLI supports basic
SQLite-backed registration and login. The GUI opens a dashboard and supports
basic in-memory task interactions; the GUI authentication and persistence flow
is still being implemented.

### User Accounts
- User registration and login
- Basic password storage (hashing is not implemented yet)
- Personalized user profiles
- Individual database records per user

### Task Management
- Add and remove tasks in the GUI
- Mark tasks as completed in the GUI
- Priority levels and due dates are supported by the model
- GUI persistence is not implemented yet

### Calendar & Scheduling
- Interactive calendar view
- Create and manage events
- Daily and weekly schedules
- Event reminders
- Import existing calendars

### Calendar Import
- Import events from `.ics` calendar files
- Automatic event synchronization
- Duplicate event detection
- Bulk event importing

### Weather Integration
- Location-based weather forecasts
- Home and work locations
- Weather recommendations
- Daily weather summaries

### Dashboard
- Personalized greeting
- Today's tasks
- Upcoming events
- Reminder notifications
- Current weather information
- Daily overview

## Technologies

- C++
- Qt 6
- SQLite
- CMake
- REST APIs
- JSON


## Planned Features

- [ ] GUI user authentication and current-user handling
- [x] Basic SQLite database integration
- [x] Dashboard UI prototype
- [ ] GUI task persistence and task editing
- [ ] GUI calendar and schedule integration
- [ ] Reminder notifications
- [x] Basic weather API integration in the CLI
- [x] Basic `.ics` calendar import model
- [ ] Settings page
- [ ] Dark mode
- [ ] Data export

## Current Progress

The project is approximately **30% complete** against the full planned
application. The strongest areas are the Qt/CMake setup, core data models,
basic SQLite user/productivity storage, CLI authentication, calendar parsing,
and the initial GUI task interactions.

The largest remaining work is connecting the GUI login and signup screens to
the database, tracking the current user, loading and saving all productivity
data, and implementing the schedule, calendar, reminders, dashboard weather,
and daily-analysis workflows.

## Building

### Requirements

- Qt 6
- CMake 3.16+
- MinGW or MSVC
- SQLite

### Build Steps

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

## Screenshots

Screenshots will be added as development progresses.

## License

This project is intended for educational and portfolio purposes.