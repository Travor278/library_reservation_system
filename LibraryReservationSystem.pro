QT += core gui widgets

CONFIG += c++17

TARGET = LibraryReservationSystem
TEMPLATE = app

SOURCES += main.cpp MainWindow.cpp LibraryManager.cpp FloorPlanWidget.cpp SeatViewWidget.cpp LoginDialog.cpp StatisticsWidget.cpp

HEADERS += MainWindow.h LibraryManager.h FloorPlanWidget.h SeatViewWidget.h LoginDialog.h DataModel.h StatisticsWidget.h
