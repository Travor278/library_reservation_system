#include "MainWindow.h"
#include "LibraryManager.h"
#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QHeaderView>
#include <QApplication>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentCampus(nullptr), currentFloor(nullptr), currentZone(nullptr) {
    
    setupUI();
    setupStatusBar();
    
    // 连接信号
    LibraryManager& mgr = LibraryManager::instance();
    connect(&mgr, &LibraryManager::dataChanged, this, &MainWindow::onDataChanged);

    // 签到成功提示
    connect(&mgr, &LibraryManager::signInSuccess, this, [this](const QString& seatId, int visitorNum) {
        QString msg = QString("✅ 签到成功！\n\n座位：%1").arg(seatId);
        if (visitorNum > 0) {
            msg += QString("\n\n🎉 您是今天第 %1 位到馆读者！").arg(visitorNum);
        }
        QMessageBox::information(this, "签到成功", msg);
    });
    
    // 定时刷新
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::onTimerUpdate);
    refreshTimer->start(30000); // 30秒刷新一次
    
    // 每秒更新时间状态
    QTimer* secondTimer = new QTimer(this);
    connect(secondTimer, &QTimer::timeout, this, &MainWindow::updateTimeStatusDisplay);
    secondTimer->start(1000);

    // 显示登录对话框
    LoginDialog* loginDialog = new LoginDialog(this);
    connect(loginDialog, &LoginDialog::loginSuccess, [this]() {
        updateUserStatus();
        show();
        showWelcomeDialog();  // 登录成功后显示温馨提示
    });
    
    if (loginDialog->exec() != QDialog::Accepted) {
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }
    delete loginDialog;
    
    // 默认选中第一个校区
    if (campusCombo->count() > 0) {
        onCampusSelected(0);
    }
    
    resize(1280, 800);
    setWindowTitle("📚 图书馆座位预约系统");
}

MainWindow::~MainWindow() {
    LibraryManager::instance().saveData();
}

void MainWindow::setupUI() {
    centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    /* 侧边栏 */
    sideBar = new QWidget();
    sideBar->setFixedWidth(220);
    sideBar->setStyleSheet(
        "QWidget#sideBar { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #1a1a2e, stop:0.5 #16213e, stop:1 #0f3460); "
        "}"
        "QLabel { color: white; background: transparent; }"
        "QPushButton { background-color: rgba(255,255,255,0.1); color: white; border: none; "
        "padding: 12px; text-align: left; font-size: 13px; border-radius: 5px; margin: 2px 5px; }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.2); }"
        "QPushButton:disabled { background-color: rgba(0,0,0,0.2); color: #607D8B; }");
    sideBar->setObjectName("sideBar");
    
    QVBoxLayout* sideLayout = new QVBoxLayout(sideBar);
    sideLayout->setSpacing(5);
    sideLayout->setContentsMargins(10, 15, 10, 15);
    
    // Logo/标题
    QLabel* logoLabel = new QLabel("📚 座位预约");
    logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px 0;");
    logoLabel->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(logoLabel);
    
    // 用户信息区
    QWidget* userInfoWidget = new QWidget();
    userInfoWidget->setStyleSheet("background-color: #1E272C; border-radius: 8px; padding: 10px;");
    QVBoxLayout* userInfoLayout = new QVBoxLayout(userInfoWidget);
    userInfoLayout->setSpacing(5);
    
    userNameLabel = new QLabel("用户名");
    userNameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    userIdLabel = new QLabel("学号: ");
    userIdLabel->setStyleSheet("font-size: 12px; color: #90A4AE;");
    creditLabel = new QLabel("信用分: 100");
    creditLabel->setStyleSheet("font-size: 12px; color: #81C784;");
    
    userInfoLayout->addWidget(userNameLabel);
    userInfoLayout->addWidget(userIdLabel);
    userInfoLayout->addWidget(creditLabel);
    sideLayout->addWidget(userInfoWidget);
    
    sideLayout->addSpacing(15);
    
    // 当前座位状态
    QLabel* statusTitle = new QLabel("📍 当前状态");
    statusTitle->setStyleSheet("font-size: 14px; color: #B0BEC5; padding-top: 10px;");
    sideLayout->addWidget(statusTitle);
    
    currentSeatLabel = new QLabel("暂无预约");
    currentSeatLabel->setStyleSheet(
        "background-color: #1E272C; padding: 15px; border-radius: 5px; "
        "font-size: 13px; color: #CFD8DC;");
    currentSeatLabel->setWordWrap(true);
    sideLayout->addWidget(currentSeatLabel);
    
    // 操作按钮
    sideLayout->addSpacing(10);
    
    btnSignIn = new QPushButton("✅ 签到");
    btnSignIn->setEnabled(false);
    connect(btnSignIn, &QPushButton::clicked, this, &MainWindow::onSignIn);
    sideLayout->addWidget(btnSignIn);
    
    btnTempLeave = new QPushButton("🚶 暂离");
    btnTempLeave->setEnabled(false);
    connect(btnTempLeave, &QPushButton::clicked, this, &MainWindow::onTempLeave);
    sideLayout->addWidget(btnTempLeave);
    
    btnReturn = new QPushButton("↩️ 返回座位");
    btnReturn->setEnabled(false);
    connect(btnReturn, &QPushButton::clicked, this, &MainWindow::onReturnFromLeave);
    sideLayout->addWidget(btnReturn);
    
    btnSignOut = new QPushButton("🚪 签退");
    btnSignOut->setEnabled(false);
    connect(btnSignOut, &QPushButton::clicked, this, &MainWindow::onSignOut);
    sideLayout->addWidget(btnSignOut);
    
    btnCancel = new QPushButton("❌ 取消预约");
    btnCancel->setEnabled(false);
    connect(btnCancel, &QPushButton::clicked, this, &MainWindow::onCancelReservation);
    sideLayout->addWidget(btnCancel);
    
    sideLayout->addStretch();
    
    // 底部功能按钮
    btnFavorites = new QPushButton("❤️ 常用预约");
    connect(btnFavorites, &QPushButton::clicked, this, &MainWindow::showFavorites);
    sideLayout->addWidget(btnFavorites);

    btnHistory = new QPushButton("📋 使用记录");
    connect(btnHistory, &QPushButton::clicked, this, &MainWindow::showHistory);
    sideLayout->addWidget(btnHistory);
    
    btnLogout = new QPushButton("🔓 退出登录");
    btnLogout->setStyleSheet(
        "QPushButton { background-color: #B71C1C; }"
        "QPushButton:hover { background-color: #C62828; }");
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);
    sideLayout->addWidget(btnLogout);
    // 统计分析按钮
    QPushButton* btnStatistics = new QPushButton("📊 数据统计");
    btnStatistics->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; padding: 10px; "
        "border: none; border-radius: 5px; font-size: 14px; }"
        "QPushButton:hover { background-color: #7B1FA2; }");
    connect(btnStatistics, &QPushButton::clicked, this, &MainWindow::showStatistics);
    sideLayout->addWidget(btnStatistics);  // 添加到侧边栏布局
    
    mainLayout->addWidget(sideBar);
    
    /* 主内容区 */
    mainStack = new QStackedWidget();
    mainStack->setStyleSheet(
        "QStackedWidget { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "       stop:0 #f5f7fa, stop:0.5 #e4e8ed, stop:1 #f0f2f5); "
        "}");
    
    // 页面0: 校区选择/楼层列表
    setupCampusPage();
    mainStack->addWidget(campusPage);
    
    // 页面1: 楼层平面图
    floorPlanWidget = new FloorPlanWidget();
    connect(floorPlanWidget, &FloorPlanWidget::zoneSelected, this, &MainWindow::onZoneSelected);
    connect(floorPlanWidget, &FloorPlanWidget::backRequested, this, &MainWindow::onBackToFloorList);
    mainStack->addWidget(floorPlanWidget);
    
    // 页面2: 座位视图
    seatViewWidget = new SeatViewWidget();
    connect(seatViewWidget, &SeatViewWidget::backRequested, this, &MainWindow::onBackToFloorPlan);
    connect(seatViewWidget, &SeatViewWidget::seatSelectedForDate, this, &MainWindow::onSeatSelectedForDate);
    mainStack->addWidget(seatViewWidget);
    
    // 页面3: 统计分析
    statisticsWidget = new StatisticsWidget();
    connect(statisticsWidget, &StatisticsWidget::backRequested, [this](){
        mainStack->setCurrentIndex(0);  // 返回首页
    });
    mainStack->addWidget(statisticsWidget);

    mainLayout->addWidget(mainStack, 1);
}

void MainWindow::updateAnnouncements() {
    if (!announcementLabel) return;

    LibraryManager& mgr = LibraryManager::instance();
    QList<Announcement> announcements = mgr.getAnnouncements();

    QString content;
    int count = 0;
    for (const Announcement& ann : announcements) {
        if (count >= 2) break;  // 只显示前2条
        if (ann.expireTime < QDateTime::currentDateTime()) continue;  // 跳过过期的

        QString prefix = ann.isImportant ? "⚠️ " : "📌 ";
        content += prefix + ann.content + "\n";
        count++;
    }

    if (content.isEmpty()) {
        content = "暂无公告";
    }

    announcementLabel->setText(content.trimmed());
}

void MainWindow::setupCampusPage() {
    campusPage = new QWidget();
    campusPage->setStyleSheet("background: transparent;");
    QVBoxLayout* layout = new QVBoxLayout(campusPage);
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);

    /* 顶部栏（标题 + 规则按钮） */
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    // 标题
    QLabel* titleLabel = new QLabel("📚 选择校区");
    titleLabel->setStyleSheet(
        "font-size: 28px; font-weight: bold; "
        "color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2);");
    topBarLayout->addWidget(titleLabel);

    topBarLayout->addStretch();

    // 当前时间和状态
    timeStatusLabel = new QLabel();
    timeStatusLabel->setStyleSheet(
        "background-color: #E3F2FD; "
        "color: #1565C0; "
        "padding: 8px 15px; "
        "border-radius: 15px; "
        "font-size: 13px; "
        "font-weight: bold; "
        );
    topBarLayout->addWidget(timeStatusLabel);

    topBarLayout->addSpacing(10);

    // 查看预约规则按钮
    QPushButton* rulesBtn = new QPushButton("📋 查看预约规则");
    rulesBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: transparent; "
        "   color: #FF7043; "
        "   border: 2px solid #FF7043; "
        "   padding: 10px 20px; "
        "   border-radius: 8px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { "
        "   background-color: #FF7043; "
        "   color: white; "
        "}"
        );
    connect(rulesBtn, &QPushButton::clicked, this, &MainWindow::showRulesDialog);
    topBarLayout->addWidget(rulesBtn);

    layout->addLayout(topBarLayout);

    /* 公告区域 */
    announcementWidget = new QWidget();
    announcementWidget->setStyleSheet(
        "QWidget#announcementWidget { "
        "   background-color: #FFF8E1; "
        "   border: 1px solid #FFE082; "
        "   border-radius: 10px; "
        "}"
        );
    announcementWidget->setObjectName("announcementWidget");

    QVBoxLayout* annLayout = new QVBoxLayout(announcementWidget);
    annLayout->setContentsMargins(15, 12, 15, 12);
    annLayout->setSpacing(8);

    // 公告标题栏
    QHBoxLayout* annTitleLayout = new QHBoxLayout();
    QLabel* annTitleLabel = new QLabel("📢 公告通知");
    annTitleLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #F57C00; background: transparent;");
    annTitleLayout->addWidget(annTitleLabel);
    annTitleLayout->addStretch();

    QPushButton* viewAllAnnBtn = new QPushButton("查看全部 >");
    viewAllAnnBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #1976D2; font-size: 13px; }"
        "QPushButton:hover { color: #0D47A1; text-decoration: underline; }"
        );
    connect(viewAllAnnBtn, &QPushButton::clicked, this, &MainWindow::showAllAnnouncements);
    annTitleLayout->addWidget(viewAllAnnBtn);
    annLayout->addLayout(annTitleLayout);

    // 公告内容
    announcementLabel = new QLabel();
    announcementLabel->setWordWrap(true);
    announcementLabel->setStyleSheet(
        "color: #E65100; "
        "font-size: 14px; "
        "background: transparent; "
        "line-height: 1.5; "
        );
    annLayout->addWidget(announcementLabel);

    layout->addWidget(announcementWidget);
    /* 顶部栏结束 */
    
    // 校区选择
    QHBoxLayout* campusLayout = new QHBoxLayout();
    QLabel* campusLabel = new QLabel("当前校区:");
    campusLabel->setStyleSheet("font-size: 16px; color: #333;");
    
    campusCombo = new QComboBox();
    campusCombo->setStyleSheet(
        "QComboBox { padding: 12px 15px; font-size: 15px; min-width: 200px; "
        "border: 2px solid #667eea; border-radius: 8px; background-color: white; color: #333; }"
        "QComboBox:hover { border-color: #764ba2; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView { background-color: white; selection-background-color: #667eea; }");
    
    LibraryManager& mgr = LibraryManager::instance();
    for (auto& campus : mgr.getCampuses()) {
        campusCombo->addItem(campus.name, campus.id);
    }
    connect(campusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onCampusSelected);
    
    campusLayout->addWidget(campusLabel);
    campusLayout->addWidget(campusCombo);
    campusLayout->addStretch();
    layout->addLayout(campusLayout);
    
    // 校区图片（占位）
    campusImageLabel = new QLabel();
    campusImageLabel->setFixedHeight(160);
    campusImageLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "   stop:0 #667eea, stop:0.5 #764ba2, stop:1 #f093fb); "
        "border-radius: 12px; color: white; font-size: 18px; font-weight: bold;");
    campusImageLabel->setAlignment(Qt::AlignCenter);
    campusImageLabel->setText("🏛️ 校区概览");
    layout->addWidget(campusImageLabel);
    
    // 楼层选择标题
    QLabel* floorTitle = new QLabel("🏢 选择楼层");
    floorTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #333; margin-top: 10px;");
    layout->addWidget(floorTitle);
    
    // 楼层按钮容器
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: transparent; }"
        "QScrollBar:vertical { background: #E0E0E0; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #9E9E9E; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");
    
    floorButtonsWidget = new QWidget();
    floorButtonsWidget->setStyleSheet("background-color: transparent;");
    scrollArea->setWidget(floorButtonsWidget);
    
    layout->addWidget(scrollArea, 1);
}

void MainWindow::setupStatusBar() {
    statusBar()->setStyleSheet(
        "QStatusBar { background-color: #ECEFF1; border-top: 1px solid #CFD8DC; }"
        "QStatusBar::item { border: none; }");
    statusBar()->showMessage("就绪 | 图书馆座位预约系统 v1.0");
}

void MainWindow::onCampusSelected(int index) {
    if (index < 0) return;

    QString campusId = campusCombo->itemData(index).toString();
    currentCampus = LibraryManager::instance().getCampusById(campusId);

    if (!currentCampus) return;

    // 根据校区设置不同的渐变背景色
    QString gradientStyle;
    if (campusId == "xiaoxiang") {
        // 潇湘校区 - 紫蓝渐变
        gradientStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                        "stop:0 #667eea, stop:0.5 #764ba2, stop:1 #f093fb);";
    } else if (campusId == "yuelu") {
        // 岳麓山校区 - 青蓝渐变
        gradientStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                        "stop:0 #00d2ff, stop:0.5 #3a7bd5, stop:1 #00d2ff);";
    } else if (campusId == "tianxin") {
        // 天心校区 - 橙红渐变
        gradientStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                        "stop:0 #f5af19, stop:0.5 #f12711, stop:1 #f5af19);";
    } else if (campusId == "xinglin") {
        // 杏林校区 - 绿色渐变
        gradientStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                        "stop:0 #56ab2f, stop:0.5 #a8e063, stop:1 #56ab2f);";
    } else {
        gradientStyle = "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                        "stop:0 #667eea, stop:1 #764ba2);";
    }

    campusImageLabel->setStyleSheet(
        gradientStyle +
        "border-radius: 12px; color: white; font-size: 18px; font-weight: bold;");

    campusImageLabel->setText(QString("🏛️ %1\n总座位: %2 | 空闲: %3")
                                  .arg(currentCampus->name)
                                  .arg(currentCampus->getTotalCount())
                                  .arg(currentCampus->getAvailableCount()));

    updateFloorList();
    updateAnnouncements();
    updateTimeStatusDisplay();
}

void MainWindow::updateFloorList() {
    if (!currentCampus) return;
    
    // 清除旧按钮
    for (auto btn : floorButtons) {
        btn->deleteLater();
    }
    floorButtons.clear();
    
    // 删除旧布局
    if (floorButtonsWidget->layout()) {
        QLayoutItem* item;
        while ((item = floorButtonsWidget->layout()->takeAt(0)) != nullptr) {
            delete item;
        }
        delete floorButtonsWidget->layout();
    }
    
    // 创建新的网格布局
    QGridLayout* grid = new QGridLayout(floorButtonsWidget);
    grid->setSpacing(15);
    grid->setContentsMargins(10, 10, 10, 10);
    
    int col = 0;
    int row = 0;
    int maxCols = 4;
    
    for (auto& floor : currentCampus->floors) {
        QPushButton* btn = new QPushButton();
        btn->setFixedSize(180, 100);
        
        // 根据空闲率设置颜色
        double ratio = floor.getTotalCount() > 0 ? 
                       (double)floor.getAvailableCount() / floor.getTotalCount() : 0;
        QString bgColor, borderColor;
        if (ratio > 0.5) {
            bgColor = "#C8E6C9";
            borderColor = "#4CAF50";
        } else if (ratio > 0.2) {
            bgColor = "#FFF9C4";
            borderColor = "#FFC107";
        } else {
            bgColor = "#FFCDD2";
            borderColor = "#F44336";
        }
        
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; border: 3px solid %2; "
            "border-radius: 12px; font-size: 14px; color: #333; font-weight: bold; }"
            "QPushButton:hover { border-width: 4px; background-color: %3; }")
            .arg(bgColor).arg(borderColor).arg(QColor(bgColor).lighter(105).name()));
        
        btn->setText(QString("%1楼\n\n🪑 空座: %2 / %3")
             .arg(floor.level)
             .arg(floor.getAvailableCount())
             .arg(floor.getTotalCount()));
        
        // 设置详细的tooltip提示
        btn->setToolTip(QString(
            "<div style='font-size:14px; padding:5px;'>"
            "<b>%1楼</b><br><br>"
            "📊 座位统计：<br>"
            "&nbsp;&nbsp;• 总座位: %2<br>"
            "&nbsp;&nbsp;• 空闲: <span style='color:#4CAF50;'>%3</span><br>"
            "&nbsp;&nbsp;• 使用率: %4%<br><br>"
            "📍 区域数量: %5个"
            "</div>")
            .arg(floor.level)
            .arg(floor.getTotalCount())
            .arg(floor.getAvailableCount())
            .arg(floor.getTotalCount() > 0 ?
                     100 - floor.getAvailableCount() * 100 / floor.getTotalCount() : 0)
            .arg(floor.zones.size()));

        connect(btn, &QPushButton::clicked, [this, level = floor.level]() {
            onFloorSelected(level);
        });
        
        grid->addWidget(btn, row, col);
        floorButtons.append(btn);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
    
    // 添加弹性空间
    grid->setRowStretch(row + 1, 1);
}

void MainWindow::onFloorSelected(int floorLevel) {
    if (!currentCampus) return;
    
    currentFloor = currentCampus->findFloor(floorLevel);
    if (!currentFloor) return;
    
    floorPlanWidget->loadFloor(currentFloor, currentCampus->name);
    mainStack->setCurrentWidget(floorPlanWidget);
}

void MainWindow::onZoneSelected(Zone* zone, const QString& floorName) {
    if (!zone) return;
    
    currentZone = zone;
    seatViewWidget->loadZone(zone, floorName);
    mainStack->setCurrentWidget(seatViewWidget);
}

void MainWindow::onSeatSelected(Seat* seat) {
    if (!seat || !currentCampus || !currentFloor || !currentZone) return;
    
    LibraryManager& mgr = LibraryManager::instance();
    
    if (mgr.reserveSeat(currentCampus->id, currentFloor->level, currentZone->id, seat->id)) {
        QMessageBox::information(this, "预约成功", 
                                 QString("成功预约座位: %1\n\n请在15分钟内签到").arg(seat->id));
        updateUserStatus();
        seatViewWidget->refresh();
    }
}

void MainWindow::onBackToFloorList() {
    mainStack->setCurrentWidget(campusPage);
    updateFloorList();
}

void MainWindow::onBackToFloorPlan() {
    mainStack->setCurrentWidget(floorPlanWidget);
    floorPlanWidget->refresh();
}

void MainWindow::onDataChanged() {
    updateUserStatus();
    refreshCurrentView();
}

void MainWindow::onTimerUpdate() {
    updateUserStatus();
    refreshCurrentView();
}

void MainWindow::updateUserStatus() {
    LibraryManager& mgr = LibraryManager::instance();
    User* user = mgr.getCurrentUser();
    
    if (!user) return;
    
    userNameLabel->setText(user->name);
    userIdLabel->setText(QString("学号: %1").arg(user->userId));
    
    // 信用分颜色
    QString creditColor = user->creditScore >= 80 ? "#81C784" : 
                          (user->creditScore >= 60 ? "#FFD54F" : "#E57373");
    creditLabel->setText(QString("信用分: %1").arg(user->creditScore));
    creditLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(creditColor));
    
    // 当前座位状态
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (seat) {
        QString statusText = QString("座位: %1\n状态: %2").arg(seat->id).arg(seat->getStatusName());
        
        if (seat->status == Reserved) {
            int minutes = seat->reserveTime.secsTo(QDateTime::currentDateTime()) / 60;
            int remaining = 15 - minutes;
            statusText += QString("\n剩余签到时间: %1分钟").arg(qMax(0, remaining));
        } else if (seat->status == TemporaryLeave) {
            int minutes = seat->leaveTime.secsTo(QDateTime::currentDateTime()) / 60;
            int remaining = 30 - minutes;
            statusText += QString("\n剩余暂离时间: %1分钟").arg(qMax(0, remaining));
        }
        
        currentSeatLabel->setText(statusText);
        currentSeatLabel->setStyleSheet(
            "background-color: #1E272C; padding: 15px; border-radius: 5px; "
            "font-size: 13px; color: #4CAF50; border-left: 4px solid #4CAF50;");
        
        // 更新按钮状态
        btnSignIn->setEnabled(seat->status == Reserved);
        btnTempLeave->setEnabled(seat->status == Occupied);
        btnReturn->setEnabled(seat->status == TemporaryLeave);
        btnSignOut->setEnabled(seat->status == Occupied || seat->status == TemporaryLeave);
        btnCancel->setEnabled(seat->status == Reserved);
    } else {
        currentSeatLabel->setText("暂无预约\n\n选择座位开始预约");
        currentSeatLabel->setStyleSheet(
            "background-color: #1E272C; padding: 15px; border-radius: 5px; "
            "font-size: 13px; color: #CFD8DC;");
        
        btnSignIn->setEnabled(false);
        btnTempLeave->setEnabled(false);
        btnReturn->setEnabled(false);
        btnSignOut->setEnabled(false);
        btnCancel->setEnabled(false);
    }
}

void MainWindow::refreshCurrentView() {
    int currentIndex = mainStack->currentIndex();
    
    if (currentIndex == 0) {
        updateFloorList();
    } else if (currentIndex == 1) {
        floorPlanWidget->refresh();
    } else if (currentIndex == 2) {
        seatViewWidget->refresh();
    }
}

void MainWindow::onSignIn() {
    LibraryManager& mgr = LibraryManager::instance();
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (seat && mgr.signIn(seat->id)) {
        updateUserStatus();
        refreshCurrentView();
    }
}

void MainWindow::onSignOut() {
    LibraryManager& mgr = LibraryManager::instance();
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (!seat) return;
    
    int ret = QMessageBox::question(this, "确认签退", "确定要签退吗？");
    if (ret == QMessageBox::Yes && mgr.signOut(seat->id)) {
        QMessageBox::information(this, "签退成功", "签退成功，信用分+1");
        updateUserStatus();
        refreshCurrentView();
    }
}

void MainWindow::onTempLeave() {
    LibraryManager& mgr = LibraryManager::instance();
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (seat && mgr.setTemporaryLeave(seat->id)) {
        QMessageBox::information(this, "暂离", "已设置暂离状态\n请在30分钟内返回");
        updateUserStatus();
        refreshCurrentView();
    }
}

void MainWindow::onReturnFromLeave() {
    LibraryManager& mgr = LibraryManager::instance();
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (seat && mgr.returnFromLeave(seat->id)) {
        QMessageBox::information(this, "返回成功", "欢迎回来");
        updateUserStatus();
        refreshCurrentView();
    }
}

void MainWindow::onCancelReservation() {
    LibraryManager& mgr = LibraryManager::instance();
    Seat* seat = mgr.getUserCurrentSeat();
    
    if (!seat) return;
    
    int ret = QMessageBox::question(this, "确认取消", "确定要取消预约吗？");
    if (ret == QMessageBox::Yes && mgr.cancelReservation(seat->id)) {
        QMessageBox::information(this, "已取消", "预约已取消");
        updateUserStatus();
        refreshCurrentView();
    }
}

void MainWindow::onLogout() {
    int ret = QMessageBox::question(this, "退出登录", "确定要退出登录吗？");
    if (ret != QMessageBox::Yes) return;
    
    LibraryManager::instance().logout();
    
    // 重新显示登录对话框
    hide();
    LoginDialog* loginDialog = new LoginDialog(this);
    connect(loginDialog, &LoginDialog::loginSuccess, [this]() {
        updateUserStatus();
        if (campusCombo->count() > 0) {
            onCampusSelected(0);
        }
        mainStack->setCurrentWidget(campusPage);
        show();
    });
    
    if (loginDialog->exec() != QDialog::Accepted) {
        close();
    }
    delete loginDialog;
}

void MainWindow::showHistory() {
    LibraryManager& mgr = LibraryManager::instance();
    User* user = mgr.getCurrentUser();
    if (!user) return;
    
    QList<UsageRecord> records = mgr.getUserHistory(user->userId);
    
    // 创建历史记录对话框
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("使用记录");
    dialog->resize(700, 500);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QLabel* titleLabel = new QLabel(QString("📋 %1 的使用记录").arg(user->name));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(titleLabel);
    
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"时间", "座位号", "操作", "备注"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setRowCount(records.size());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    
    for (int i = 0; i < records.size(); i++) {
        const UsageRecord& rec = records[i];
        table->setItem(i, 0, new QTableWidgetItem(rec.startTime.toString("yyyy-MM-dd HH:mm")));
        table->setItem(i, 1, new QTableWidgetItem(rec.seatId));
        table->setItem(i, 2, new QTableWidgetItem(rec.action));
        table->setItem(i, 3, new QTableWidgetItem(""));
    }
    
    layout->addWidget(table);
    
    QPushButton* closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog->exec();
    delete dialog;
}

void MainWindow::showUserInfo() {
    LibraryManager& mgr = LibraryManager::instance();
    User* user = mgr.getCurrentUser();
    if (!user) return;
    
    QString info = QString(
       "用户信息\n\n"
       "姓名: %1\n"
       "学号: %2\n"
       "身份: %3\n"
       "信用分: %4\n"
       "本月违约: %5")
       .arg(user->name)
       .arg(user->userId)
       .arg(user->getTypeName())
       .arg(user->creditScore)
       .arg(user->monthlyViolations);
    
    QMessageBox::information(this, "用户信息", info);
}

void MainWindow::showStatistics() {
    statisticsWidget->refresh();
    mainStack->setCurrentWidget(statisticsWidget);
}

void MainWindow::showWelcomeDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("温馨提示");
    dialog->setFixedSize(480, 320);
    dialog->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 20);
    layout->setSpacing(15);

    // 顶部标题栏
    QLabel* headerLabel = new QLabel("⚠️ 温馨提示");
    headerLabel->setStyleSheet(
        "background-color: #FF7043; "
        "color: white; "
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 18px; "
        );
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // 提示内容
    QLabel* contentLabel = new QLabel();
    contentLabel->setText(
        "<div style='padding: 15px; line-height: 1.8;'>"
        "<p style='color: #D84315; font-size: 15px; font-weight: bold;'>安全须知：</p>"
        "<p style='color: #E65100; font-size: 14px;'>"
        "1. 请勿携带电动车电池、易燃易爆危险物品入馆！<br>"
        "2. 请勿在馆内使用大功率电器设备！"
        "</p>"
        "<hr style='border: 1px solid #FFE0B2;'>"
        "<p style='color: #1565C0; font-size: 14px;'>"
        "📌 预约成功后请在 <b style='color:#D32F2F;'>15分钟内</b> 完成签到<br>"
        "📌 暂离时间不得超过 <b style='color:#D32F2F;'>30分钟</b><br>"
        "📌 每周最多取消预约 <b style='color:#D32F2F;'>1次</b>"
        "</p>"
        "</div>"
        );
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet("background-color: #FFF8E1; margin: 15px; border-radius: 10px;");
    layout->addWidget(contentLabel);

    layout->addStretch();

    // 确定按钮
    QPushButton* okBtn = new QPushButton("我已知晓");
    okBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #4CAF50; "
        "   color: white; "
        "   padding: 14px 50px; "
        "   border: none; "
        "   border-radius: 8px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #43A047; }"
        );
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::showRulesDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("座位预约规则");
    dialog->setFixedSize(650, 750);
    dialog->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(0);

    // 顶部标题
    QLabel* headerLabel = new QLabel("📋 座位预约规则（暂行）");
    headerLabel->setStyleSheet(
        "background-color: #1976D2; "
        "color: white; "
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 18px; "
        );
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: white; }"
        "QScrollBar:vertical { background: #E0E0E0; width: 10px; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #9E9E9E; border-radius: 5px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        );

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(12);

    // 规则内容
    QStringList rules;
    rules << "<b>1. 适用范围</b><br>"
             "本座位管理系统具备预约、签到、签离等功能，涵盖潇湘校区图书馆各楼层自修室座位。"
             "座位开放时间：<span style='color:#1976D2; font-weight:bold;'>7:00 - 22:30</span>，"
             "预约截止时间：<span style='color:#1976D2; font-weight:bold;'>21:30</span>。";

    rules << "<b>2. 预约方式</b><br>"
             "• 读者可预约 <span style='color:#E65100; font-weight:bold;'>当日或次日</span> 的座位<br>"
             "• 预约成功后可使用该座位至当日闭馆<br>"
             "• 登录方式：学校统一身份认证（学号/工号）";

    rules << "<b>3. 签到规则</b><br>"
             "• 预约当日座位：需在 <span style='color:#D32F2F; font-weight:bold;'>15分钟内</span> 完成签到<br>"
             "• 预约次日座位：需在次日 <span style='color:#D32F2F; font-weight:bold;'>8:30前</span> 完成签到<br>"
             "• 签到方式：通过门禁刷卡或在系统中点击签到";

    rules << "<b>4. 取消预约</b><br>"
             "• 预约当日座位后 <span style='color:#D32F2F; font-weight:bold;'>每周只能取消1次</span><br>"
             "• 预约次日座位需在次日8:30前取消<br>"
             "• 操作路径：侧边栏 → 取消预约";

    rules << "<b>5. 暂离规则</b><br>"
             "• 临时离开前请点击「暂离」保留座位<br>"
             "• 暂离时间最长 <span style='color:#D32F2F; font-weight:bold;'>30分钟</span><br>"
             "• 返回后需点击「返回座位」继续使用<br>"
             "• 超时未归系统将自动释放座位并记录违规";

    rules << "<b>6. 签离规则</b><br>"
             "• 离馆时请点击「签退」释放座位<br>"
             "• 未签离的读者，系统将在闭馆后自动处理并可能记录违规";

    rules << "<b>7. 违规处理</b><br>"
             "<span style='color:#D32F2F;'>"
             "• 预约后未签到：记违规1次，<b>扣信用分10分</b><br>"
             "• 暂离超时未归：记违规1次，<b>扣信用分10分</b><br>"
             "• 未签离：记违规1次，<b>扣信用分10分</b><br>"
             "• <b>每月违规满3次</b>：暂停预约权利 <b>3天</b><br>"
             "• <b>信用分低于70分</b>：暂停预约权利"
             "</span>";

    rules << "<b>8. 信用分规则</b><br>"
             "• 初始信用分：<span style='color:#4CAF50; font-weight:bold;'>100分</span><br>"
             "• 每次违规：<span style='color:#D32F2F;'>-10分</span><br>"
             "• 每到馆使用1小时：<span style='color:#4CAF50;'>+1分</span>（上限100分）<br>"
             "• 信用分低于70分将无法预约座位";

    rules << "<b>9. 文明须知</b><br>"
             "<span style='color:#E65100; font-weight:bold;'>"
             "⚠️ 请勿携带电动车电池、易燃易爆危险物品入馆！<br>"
             "⚠️ 请勿在馆内使用大功率电器设备！<br>"
             "</span>"
             "• 请维护室内秩序，不得以任何形式占座<br>"
             "• 不得随意移动桌椅<br>"
             "• 工作人员有权清理占座物品，遗失损坏由占座者自负";

    for (const QString& rule : rules) {
        QLabel* ruleLabel = new QLabel(rule);
        ruleLabel->setWordWrap(true);
        ruleLabel->setStyleSheet(
            "QLabel { "
            "   background-color: #FAFAFA; "
            "   padding: 15px; "
            "   border-radius: 8px; "
            "   border-left: 4px solid #1976D2; "
            "   font-size: 14px; "
            "   color: #333; "
            "}"
            );
        contentLayout->addWidget(ruleLabel);
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea, 1);

    // 底部按钮
    QPushButton* closeBtn = new QPushButton("我已阅读并知晓");
    closeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #1976D2; "
        "   color: white; "
        "   padding: 14px 50px; "
        "   border: none; "
        "   border-radius: 8px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        );
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(20, 15, 20, 5);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::showFavorites() {
    LibraryManager& mgr = LibraryManager::instance();
    User* user = mgr.getCurrentUser();
    if (!user) return;

    QList<Seat*> favorites = mgr.getUserFavoriteSeats();

    // 创建常用预约对话框
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("常用预约");
    dialog->setFixedSize(500, 550);
    dialog->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(0);

    // 顶部标题
    QLabel* headerLabel = new QLabel("❤️ 常用预约");
    headerLabel->setStyleSheet(
        "background-color: #26C6DA; "
        "color: white; "
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 18px; "
        );
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // 提示信息
    QLabel* tipLabel = new QLabel("🔊 这里是您常用的座位，您可直接预约。");
    tipLabel->setStyleSheet(
        "background-color: #FFF8E1; "
        "color: #F57C00; "
        "padding: 12px 15px; "
        "font-size: 14px; "
        );
    layout->addWidget(tipLabel);

    // 常用座位标题
    QLabel* sectionLabel = new QLabel("常用座位");
    sectionLabel->setStyleSheet(
        "background-color: #26C6DA; "
        "color: white; "
        "padding: 12px 20px; "
        "font-size: 16px; "
        "font-weight: bold; "
        "margin: 15px 20px 10px 20px; "
        "border-radius: 8px; "
        );
    sectionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(sectionLabel);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: white; }"
        "QScrollBar:vertical { background: #E0E0E0; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #BDBDBD; border-radius: 4px; }"
        );

    QWidget* listWidget = new QWidget();
    QVBoxLayout* listLayout = new QVBoxLayout(listWidget);
    listLayout->setContentsMargins(20, 10, 20, 10);
    listLayout->setSpacing(8);

    if (favorites.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无常用座位\n\n使用座位后会自动记录到这里");
        emptyLabel->setStyleSheet(
            "color: #9E9E9E; "
            "font-size: 14px; "
            "padding: 40px; "
            );
        emptyLabel->setAlignment(Qt::AlignCenter);
        listLayout->addWidget(emptyLabel);
    } else {
        for (Seat* seat : favorites) {
            if (!seat) continue;

            // 创建座位项
            QWidget* itemWidget = new QWidget();
            itemWidget->setStyleSheet(
                "QWidget { "
                "   background-color: #FAFAFA; "
                "   border: 1px solid #E0E0E0; "
                "   border-radius: 8px; "
                "}"
                "QWidget:hover { background-color: #F5F5F5; }"
                );
            itemWidget->setFixedHeight(60);

            QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
            itemLayout->setContentsMargins(15, 10, 15, 10);

            // 座位信息
            QString seatInfo = seat->id;
            // 尝试解析座位ID获取更多信息 (如 XF3A001 -> 潇湘校区馆-3楼-A区)
            if (seat->id.startsWith("XF")) {
                int floor = seat->id.mid(2, 1).toInt();
                QString zone = seat->id.mid(3, 1);
                seatInfo = QString("潇湘校区馆-%1楼-%2区%3").arg(floor).arg(zone).arg(seat->id);
            }

            QLabel* infoLabel = new QLabel(seatInfo);
            infoLabel->setStyleSheet(
                "color: #333; "
                "font-size: 14px; "
                "background: transparent; "
                "border: none; "
                );
            itemLayout->addWidget(infoLabel, 1);

            // 预约按钮
            QPushButton* reserveBtn = new QPushButton("预约");
            reserveBtn->setFixedSize(70, 36);
            reserveBtn->setEnabled(seat->status == Available);
            reserveBtn->setStyleSheet(
                "QPushButton { "
                "   background-color: #26C6DA; "
                "   color: white; "
                "   border: none; "
                "   border-radius: 6px; "
                "   font-size: 14px; "
                "   font-weight: bold; "
                "}"
                "QPushButton:hover { background-color: #00ACC1; }"
                "QPushButton:disabled { "
                "   background-color: #E0E0E0; "
                "   color: #9E9E9E; "
                "}"
                );

            // 连接预约按钮
            connect(reserveBtn, &QPushButton::clicked, [this, dialog, seat]() {
                if (seat->status != Available) {
                    QMessageBox::warning(this, "提示", "该座位当前不可用");
                    return;
                }

                int ret = QMessageBox::question(this, "确认预约",
                                                QString("确定要预约座位 %1 吗？\n\n预约后请在15分钟内签到").arg(seat->id),
                                                QMessageBox::Yes | QMessageBox::No);

                if (ret == QMessageBox::Yes) {
                    // 解析座位ID获取校区、楼层、区域信息
                    LibraryManager& mgr = LibraryManager::instance();

                    // 简化处理：遍历查找座位所属信息
                    bool found = false;
                    for (auto& campus : mgr.getCampuses()) {
                        for (auto& floor : campus.floors) {
                            for (auto& zone : floor.zones) {
                                for (auto& s : zone.seats) {
                                    if (s.id == seat->id) {
                                        if (mgr.reserveSeat(campus.id, floor.level, zone.id, seat->id)) {
                                            QMessageBox::information(this, "预约成功",
                                                                     QString("成功预约座位: %1\n\n请在15分钟内签到").arg(seat->id));
                                            updateUserStatus();
                                            dialog->accept();
                                        }
                                        found = true;
                                        break;
                                    }
                                }
                                if (found) break;
                            }
                            if (found) break;
                        }
                        if (found) break;
                    }
                }
            });

            itemLayout->addWidget(reserveBtn);
            listLayout->addWidget(itemWidget);
        }
    }

    listLayout->addStretch();
    scrollArea->setWidget(listWidget);
    layout->addWidget(scrollArea, 1);

    // 关闭按钮
    QPushButton* closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #78909C; "
        "   color: white; "
        "   padding: 12px 40px; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 15px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #607D8B; }"
        );
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(20, 10, 20, 5);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::updateTimeStatusDisplay() {
    if (!timeStatusLabel) return;

    QDateTime now = QDateTime::currentDateTime();
    QTime currentTime = now.time();

    QString dateStr = now.toString("yyyy-MM-dd ddd");
    QString timeStr = now.toString("HH:mm:ss");

    LibraryManager& mgr = LibraryManager::instance();
    QString status;
    QString bgColor, textColor;

    if (mgr.canReserve()) {
        status = "预约开放中";
        bgColor = "#E8F5E9";
        textColor = "#2E7D32";
    } else if (mgr.isOpenTime()) {
        status = "今日预约已截止";
        bgColor = "#FFF3E0";
        textColor = "#E65100";
    } else {
        status = "已闭馆";
        bgColor = "#ECEFF1";
        textColor = "#607D8B";
    }

    timeStatusLabel->setText(QString("📅 %1  🕐 %2  |  %3").arg(dateStr).arg(timeStr).arg(status));
    timeStatusLabel->setStyleSheet(QString(
                                       "background-color: %1; "
                                       "color: %2; "
                                       "padding: 8px 15px; "
                                       "border-radius: 15px; "
                                       "font-size: 13px; "
                                       "font-weight: bold; "
                                       ).arg(bgColor).arg(textColor));
}

void MainWindow::showAllAnnouncements() {
    LibraryManager& mgr = LibraryManager::instance();
    QList<Announcement> announcements = mgr.getAnnouncements();

    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("公告通知");
    dialog->setFixedSize(550, 500);
    dialog->setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 15);
    layout->setSpacing(0);

    // 顶部标题
    QLabel* headerLabel = new QLabel("📢 公告通知");
    headerLabel->setStyleSheet(
        "background-color: #FF9800; "
        "color: white; "
        "font-size: 20px; "
        "font-weight: bold; "
        "padding: 18px; "
        );
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: white; }"
        "QScrollBar:vertical { background: #E0E0E0; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #BDBDBD; border-radius: 4px; }"
        );

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 15, 20, 15);
    contentLayout->setSpacing(15);

    if (announcements.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无公告");
        emptyLabel->setStyleSheet("color: #9E9E9E; font-size: 16px; padding: 50px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(emptyLabel);
    } else {
        for (const Announcement& ann : announcements) {
            if (ann.expireTime < QDateTime::currentDateTime()) continue;

            QWidget* annItem = new QWidget();
            annItem->setStyleSheet(
                "QWidget { "
                "   background-color: #FFFDE7; "
                "   border: 1px solid #FFF59D; "
                "   border-radius: 8px; "
                "}"
                );

            QVBoxLayout* itemLayout = new QVBoxLayout(annItem);
            itemLayout->setContentsMargins(15, 12, 15, 12);
            itemLayout->setSpacing(8);

            // 标题
            QString titlePrefix = ann.isImportant ? "⚠️ " : "📌 ";
            QLabel* titleLabel = new QLabel(titlePrefix + ann.title);
            titleLabel->setStyleSheet(
                QString("font-size: 16px; font-weight: bold; color: %1; background: transparent;")
                    .arg(ann.isImportant ? "#D32F2F" : "#F57C00")
                );
            itemLayout->addWidget(titleLabel);

            // 内容
            QLabel* contentLabel = new QLabel(ann.content);
            contentLabel->setWordWrap(true);
            contentLabel->setStyleSheet("font-size: 14px; color: #333; background: transparent; line-height: 1.5;");
            itemLayout->addWidget(contentLabel);

            // 时间
            QLabel* timeLabel = new QLabel(QString("发布时间: %1").arg(ann.createTime.toString("yyyy-MM-dd")));
            timeLabel->setStyleSheet("font-size: 12px; color: #9E9E9E; background: transparent;");
            itemLayout->addWidget(timeLabel);

            contentLayout->addWidget(annItem);
        }
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea, 1);

    // 关闭按钮
    QPushButton* closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #FF9800; "
        "   color: white; "
        "   padding: 12px 40px; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 15px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #F57C00; }"
        );
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(20, 10, 20, 5);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::onSeatSelectedForDate(Seat* seat, bool isTomorrow) {
    if (!seat || !currentCampus || !currentFloor || !currentZone) return;

    LibraryManager& mgr = LibraryManager::instance();

    if (mgr.reserveSeatForDate(currentCampus->id, currentFloor->level, currentZone->id, seat->id, isTomorrow)) {
        QString signInTip = isTomorrow ? "请在明早8:30前签到" : "请在15分钟内签到";
        QMessageBox::information(this, "预约成功",
                                 QString("成功预约座位: %1\n\n%2").arg(seat->id).arg(signInTip));
        updateUserStatus();
        seatViewWidget->refresh();
    }
}
