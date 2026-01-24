#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include <QTimer>
#include <QComboBox>
#include "DataModel.h"
#include "FloorPlanWidget.h"
#include "SeatViewWidget.h"
#include "StatisticsWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCampusSelected(int index);
    void onFloorSelected(int floorLevel);
    void onZoneSelected(Zone* zone, const QString& floorName);
    void onSeatSelected(Seat* seat);
    void onBackToFloorList();
    void onBackToFloorPlan();
    void onDataChanged();
    void onTimerUpdate();
    void showAllAnnouncements();
    void updateTimeStatusDisplay();
    void updateAnnouncements();
    void onSeatSelectedForDate(Seat* seat, bool isTomorrow);

    // 用户操作
    void onSignIn();
    void onSignOut();
    void onTempLeave();
    void onReturnFromLeave();
    void onCancelReservation();
    void onLogout();
    void showFavorites();
    void showHistory();
    void showUserInfo();
    void showWelcomeDialog();
    void showRulesDialog();
    void showStatistics();

private:
    void setupUI();
    void setupCampusPage();
    void setupFloorListPage();
    void setupStatusBar();
    void updateUserStatus();
    void updateFloorList();
    void refreshCurrentView();
    
    // 当前状态
    Campus* currentCampus;
    Floor* currentFloor;
    Zone* currentZone;
    
    // 主布局
    QWidget* centralWidget;
    QStackedWidget* mainStack;
    
    // 页面0: 校区选择/首页
    QLabel* timeStatusLabel;
    QWidget* announcementWidget;
    QLabel* announcementLabel;
    QWidget* campusPage;
    QComboBox* campusCombo;
    QLabel* campusImageLabel;
    QWidget* floorButtonsWidget;
    QList<QPushButton*> floorButtons;
    StatisticsWidget* statisticsWidget;
    
    // 页面1: 楼层平面图
    FloorPlanWidget* floorPlanWidget;
    
    // 页面2: 座位选择
    SeatViewWidget* seatViewWidget;
    
    // 侧边栏/用户信息
    QWidget* sideBar;
    QLabel* userNameLabel;
    QLabel* userIdLabel;
    QLabel* creditLabel;
    QLabel* currentSeatLabel;
    QPushButton* btnSignIn;
    QPushButton* btnSignOut;
    QPushButton* btnTempLeave;
    QPushButton* btnReturn;
    QPushButton* btnCancel;
    QPushButton* btnFavorites;
    QPushButton* btnHistory;
    QPushButton* btnLogout;
    
    // 定时器
    QTimer* refreshTimer;
};

#endif // MAINWINDOW_H
