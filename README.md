# DailyPlanner

A personalized desktop daily planner built with C++ and Qt. The application combines task management, calendar scheduling, reminders, user accounts, and weather integration to provide a centralized dashboard for planning and organizing daily activities.

## Features

The project is a working prototype with both CLI and Qt GUI applications.
User data and productivity information are stored in SQLite.

### User Accounts
- User registration and login
- SHA-256 password hashing
- Personalized user profiles
- Email, home city, and work city fields
- Individual database records per user

### Task Management
- Add and remove tasks in the GUI
- Mark tasks as completed in the GUI
- Edit priorities, due dates, and descriptions
- SQLite persistence for task changes

### Calendar & Scheduling
- Calendar event creation, editing, and deletion
- Hourly schedule creation, editing, and deletion
- Separate calendar and schedule pages
- Import events from `.ics` files through the GUI
- Month and week calendar filtering
- Daily and weekly recurring events

### Calendar Import
- Import events from `.ics` calendar files
- Basic `.ics` event parsing
- Duplicate event detection
- Validation of malformed calendar input

### Weather Integration
- Location-based weather forecasts
- Home and work locations
- Weather recommendations
- Weather display for home and work cities
- Automatic current-city detection with user confirmation
- Weather-aware schedule shifting with confirmation

### Dashboard
- Personalized greeting
- Today's tasks
- Upcoming events
- Reminder notifications
- Current weather information
- Daily productivity analysis
- Daily overview

## Technologies

- C++
- Qt 6
- SQLite
- CMake
- REST APIs
- JSON


## Implemented Features

- [x] GUI user authentication and current-user handling
- [x] Basic SQLite database integration
- [x] Dashboard with task, event, schedule, reminder, and weather data
- [x] GUI task persistence and task editing
- [x] GUI calendar and schedule integration
- [x] Reminder notifications
- [x] Basic weather API integration in the CLI
- [x] GUI `.ics` calendar import
- [x] Settings page and profile updates
- [x] JSON data export
- [x] Daily analysis
- [x] Month and week calendar views
- [x] Recurring reminders and custom snooze times
- [x] Duplicate calendar import detection
- [x] JSON data import with transactional saving
- [x] Dark mode
- [x] Weather-aware schedule adjustments

## Remaining Work

- [x] Automated model tests
- [ ] Richer calendar grid styling
- [ ] Recurring calendar exceptions and advanced ICS rules
- [ ] Full GUI integration tests

## Current Progress

The project is approximately **90% complete** against the full planned
application. The strongest areas are authentication, SQLite persistence,
task management, calendar and schedule handling, reminders, weather display,
settings, export, and the main dashboard workflow.

The largest remaining work is richer calendar styling, advanced ICS recurrence
rules, and full GUI integration testing.

## Verification

- CLI build: passed
- GUI build: passed
- Automated model tests: passed
- GUI startup smoke check: passed
- CMake diagnostics: none reported

Full interactive GUI workflows, including clicking through modal dialogs and
network-dependent weather/location actions, still need manual verification.

## Building

### Requirements

- Qt 6
- CMake 3.16+
- MinGW or MSVC
- SQLite
- An OpenWeather API key for dashboard weather

### Build Steps

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

To enable dashboard weather, set the `OPENWEATHER_API_KEY` environment
variable before starting the GUI. Location detection requires an internet
connection and uses an external IP-based location service.

## Screenshots

Screenshots will be added as development progresses.

## License

This project is intended for educational and portfolio purposes.