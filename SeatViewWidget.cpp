#include "SeatViewWidget.h"
#include "LibraryManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMessageBox>
#include <QHeaderView>
#include <QToolTip>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QGridLayout>

/* SeatButton 实现 */
SeatButton::SeatButton(Seat* s, QWidget* parent)
    : QPushButton(parent), seat(s), isHovered(false) {
    setFixedSize(45, 45);
    setCursor(Qt::PointingHandCursor);
    updateTooltip();

    connect(this, &QPushButton::clicked, [this]() {
        emit seatClicked(seat);
    });
}

void SeatButton::updateTooltip() {
    if (!seat) return;

    QString statusText = seat->getStatusName();
    QString powerText = seat->hasPower ? "有电源" : "无电源";
    QString actionTip;

    switch(seat->status) {
    case Available:
        actionTip = ">>> 点击预约此座位 <<<";
        break;
    case Reserved:
        actionTip = QString("剩余签到: %1 分钟").arg(seat->getRemainingSignInMinutes());
        break;
    case TemporaryLeave:
        actionTip = QString("剩余暂离: %1 分钟").arg(seat->getRemainingLeaveMinutes());
        break;
    case Occupied:
        actionTip = "使用中";
        break;
    case Broken:
        actionTip = "暂停使用";
        break;
    default:
        actionTip = "";
    }

    QString tip = QString(
                      "【座位 %1】\n"
                      "-------------------\n"
                      "状态: %2\n"
                      "电源: %3\n"
                      "-------------------\n"
                      "%4")
                      .arg(seat->displayName)
                      .arg(statusText)
                      .arg(powerText)
                      .arg(actionTip);

    if (!seat->currentUserId.isEmpty() && seat->status != Available) {
        tip = QString(
                  "【座位 %1】\n"
                  "-------------------\n"
                  "状态: %2\n"
                  "电源: %3\n"
                  "使用者: %4\n"
                  "-------------------\n"
                  "%5")
                  .arg(seat->displayName)
                  .arg(statusText)
                  .arg(powerText)
                  .arg(seat->currentUserId)
                  .arg(actionTip);
    }

    setToolTip(tip);
}

void SeatButton::updateStatus() {
    updateTooltip();
    update();
}

void SeatButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor = seat->getStatusColor();
    if (isHovered) {
        bgColor = bgColor.lighter(120);
    }

    // 绘制背景
    painter.setBrush(bgColor);
    painter.setPen(QPen(bgColor.darker(130), 1));
    painter.drawRoundedRect(2, 2, width() - 4, height() - 4, 6, 6);

    // 绘制座位号（取后3-4位）
    painter.setPen(Qt::white);
    QFont font("Arial", 8, QFont::Bold);
    painter.setFont(font);

    QString displayId = seat->id.right(4);
    if (displayId.startsWith('0')) displayId = displayId.mid(1);
    painter.drawText(rect(), Qt::AlignCenter, displayId);

    // 如果有电源，绘制小图标
    if (seat->hasPower) {
        painter.setFont(QFont("Arial", 6));
        painter.drawText(width() - 12, 10, "⚡");
    }
}

void SeatButton::enterEvent(QEnterEvent*) {
    isHovered = true;
    update();
}

void SeatButton::leaveEvent(QEvent*) {
    isHovered = false;
    update();
}

/* SeatViewWidget 实现 */
SeatViewWidget::SeatViewWidget(QWidget* parent)
    : QWidget(parent),
    currentZone(nullptr),
    btnBack(nullptr),
    btnToggle(nullptr),
    titleLabel(nullptr),
    infoLabel(nullptr),
    filterCombo(nullptr),
    legendWidget(nullptr),
    stackedWidget(nullptr),
    mapPage(nullptr),
    mapScrollArea(nullptr),
    mapContainer(nullptr),
    listTable(nullptr),
    btnToday(nullptr),
    btnTomorrow(nullptr),
    timeRangeLabel(nullptr),
    reserveStatusLabel(nullptr)
{
    setupUI();

    // 初始化日期
    selectedDate = QDate::currentDate();

    // 定时器更新时间状态
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        if (btnToday && btnTomorrow && reserveStatusLabel) {
            updateTimeStatus();
            updateDateButtons();
        }
    });
    timer->start(1000);
}

void SeatViewWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    /* 顶部栏 */
    QHBoxLayout* topBar = new QHBoxLayout();

    btnBack = new QPushButton("← 返回");
    btnBack->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 8px 20px; "
        "border: none; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #546E7A; }");
    connect(btnBack, &QPushButton::clicked, this, &SeatViewWidget::backRequested);

    titleLabel = new QLabel("选择座位");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");

    btnToggle = new QPushButton("切换为列表模式");
    btnToggle->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; padding: 8px 15px; "
        "border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #1976D2; }");
    connect(btnToggle, &QPushButton::clicked, this, &SeatViewWidget::toggleViewMode);

    filterCombo = new QComboBox();
    filterCombo->addItem("全部座位", -1);
    filterCombo->addItem("仅显示可用", Available);
    filterCombo->addItem("仅显示有电源", 100);
    filterCombo->setStyleSheet("padding: 5px; min-width: 120px;");
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SeatViewWidget::onFilterChanged);

    topBar->addWidget(btnBack);
    topBar->addWidget(titleLabel);
    topBar->addStretch();
    topBar->addWidget(new QLabel("筛选:"));
    topBar->addWidget(filterCombo);
    topBar->addWidget(btnToggle);

    mainLayout->addLayout(topBar);

    /* 信息栏 */
    infoLabel = new QLabel();
    infoLabel->setStyleSheet(
        "QLabel { background-color: #E3F2FD; padding: 10px; border-radius: 5px; "
        "color: #1565C0; font-size: 13px; }");
    mainLayout->addWidget(infoLabel);

    /* 日期选择和时间状态 */
    QHBoxLayout* dateTimeLayout = new QHBoxLayout();
    dateTimeLayout->setSpacing(15);

    btnToday = new QPushButton();
    btnToday->setCheckable(true);
    btnToday->setChecked(true);
    updateDateButtons();
    connect(btnToday, &QPushButton::clicked, [this]() {
        selectedDate = QDate::currentDate();
        btnToday->setChecked(true);
        btnTomorrow->setChecked(false);
        updateDateButtons();
        updateTimeStatus();
    });
    dateTimeLayout->addWidget(btnToday);

    btnTomorrow = new QPushButton();
    btnTomorrow->setCheckable(true);
    btnTomorrow->setChecked(false);
    connect(btnTomorrow, &QPushButton::clicked, [this]() {
        selectedDate = QDate::currentDate().addDays(1);
        btnToday->setChecked(false);
        btnTomorrow->setChecked(true);
        updateDateButtons();
        updateTimeStatus();
    });
    dateTimeLayout->addWidget(btnTomorrow);

    dateTimeLayout->addStretch();

    timeRangeLabel = new QLabel("07:00-22:30");
    timeRangeLabel->setStyleSheet(
        "background-color: #26C6DA; "
        "color: white; "
        "padding: 8px 20px; "
        "border-radius: 15px; "
        "font-size: 14px; "
        "font-weight: bold; "
        );
    dateTimeLayout->addWidget(timeRangeLabel);

    dateTimeLayout->addStretch();

    reserveStatusLabel = new QLabel();
    reserveStatusLabel->setStyleSheet(
        "padding: 8px 15px; "
        "border-radius: 5px; "
        "font-size: 13px; "
        "font-weight: bold; "
        );
    dateTimeLayout->addWidget(reserveStatusLabel);

    mainLayout->addLayout(dateTimeLayout);

    selectedDate = QDate::currentDate();
    updateTimeStatus();

    /* 图例 */
    legendWidget = new QWidget();
    QHBoxLayout* legendLayout = new QHBoxLayout(legendWidget);
    legendLayout->setContentsMargins(0, 5, 0, 5);

    auto addLegend = [legendLayout](const QColor& color, const QString& text) {
        QLabel* colorLabel = new QLabel();
        colorLabel->setFixedSize(20, 20);
        colorLabel->setStyleSheet(QString("background-color: %1; border-radius: 3px;").arg(color.name()));
        legendLayout->addWidget(colorLabel);
        QLabel* textLabel = new QLabel(text);
        textLabel->setStyleSheet("color: #333; font-size: 13px;");
        legendLayout->addWidget(textLabel);
        legendLayout->addSpacing(15);
    };

    addLegend(QColor(76, 175, 80), "可用");
    addLegend(QColor(158, 158, 158), "使用中");
    addLegend(QColor(33, 150, 243), "已预约");
    addLegend(QColor(255, 193, 7), "暂离");
    addLegend(QColor(244, 67, 54), "维护");
    legendLayout->addStretch();

    mainLayout->addWidget(legendWidget);

    /* 内容区域 */
    stackedWidget = new QStackedWidget();

    mapPage = new QWidget();
    QVBoxLayout* mapLayout = new QVBoxLayout(mapPage);
    mapLayout->setContentsMargins(0, 0, 0, 0);

    mapScrollArea = new QScrollArea();
    mapScrollArea->setWidgetResizable(true);
    mapScrollArea->setStyleSheet("QScrollArea { border: 1px solid #ddd; background-color: #FAFAFA; }");

    mapContainer = new QWidget();
    mapContainer->setStyleSheet("background-color: #FFF8E1;");
    mapScrollArea->setWidget(mapContainer);
    mapLayout->addWidget(mapScrollArea);

    listTable = new QTableWidget();
    listTable->setColumnCount(5);
    listTable->setHorizontalHeaderLabels({"座位号", "状态", "电源", "使用者", "操作"});
    listTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    listTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    listTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listTable->setAlternatingRowColors(true);
    listTable->setStyleSheet(
        "QTableWidget { "
        "   gridline-color: #E0E0E0; "
        "   background-color: white; "
        "   alternate-background-color: #F5F5F5; "
        "   color: #333333; "
        "   font-size: 14px; "
        "   selection-background-color: #BBDEFB; "
        "   selection-color: #333333; "
        "}"
        "QTableWidget::item { "
        "   padding: 10px; "
        "   color: #333333; "
        "   background-color: transparent; "
        "}"
        "QHeaderView::section { "
        "   background-color: #455A64; "
        "   color: white; "
        "   padding: 12px 8px; "
        "   font-weight: bold; "
        "   font-size: 14px; "
        "   border: none; "
        "   border-right: 1px solid #37474F; "
        "}");

    stackedWidget->addWidget(mapPage);
    stackedWidget->addWidget(listTable);

    mainLayout->addWidget(stackedWidget, 1);
}

void SeatViewWidget::loadZone(Zone* zone, const QString& floorName) {
    currentZone = zone;
    currentFloorName = floorName;

    if (!zone) return;

    titleLabel->setText(QString("%1 - %2").arg(floorName).arg(zone->name));
    infoLabel->setText(QString("座位号范围: %1 - %2 | 总座位: %3 | 空闲: %4")
                           .arg(zone->seats.isEmpty() ? "" : zone->seats.first().id)
                           .arg(zone->seats.isEmpty() ? "" : zone->seats.last().id)
                           .arg(zone->getTotalCount())
                           .arg(zone->getAvailableCount()));

    setupMapView();
    setupListView();
}

void SeatViewWidget::setupMapView() {
    /* 彻底清除mapContainer的所有子控件 */
    // 清除座位按钮列表
    seatButtons.clear();

    // 删除mapContainer的所有子控件（包括座位按钮和装饰元素）
    QList<QWidget*> children = mapContainer->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children) {
        child->deleteLater();
    }

    // 删除旧布局
    if (mapContainer->layout()) {
        QLayoutItem* item;
        while ((item = mapContainer->layout()->takeAt(0)) != nullptr) {
            delete item;
        }
        delete mapContainer->layout();
    }

    if (!currentZone) return;

    // 设置 ToolTip 样式
    mapContainer->setStyleSheet(
        "QWidget { background-color: #FFF8E1; }"
        "QToolTip { "
        "   background-color: #37474F; "
        "   color: #FFFFFF; "
        "   border: 1px solid #263238; "
        "   padding: 10px; "
        "   border-radius: 4px; "
        "   font-size: 13px; "
        "   font-family: 'Microsoft YaHei'; "
        "}"
        );

    // 创建网格布局
    QGridLayout* grid = new QGridLayout(mapContainer);
    grid->setSpacing(5);
    grid->setContentsMargins(20, 20, 20, 20);

    int filterValue = filterCombo->currentData().toInt();
    int cols = 10;

    int visibleCount = 0;
    for (int i = 0; i < currentZone->seats.size(); i++) {
        Seat& seat = currentZone->seats[i];

        if (filterValue == Available && seat.status != Available) continue;
        if (filterValue == 100 && !seat.hasPower) continue;

        SeatButton* btn = new SeatButton(&seat, mapContainer);
        connect(btn, &SeatButton::seatClicked, this, &SeatViewWidget::onSeatClicked);

        int row = visibleCount / cols;  // 【修复】使用visibleCount而不是i
        int col = visibleCount % cols;
        grid->addWidget(btn, row, col);
        seatButtons.append(btn);
        visibleCount++;
    }

    /* 添加装饰元素，排除3-7楼的C区 */
    bool showPillars = (currentZone->seats.size() > 50);

    // 检查是否是3-7楼的C区（潇湘校区）- 这些区域不显示柱子
    if (currentZone && currentZone->id == "C") {
        if (!currentZone->seats.isEmpty()) {
            QString seatPrefix = currentZone->seats.first().id;
            // XF3C, XF4C, XF5C, XF6C, XF7C 这些不显示柱子
            if (seatPrefix.startsWith("XF3") ||
                seatPrefix.startsWith("XF4") ||
                seatPrefix.startsWith("XF5") ||
                seatPrefix.startsWith("XF6") ||
                seatPrefix.startsWith("XF7")) {
                showPillars = false;
            }
        }
    }

    if (showPillars) {
        int totalRows = (visibleCount + cols - 1) / cols;  // 计算实际行数

        // 只有当行数足够时才添加柱子
        if (totalRows > 3) {
            QLabel* pillar1 = new QLabel("柱子", mapContainer);
            pillar1->setStyleSheet("background-color: #BDBDBD; padding: 5px; border-radius: 3px;");
            pillar1->setAlignment(Qt::AlignCenter);
            grid->addWidget(pillar1, 3, cols + 1);
        }

        if (totalRows > 8) {
            QLabel* pillar2 = new QLabel("书架", mapContainer);
            pillar2->setStyleSheet("background-color: #BDBDBD; padding: 5px; border-radius: 3px;");
            pillar2->setAlignment(Qt::AlignCenter);
            grid->addWidget(pillar2, 8, cols + 1);
        }
    }
}

void SeatViewWidget::setupListView() {
    listTable->setRowCount(0);
    if (!currentZone) return;

    int filterValue = filterCombo->currentData().toInt();

    for (int i = 0; i < currentZone->seats.size(); i++) {
        Seat& seat = currentZone->seats[i];

        if (filterValue == Available && seat.status != Available) continue;
        if (filterValue == 100 && !seat.hasPower) continue;

        int row = listTable->rowCount();
        listTable->insertRow(row);

        QTableWidgetItem* idItem = new QTableWidgetItem(seat.id);
        idItem->setData(Qt::UserRole, i);
        listTable->setItem(row, 0, idItem);

        QTableWidgetItem* statusItem = new QTableWidgetItem(seat.getStatusName());
        statusItem->setForeground(seat.getStatusColor());
        listTable->setItem(row, 1, statusItem);

        listTable->setItem(row, 2, new QTableWidgetItem(seat.hasPower ? "✓" : ""));

        listTable->setItem(row, 3, new QTableWidgetItem(seat.currentUserId));

        QPushButton* actionBtn = new QPushButton("预约");
        actionBtn->setEnabled(seat.status == Available);
        actionBtn->setMinimumWidth(70);
        actionBtn->setStyleSheet(
            "QPushButton { "
            "   background-color: #4CAF50; "
            "   color: white; "
            "   padding: 6px 16px; "
            "   border: none; "
            "   border-radius: 4px; "
            "   font-size: 13px; "
            "   font-weight: bold; "
            "   min-width: 60px; "
            "}"
            "QPushButton:hover { background-color: #43A047; }"
            "QPushButton:disabled { "
            "   background-color: #BDBDBD; "
            "   color: #757575; "
            "}");

        connect(actionBtn, &QPushButton::clicked, [this, &seat]() {
            onSeatClicked(&seat);
        });

        listTable->setCellWidget(row, 4, actionBtn);
    }
}

void SeatViewWidget::refresh() {
    if (currentZone) {
        loadZone(currentZone, currentFloorName);
    }
}

void SeatViewWidget::toggleViewMode() {
    int currentIndex = stackedWidget->currentIndex();
    if (currentIndex == 0) {
        stackedWidget->setCurrentIndex(1);
        btnToggle->setText("切换为地图模式");
    } else {
        stackedWidget->setCurrentIndex(0);
        btnToggle->setText("切换为列表模式");
    }
}

void SeatViewWidget::onFilterChanged() {
    setupMapView();
    setupListView();
}

void SeatViewWidget::onSeatClicked(Seat* seat) {
    if (!seat) return;

    LibraryManager& mgr = LibraryManager::instance();

    if (!mgr.isLoggedIn()) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    if (seat->status != Available) {
        QString info = QString("座位: %1\n状态: %2").arg(seat->id).arg(seat->getStatusName());
        if (!seat->currentUserId.isEmpty()) {
            info += QString("\n使用者: %1").arg(seat->currentUserId);
        }
        QMessageBox::information(this, "座位信息", info);
        return;
    }

    if (!canReserveNow()) {
        QTime now = QTime::currentTime();
        QTime reserveEndTime(21, 30);
        QTime closeTime(22, 30);

        QString msg;
        if (selectedDate == QDate::currentDate()) {
            if (now > reserveEndTime && now <= closeTime) {
                int secs = now.secsTo(closeTime);
                int hours = secs / 3600;
                int mins = (secs % 3600) / 60;
                int s = secs % 60;
                msg = QString("当前已超过预约时间（21:30）\n\n"
                              "暂停预约，距离闭馆还有 %1:%2:%3\n\n"
                              "💡 提示：18:00后可预约明天的座位")
                          .arg(hours).arg(mins, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
            } else if (now > closeTime) {
                msg = "图书馆已闭馆\n\n💡 提示：18:00后可预约明天的座位";
            } else {
                msg = "当前不在预约时间内\n\n开放预约时间：07:00 - 21:30";
            }
        } else {
            msg = "18:00后才能预约明天的座位";
        }

        QMessageBox::warning(this, "暂停预约", msg);
        return;
    }

    bool isToday = (selectedDate == QDate::currentDate());
    QString dateStr = isToday ? "今天" : "明天";
    QString signInTip = isToday ? "预约后请在15分钟内签到" : "预约后请在明早8:30前签到";

    int ret = QMessageBox::question(this, "确认预约",
                                    QString("确定要预约%1的座位 %2 吗？\n\n%3")
                                        .arg(dateStr).arg(seat->id).arg(signInTip),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        bool isTomorrow = (selectedDate != QDate::currentDate());
        emit seatSelectedForDate(seat, isTomorrow);
    }
}

void SeatViewWidget::showSeatInfo(Seat* seat) {
    if (!seat) return;

    QString info = QString("座位号: %1\n状态: %2\n电源: %3")
                       .arg(seat->id)
                       .arg(seat->getStatusName())
                       .arg(seat->hasPower ? "有" : "无");

    if (!seat->currentUserId.isEmpty()) {
        info += QString("\n当前使用者: %1").arg(seat->currentUserId);
    }

    QMessageBox::information(this, "座位详情", info);
}

void SeatViewWidget::updateDateButtons() {
    if (!btnToday || !btnTomorrow) return;

    QDate today = QDate::currentDate();
    QDate tomorrow = today.addDays(1);

    QString todayText = QString("今天(%1)").arg(today.toString("yyyy-MM-dd"));
    QString tomorrowText = QString("明天(%1)").arg(tomorrow.toString("yyyy-MM-dd"));

    btnToday->setText(todayText);
    btnTomorrow->setText(tomorrowText);

    QString selectedStyle =
        "QPushButton { "
        "   background-color: #26C6DA; "
        "   color: white; "
        "   padding: 10px 25px; "
        "   border: none; "
        "   border-radius: 20px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "}"
        "QPushButton:hover { background-color: #00ACC1; }";

    QString unselectedStyle =
        "QPushButton { "
        "   background-color: white; "
        "   color: #333; "
        "   padding: 10px 25px; "
        "   border: 2px solid #E0E0E0; "
        "   border-radius: 20px; "
        "   font-size: 14px; "
        "}"
        "QPushButton:hover { border-color: #26C6DA; color: #26C6DA; }";

    btnToday->setStyleSheet(btnToday->isChecked() ? selectedStyle : unselectedStyle);
    btnTomorrow->setStyleSheet(btnTomorrow->isChecked() ? selectedStyle : unselectedStyle);

    QTime now = QTime::currentTime();
    QTime canReserveTomorrowTime(18, 0);

    if (now < canReserveTomorrowTime) {
        btnTomorrow->setEnabled(false);
        btnTomorrow->setToolTip("18:00后可预约明天的座位");
        btnTomorrow->setStyleSheet(
            "QPushButton { "
            "   background-color: #F5F5F5; "
            "   color: #9E9E9E; "
            "   padding: 10px 25px; "
            "   border: 2px solid #E0E0E0; "
            "   border-radius: 20px; "
            "   font-size: 14px; "
            "}"
            );
    } else {
        btnTomorrow->setEnabled(true);
        btnTomorrow->setToolTip("");
    }
}

void SeatViewWidget::updateTimeStatus() {
    if (!reserveStatusLabel) return;

    QTime now = QTime::currentTime();
    QTime openTime(7, 0);
    QTime reserveEndTime(21, 30);
    QTime closeTime(22, 30);

    bool isToday = (selectedDate == QDate::currentDate());

    if (isToday) {
        if (now < openTime) {
            int mins = now.secsTo(openTime) / 60;
            int hours = mins / 60;
            mins = mins % 60;
            reserveStatusLabel->setText(QString("⏳ 开放预约(%1:%2)").arg(hours).arg(mins, 2, 10, QChar('0')));
            reserveStatusLabel->setStyleSheet(
                "background-color: #FFF8E1; color: #F57C00; "
                "padding: 8px 15px; border-radius: 5px; font-size: 13px; font-weight: bold;");
        } else if (now <= reserveEndTime) {
            reserveStatusLabel->setText("✅ 可以预约");
            reserveStatusLabel->setStyleSheet(
                "background-color: #E8F5E9; color: #4CAF50; "
                "padding: 8px 15px; border-radius: 5px; font-size: 13px; font-weight: bold;");
        } else if (now <= closeTime) {
            int secs = now.secsTo(closeTime);
            int hours = secs / 3600;
            int mins = (secs % 3600) / 60;
            int s = secs % 60;
            reserveStatusLabel->setText(QString("🚫 暂停预约(%1:%2:%3)")
                                            .arg(hours).arg(mins, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
            reserveStatusLabel->setStyleSheet(
                "background-color: #FFEBEE; color: #E53935; "
                "padding: 8px 15px; border-radius: 5px; font-size: 13px; font-weight: bold;");
        } else {
            reserveStatusLabel->setText("🔒 已闭馆");
            reserveStatusLabel->setStyleSheet(
                "background-color: #ECEFF1; color: #607D8B; "
                "padding: 8px 15px; border-radius: 5px; font-size: 13px; font-weight: bold;");
        }
    } else {
        reserveStatusLabel->setText("✅ 可预约明天座位");
        reserveStatusLabel->setStyleSheet(
            "background-color: #E3F2FD; color: #1976D2; "
            "padding: 8px 15px; border-radius: 5px; font-size: 13px; font-weight: bold;");
    }
}

bool SeatViewWidget::canReserveNow() {
    QTime now = QTime::currentTime();
    QTime openTime(7, 0);
    QTime reserveEndTime(21, 30);
    QTime canReserveTomorrowTime(18, 0);

    bool isToday = (selectedDate == QDate::currentDate());

    if (isToday) {
        return (now >= openTime && now <= reserveEndTime);
    } else {
        return (now >= canReserveTomorrowTime);
    }
}
