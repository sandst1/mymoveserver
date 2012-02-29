#/bin/sh
#Name=Calendar: Add new event
qdbus com.nokia.Calendar / com.nokia.maemo.meegotouch.CalendarInterface.showDefaultView
qdbus com.nokia.Calendar / com.nokia.maemo.meegotouch.CalendarInterface.newEvent "" ""
