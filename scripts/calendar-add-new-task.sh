#/bin/sh
#Name=Calendar: Add new task
qdbus com.nokia.Calendar / com.nokia.maemo.meegotouch.CalendarInterface.showDefaultView
qdbus com.nokia.Calendar / com.nokia.maemo.meegotouch.CalendarInterface.newTodo "" ""
