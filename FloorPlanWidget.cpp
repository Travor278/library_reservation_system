/*
 * FloorPlanWidget.cpp
 * 楼层平面图视图实现
 */

#include "FloorPlanWidget.h"
#include <QPainter>
#include <QResizeEvent>

/* ZoneButton 实现 */
ZoneButton::ZoneButton(Zone* z, QWidget* parent)
    : QPushButton(parent), zone(z) {
    setCursor(Qt::PointingHandCursor);
    updateTooltip();
    connect(this, &QPushButton::clicked, [this]() {
        emit zoneClicked(zone);
    });
    updateInfo();
}

void ZoneButton::updateTooltip() {
    if (!zone) return;

    int total = zone->getTotalCount();
    int available = zone->getAvailableCount();
    int percent = total > 0 ? (available * 100 / total) : 0;

    // 根据空闲率给出状态描述
    QString statusText;
    if (!zone->isAvailable) {
        statusText = "× 暂停预约";
    } else if (percent > 60) {
        statusText = "★ 空位充足，推荐前往";
    } else if (percent > 30) {
        statusText = "◆ 空位较多";
    } else if (percent > 10) {
        statusText = "▲ 空位较少，建议换区";
    } else {
        statusText = "● 几乎满座";
    }

    QString tip = QString(
                      "【%1】%2\n"
                      "━━━━━━━━━━━━━━\n"
                      "总座位: %3\n"
                      "空  闲: %4\n"
                      "空闲率: %5%\n"
                      "━━━━━━━━━━━━━━\n"
                      "%6\n"
                      "━━━━━━━━━━━━━━\n"
                      "点击查看座位详情")
                      .arg(zone->name)
                      .arg(zone->fullName)
                      .arg(total)
                      .arg(available)
                      .arg(percent)
                      .arg(statusText);

    setToolTip(tip);
}

void ZoneButton::updateInfo() {
    setText(QString("%1\n空座:%2").arg(zone->name).arg(zone->getAvailableCount()));
    updateTooltip();
    update();
}

void ZoneButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 获取区域颜色
    QColor bgColor = zone->getZoneColor();    
    // 绘制背景
    painter.setBrush(bgColor);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(rect().adjusted(1, 1, -1, -1));
    
    // 绘制文字
    painter.setPen(Qt::black);
    QFont font("Microsoft YaHei", 11, QFont::Bold);
    painter.setFont(font);
    
    // 区域名称
    QString zoneName = zone->name;
    QString availableText = QString("空座:%1").arg(zone->getAvailableCount());
    
    QFontMetrics fm(font);
    int textHeight = fm.height();
    
    QRect nameRect = rect().adjusted(5, 5, -5, -rect().height()/2);
    QRect countRect = rect().adjusted(5, rect().height()/2, -5, -5);
    
    painter.drawText(nameRect, Qt::AlignCenter, zoneName);
    
    // 空座数用稍小字体
    QFont smallFont("Microsoft YaHei", 9);
    painter.setFont(smallFont);
    painter.drawText(countRect, Qt::AlignCenter, availableText);
}

void ZoneButton::resizeEvent(QResizeEvent* event) {
    QPushButton::resizeEvent(event);
    update();
}

/* FloorPlanWidget 实现 */
FloorPlanWidget::FloorPlanWidget(QWidget* parent) 
    : QWidget(parent), currentFloor(nullptr) {
    setupUI();
}

void FloorPlanWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 设置背景
    setStyleSheet("background-color: #f5f7fa;");
    
    // 顶部栏
    QHBoxLayout* topBar = new QHBoxLayout();
    
    btnBack = new QPushButton("← 返回");
    btnBack->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 8px 20px; "
        "border: none; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #546E7A; }");
    connect(btnBack, &QPushButton::clicked, this, &FloorPlanWidget::backRequested);
    
    titleLabel = new QLabel("楼层平面图");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    
    topBar->addWidget(btnBack);
    topBar->addStretch();
    topBar->addWidget(titleLabel);
    topBar->addStretch();
    
    mainLayout->addLayout(topBar);

    // 信息栏
    infoLabel = new QLabel();
    infoLabel->setStyleSheet(
        "QLabel { background-color: #fff3e0; padding: 12px 15px; border-radius: 8px; "
        "color: #e65100; font-size: 14px; font-weight: 500; }");
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);
    
    // 图例
    legendWidget = new QWidget();
    legendWidget->setStyleSheet("background-color: white; border-radius: 5px; padding: 5px;");
    QHBoxLayout* legendLayout = new QHBoxLayout(legendWidget);
    legendLayout->setContentsMargins(10, 8, 10, 8);
    legendLayout->setSpacing(8);
    
    QLabel* legendTitle = new QLabel("颜色说明:");
    legendTitle->setStyleSheet("font-weight: bold; color: #333;");
    legendLayout->addWidget(legendTitle);

    // 添加图例项的lambda
    auto addLegend = [legendLayout](const QColor& color, const QString& text) {
        QLabel* colorLabel = new QLabel();
        colorLabel->setFixedSize(22, 16);
        colorLabel->setStyleSheet(QString("background-color: %1; border: 1px solid #666; border-radius: 2px;").arg(color.name()));
        legendLayout->addWidget(colorLabel);
        QLabel* textLabel = new QLabel(text);
        textLabel->setStyleSheet("color: #333; font-size: 12px;");
        legendLayout->addWidget(textLabel);
        legendLayout->addSpacing(8);
    };
    
    addLegend(QColor(139, 195, 74), "空闲充足(>60%)");
    addLegend(QColor(255, 235, 59), "较多空位(30-60%)");
    addLegend(QColor(255, 152, 0), "空位较少(10-30%)");
    addLegend(QColor(244, 67, 54), "几乎满座(<10%)");
    legendLayout->addStretch();
    
    mainLayout->addWidget(legendWidget);
    
    // 平面图容器
    planContainer = new QWidget();
    planContainer->setMinimumSize(600, 450);
    planContainer->setStyleSheet("background-color: #FAFAFA; border: 2px solid #E0E0E0; border-radius: 5px;");
    
    mainLayout->addWidget(planContainer, 1);
    
    // 底部提示
    QLabel* tipLabel = new QLabel("点击区域查看座位详情");
    tipLabel->setStyleSheet("color: #757575; font-size: 12px;");
    tipLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(tipLabel);
}

void FloorPlanWidget::loadFloor(Floor* floor, const QString& campusName) {
    currentFloor = floor;
    currentCampusName = campusName;
    
    if (!floor) return;
    
    titleLabel->setText(QString("%1楼平面图").arg(floor->level));
    infoLabel->setText(QString("%1 | %2楼 | 总座位: %3 | 空闲: %4")
                       .arg(campusName)
                       .arg(floor->level)
                       .arg(floor->getTotalCount())
                       .arg(floor->getAvailableCount()));
    
    createZoneButtons();
}

void FloorPlanWidget::refresh() {
    if (currentFloor) {
        // 更新信息
        infoLabel->setText(QString("%1 | %2楼 | 总座位: %3 | 空闲: %4")
                           .arg(currentCampusName)
                           .arg(currentFloor->level)
                           .arg(currentFloor->getTotalCount())
                           .arg(currentFloor->getAvailableCount()));
        
        // 更新区域按钮
        for (auto btn : zoneButtons) {
            btn->updateInfo();
        }
    }
}

void FloorPlanWidget::clearZones() {
    for (auto btn : zoneButtons) {
        btn->deleteLater();
    }
    zoneButtons.clear();
}

void FloorPlanWidget::createZoneButtons() {
    clearZones();

    if (!currentFloor) return;

    planContainer->setStyleSheet(
        "QWidget { background-color: #FAFAFA; }"
        "QToolTip { "
        "   background-color: #263238; "
        "   color: #FFFFFF; "
        "   border: 1px solid #455A64; "
        "   padding: 12px; "
        "   border-radius: 6px; "
        "   font-size: 14px; "
        "   font-family: 'Microsoft YaHei'; "
        "}"
        );

    int containerW = planContainer->width();
    int containerH = planContainer->height();

    // 确保合理的最小尺寸
    if (containerW < 100) containerW = 600;
    if (containerH < 100) containerH = 450;

    // 根据Zone的相对坐标创建按钮
    for (auto& zone : currentFloor->zones) {
        ZoneButton* btn = new ZoneButton(&zone, planContainer);
        connect(btn, &ZoneButton::zoneClicked, this, &FloorPlanWidget::onZoneClicked);

        // 根据相对坐标计算实际位置
        int x = static_cast<int>(zone.x * containerW);
        int y = static_cast<int>(zone.y * containerH);
        int w = static_cast<int>(zone.w * containerW);
        int h = static_cast<int>(zone.h * containerH);

        // 确保最小尺寸
        w = qMax(w, 80);
        h = qMax(h, 60);

        btn->setGeometry(x, y, w, h);
        btn->show();

        zoneButtons.append(btn);
    }

    // 装饰元素：楼梯、电梯、卫生间、入口（放在边缘避免遮挡区域）
    // 楼梯 - 放在右下角边缘
    QLabel* stairsLabel = new QLabel("楼梯", planContainer);
    stairsLabel->setStyleSheet(
        "background-color: #78909C; color: white; padding: 5px 10px; "
        "font-size: 11px; border-radius: 3px;");
    stairsLabel->adjustSize();
    stairsLabel->move(containerW - stairsLabel->width() - 15, containerH - 40);
    stairsLabel->show();

    // 电梯 - 放在右侧中间边缘
    QLabel* elevatorLabel = new QLabel("电梯", planContainer);
    elevatorLabel->setStyleSheet(
        "background-color: #607D8B; color: white; padding: 5px 10px; "
        "font-size: 11px; border-radius: 3px;");
    elevatorLabel->adjustSize();
    elevatorLabel->move(containerW - elevatorLabel->width() - 15, containerH / 2 - 15);
    elevatorLabel->show();

    // 卫生间 - 左下角
    QLabel* wcLabel = new QLabel("卫生间", planContainer);
    wcLabel->setStyleSheet(
        "background-color: #546E7A; color: white; padding: 5px 10px; "
        "font-size: 11px; border-radius: 3px;");
    wcLabel->adjustSize();
    wcLabel->move(10, containerH - 40);
    wcLabel->show();

    // 卫生间2 - 右上角
    QLabel* wcLabel2 = new QLabel("卫生间", planContainer);
    wcLabel2->setStyleSheet(
        "background-color: #546E7A; color: white; padding: 5px 10px; "
        "font-size: 11px; border-radius: 3px;");
    wcLabel2->adjustSize();
    wcLabel2->move(containerW - wcLabel2->width() - 15, 10);
    wcLabel2->show();

    // 入口 - 底部中间
    QLabel* entranceLabel = new QLabel("入口 ↓", planContainer);
    entranceLabel->setStyleSheet(
        "background-color: #4CAF50; color: white; padding: 5px 12px; "
        "font-size: 11px; font-weight: bold; border-radius: 3px;");
    entranceLabel->adjustSize();
    entranceLabel->move(containerW / 2 - entranceLabel->width() / 2, containerH - 40);
    entranceLabel->show();
}

void FloorPlanWidget::onZoneClicked(Zone* zone) {
    if (zone && currentFloor) {
        QString floorName = QString("%1楼%2").arg(currentFloor->level).arg(zone->name);
        emit zoneSelected(zone, floorName);
    }
}
