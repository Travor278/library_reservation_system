#include "StatisticsWidget.h"
#include "LibraryManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPainter>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QMap>
#include <QPainterPath>
#include <QLinearGradient>

/* 使用率图表组件 - 美化版 */
class UsageChartWidget : public QWidget {
public:
    QMap<QString, double> data;

    UsageChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(280);
        setMinimumWidth(400);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 背景
        painter.fillRect(rect(), QColor(250, 250, 252));

        if (data.isEmpty()) {
            painter.setPen(QColor(150, 150, 150));
            painter.setFont(QFont("Microsoft YaHei", 14));
            painter.drawText(rect(), Qt::AlignCenter, "暂无数据");
            return;
        }

        int margin = 60;
        int rightMargin = 30;
        int topMargin = 40;
        int bottomMargin = 50;

        int chartWidth = width() - margin - rightMargin;
        int chartHeight = height() - topMargin - bottomMargin;
        int bottomY = height() - bottomMargin;

        // 计算柱子宽度
        int barCount = data.size();
        int totalSpacing = barCount > 1 ? (barCount - 1) * 15 : 0;
        int barWidth = qMin(60, (chartWidth - totalSpacing) / barCount);
        int actualWidth = barCount * barWidth + totalSpacing;
        int startX = margin + (chartWidth - actualWidth) / 2;

        // 绘制网格线和Y轴刻度
        painter.setPen(QPen(QColor(230, 230, 230), 1));
        painter.setFont(QFont("Microsoft YaHei", 9));

        for (int i = 0; i <= 5; i++) {
            int y = bottomY - (i * chartHeight / 5);

            // 网格线
            painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));
            painter.drawLine(margin, y, width() - rightMargin, y);

            // Y轴刻度文字
            painter.setPen(QColor(120, 120, 120));
            QString label = QString("%1%").arg(i * 20);
            painter.drawText(5, y - 8, margin - 10, 16, Qt::AlignRight | Qt::AlignVCenter, label);
        }

        // 绘制坐标轴
        painter.setPen(QPen(QColor(200, 200, 200), 2));
        painter.drawLine(margin, topMargin, margin, bottomY);
        painter.drawLine(margin, bottomY, width() - rightMargin, bottomY);

        // 绘制柱状图
        int x = startX;
        QStringList keys = data.keys();

        // 渐变色数组
        QList<QPair<QColor, QColor>> gradients = {
            {QColor(66, 133, 244), QColor(25, 118, 210)},    // 蓝色
            {QColor(52, 168, 83), QColor(27, 94, 32)},       // 绿色
            {QColor(251, 188, 5), QColor(245, 127, 23)},     // 黄色
            {QColor(234, 67, 53), QColor(183, 28, 28)},      // 红色
            {QColor(155, 89, 182), QColor(106, 27, 154)},    // 紫色
            {QColor(0, 188, 212), QColor(0, 131, 143)},      // 青色
            {QColor(255, 152, 0), QColor(230, 81, 0)},       // 橙色
        };

        for (int i = 0; i < keys.size(); i++) {
            const QString& key = keys[i];
            double value = data[key];

            // 最小高度保证可见（即使是0%也显示一个小条）
            int barHeight = qMax(4, static_cast<int>(value * chartHeight / 100));

            // 选择渐变色
            auto& grad = gradients[i % gradients.size()];
            QColor topColor = grad.first;
            QColor bottomColor = grad.second;

            // 根据使用率调整颜色
            if (value > 80) {
                topColor = QColor(244, 67, 54);
                bottomColor = QColor(183, 28, 28);
            } else if (value > 60) {
                topColor = QColor(255, 152, 0);
                bottomColor = QColor(230, 81, 0);
            } else if (value > 40) {
                topColor = QColor(255, 193, 7);
                bottomColor = QColor(245, 127, 23);
            }

            // 绘制柱子（圆角矩形+渐变）
            QRect barRect(x, bottomY - barHeight, barWidth, barHeight);

            QLinearGradient gradient(barRect.topLeft(), barRect.bottomLeft());
            gradient.setColorAt(0, topColor);
            gradient.setColorAt(1, bottomColor);

            QPainterPath path;
            path.addRoundedRect(barRect, 4, 4);
            painter.fillPath(path, gradient);

            // 柱子边框
            painter.setPen(QPen(bottomColor.darker(110), 1));
            painter.drawPath(path);

            // 数值标签（柱子上方）
            painter.setPen(QColor(80, 80, 80));
            painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
            QString valueText = QString::number(value, 'f', 1) + "%";
            painter.drawText(x - 5, bottomY - barHeight - 20, barWidth + 10, 18,
                             Qt::AlignCenter, valueText);

            // X轴标签
            painter.setPen(QColor(100, 100, 100));
            painter.setFont(QFont("Microsoft YaHei", 10));
            painter.drawText(x - 10, bottomY + 8, barWidth + 20, 25,
                             Qt::AlignCenter, key);

            x += barWidth + 15;
        }

        // 图表标题
        painter.setPen(QColor(60, 60, 60));
        painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        painter.drawText(margin, 5, chartWidth, 30, Qt::AlignCenter, "实时座位使用率");
    }
};

/* StatisticsWidget 实现 */
StatisticsWidget::StatisticsWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void StatisticsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 顶部栏
    QHBoxLayout* topBar = new QHBoxLayout();

    btnBack = new QPushButton("← 返回");
    btnBack->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; padding: 8px 20px; "
        "border: none; border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #546E7A; }");
    connect(btnBack, &QPushButton::clicked, this, &StatisticsWidget::backRequested);

    QLabel* titleLabel = new QLabel("📊 数据统计分析");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1565C0;");

    topBar->addWidget(btnBack);
    topBar->addWidget(titleLabel);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    // 筛选栏
    QHBoxLayout* filterBar = new QHBoxLayout();
    filterBar->setSpacing(15);

    QLabel* campusLabel = new QLabel("校区:");
    campusLabel->setStyleSheet("font-size: 13px; color: #333;");
    filterBar->addWidget(campusLabel);

    campusCombo = new QComboBox();
    campusCombo->addItem("全部校区", "all");
    LibraryManager& mgr = LibraryManager::instance();
    for (auto& campus : mgr.getCampuses()) {
        campusCombo->addItem(campus.name, campus.id);
    }
    campusCombo->setMinimumWidth(150);
    campusCombo->setStyleSheet(
        "QComboBox { padding: 6px 12px; border: 1px solid #ccc; border-radius: 4px; "
        "background: white; font-size: 13px; color: #333; }"
        "QComboBox:hover { border-color: #2196F3; }"
        "QComboBox QAbstractItemView { background: white; color: #333; selection-background-color: #2196F3; }");
    connect(campusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatisticsWidget::onFilterChanged);
    filterBar->addWidget(campusCombo);

    filterBar->addSpacing(20);

    QLabel* dateLabel = new QLabel("日期范围:");
    dateLabel->setStyleSheet("font-size: 13px; color: #333;");
    filterBar->addWidget(dateLabel);

    startDateEdit = new QDateEdit(QDate::currentDate().addDays(-7));
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setStyleSheet(
        "QDateEdit { padding: 6px; border: 1px solid #ccc; border-radius: 4px; }");
    filterBar->addWidget(startDateEdit);

    QLabel* toLabel = new QLabel("至");
    toLabel->setStyleSheet("font-size: 13px; color: #666;");
    filterBar->addWidget(toLabel);

    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setStyleSheet(
        "QDateEdit { padding: 6px; border: 1px solid #ccc; border-radius: 4px; }");
    filterBar->addWidget(endDateEdit);

    filterBar->addSpacing(10);

    btnRefresh = new QPushButton("🔄 刷新");
    btnRefresh->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; padding: 8px 20px; "
        "border: none; border-radius: 4px; font-size: 13px; }"
        "QPushButton:hover { background-color: #1976D2; }");
    connect(btnRefresh, &QPushButton::clicked, this, &StatisticsWidget::refresh);
    filterBar->addWidget(btnRefresh);

    btnExport = new QPushButton("📥 导出CSV");
    btnExport->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; "
        "border: none; border-radius: 4px; font-size: 13px; }"
        "QPushButton:hover { background-color: #388E3C; }");
    connect(btnExport, &QPushButton::clicked, this, &StatisticsWidget::exportData);
    filterBar->addWidget(btnExport);

    filterBar->addStretch();
    mainLayout->addLayout(filterBar);

    // 概览统计卡片
    QGroupBox* overviewGroup = new QGroupBox("📈 概览统计");
    overviewGroup->setStyleSheet(
        "QGroupBox { font-size: 14px; font-weight: bold; border: 2px solid #E3F2FD; "
        "border-radius: 8px; margin-top: 12px; padding: 15px; background: #FAFAFA; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 8px; "
        "color: #1565C0; }");

    QHBoxLayout* overviewLayout = new QHBoxLayout(overviewGroup);
    overviewLayout->setSpacing(20);

    // 统计卡片创建函数
    auto createStatCard = [](const QString& title, const QString& value,
                             const QString& color, const QString& icon) -> QWidget* {
        QWidget* card = new QWidget();
        card->setStyleSheet(QString(
                                "QWidget { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                                "stop:0 white, stop:1 #F5F5F5); border-radius: 10px; "
                                "border: 1px solid #E0E0E0; border-left: 5px solid %1; }").arg(color));
        card->setMinimumSize(160, 90);
        card->setMaximumHeight(100);

        QVBoxLayout* layout = new QVBoxLayout(card);
        layout->setContentsMargins(15, 10, 15, 10);

        QLabel* iconLabel = new QLabel(icon + " " + title);
        iconLabel->setStyleSheet("color: #757575; font-size: 12px; font-weight: normal;");

        QLabel* valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString(
                                      "color: %1; font-size: 28px; font-weight: bold;").arg(color));
        valueLabel->setObjectName("valueLabel");

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addStretch();
        return card;
    };

    QWidget* card1 = createStatCard("总预约次数", "0", "#2196F3", "📋");
    totalReservationsLabel = card1->findChild<QLabel*>("valueLabel");
    overviewLayout->addWidget(card1);

    QWidget* card2 = createStatCard("签到次数", "0", "#4CAF50", "✅");
    totalSignInsLabel = card2->findChild<QLabel*>("valueLabel");
    overviewLayout->addWidget(card2);

    QWidget* card3 = createStatCard("平均使用时长", "0h", "#FF9800", "⏱️");
    avgUsageTimeLabel = card3->findChild<QLabel*>("valueLabel");
    overviewLayout->addWidget(card3);

    QWidget* card4 = createStatCard("高峰时段", "--", "#9C27B0", "🔥");
    peakHourLabel = card4->findChild<QLabel*>("valueLabel");
    overviewLayout->addWidget(card4);

    overviewLayout->addStretch();
    mainLayout->addWidget(overviewGroup);

    // 下半部分：图表和热门座位
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(15);

    // 使用率柱状图
    QGroupBox* chartGroup = new QGroupBox("📊 各楼层使用率");
    chartGroup->setStyleSheet(
        "QGroupBox { font-size: 14px; font-weight: bold; border: 2px solid #E8F5E9; "
        "border-radius: 8px; margin-top: 12px; padding: 15px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 8px; "
        "color: #2E7D32; }");
    QVBoxLayout* chartLayout = new QVBoxLayout(chartGroup);

    usageChartWidget = new UsageChartWidget();
    chartLayout->addWidget(usageChartWidget);

    bottomLayout->addWidget(chartGroup, 3);

    // 热门座位表格
    QGroupBox* hotSeatsGroup = new QGroupBox("🔥 热门座位 TOP10");
    hotSeatsGroup->setStyleSheet(
        "QGroupBox { font-size: 14px; font-weight: bold; border: 2px solid #FFF3E0; "
        "border-radius: 8px; margin-top: 12px; padding: 15px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 8px; "
        "color: #E65100; }");
    QVBoxLayout* hotSeatsLayout = new QVBoxLayout(hotSeatsGroup);

    hotSeatsTable = new QTableWidget();
    hotSeatsTable->setColumnCount(4);
    hotSeatsTable->setHorizontalHeaderLabels({"排名", "座位号", "使用次数", "所在区域"});
    hotSeatsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hotSeatsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    hotSeatsTable->setAlternatingRowColors(true);
    hotSeatsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    hotSeatsTable->verticalHeader()->setVisible(false);
    hotSeatsTable->setStyleSheet(
        "QTableWidget { gridline-color: #E0E0E0; border: none; font-size: 13px; }"
        "QTableWidget::item { padding: 8px; }"
        "QTableWidget::item:selected { background-color: #E3F2FD; color: #1565C0; }"
        "QHeaderView::section { background-color: #ECEFF1; padding: 10px; "
        "font-weight: bold; border: none; border-bottom: 2px solid #BDBDBD; }");

    hotSeatsLayout->addWidget(hotSeatsTable);

    bottomLayout->addWidget(hotSeatsGroup, 2);

    mainLayout->addLayout(bottomLayout, 1);
}

void StatisticsWidget::refresh() {
    updateStatistics();
    updateHotSeats();
    updateUsageChart();
}

void StatisticsWidget::onFilterChanged() {
    refresh();
}

void StatisticsWidget::updateStatistics() {
    LibraryManager& mgr = LibraryManager::instance();
    QList<UsageRecord> history = mgr.getAllHistory();

    int totalReservations = 0;
    int totalSignIns = 0;
    int totalHours = 0;
    QMap<int, int> hourCount;

    QDate startDate = startDateEdit->date();
    QDate endDate = endDateEdit->date();

    for (const auto& rec : history) {
        QDate recDate = rec.startTime.date();
        if (recDate < startDate || recDate > endDate) continue;

        if (rec.action == "预约") {
            totalReservations++;
            hourCount[rec.startTime.time().hour()]++;
        }
        if (rec.action == "签到") {
            totalSignIns++;
            totalHours += 2; // 假设平均每次2小时
        }
    }

    totalReservationsLabel->setText(QString::number(totalReservations));
    totalSignInsLabel->setText(QString::number(totalSignIns));

    double avgHours = totalSignIns > 0 ? (double)totalHours / totalSignIns : 0;
    avgUsageTimeLabel->setText(QString("%1h").arg(avgHours, 0, 'f', 1));

    // 高峰时段
    int peakHour = -1;
    int maxCount = 0;
    for (auto it = hourCount.begin(); it != hourCount.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            peakHour = it.key();
        }
    }

    if (peakHour >= 0) {
        peakHourLabel->setText(QString("%1:00").arg(peakHour, 2, 10, QChar('0')));
    } else {
        peakHourLabel->setText("--");
    }
}

void StatisticsWidget::updateHotSeats() {
    hotSeatsTable->setRowCount(0);

    LibraryManager& mgr = LibraryManager::instance();
    QList<UsageRecord> history = mgr.getAllHistory();

    QMap<QString, int> seatCount;
    for (const auto& rec : history) {
        if (rec.action == "预约" || rec.action == "签到") {
            seatCount[rec.seatId]++;
        }
    }

    // 排序
    QList<QPair<QString, int>> sortedSeats;
    for (auto it = seatCount.begin(); it != seatCount.end(); ++it) {
        sortedSeats.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedSeats.begin(), sortedSeats.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });

    int count = qMin(10, sortedSeats.size());
    hotSeatsTable->setRowCount(count);

    // 排名对应的颜色
    // 前三名背景色（柔和版）
    QList<QColor> rankBgColors = {
        QColor(255, 248, 220),  // 第1名：淡金色
        QColor(240, 240, 245),  // 第2名：淡银色
        QColor(255, 240, 230)   // 第3名：淡铜色
    };
    QList<QColor> rankTextColors = {
        QColor(184, 134, 11),   // 第1名：深金色文字
        QColor(105, 105, 105),  // 第2名：深灰色文字
        QColor(160, 82, 45)     // 第3名：棕色文字
    };

    for (int i = 0; i < count; i++) {
        // 排名
        QTableWidgetItem* rankItem = new QTableWidgetItem(QString::number(i + 1));
        rankItem->setTextAlignment(Qt::AlignCenter);
        if (i < 3) {
            rankItem->setBackground(rankBgColors[i]);
            rankItem->setForeground(rankTextColors[i]);
            rankItem->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
        }
        hotSeatsTable->setItem(i, 0, rankItem);

        // 座位号
        QTableWidgetItem* seatItem = new QTableWidgetItem(sortedSeats[i].first);
        seatItem->setTextAlignment(Qt::AlignCenter);
        hotSeatsTable->setItem(i, 1, seatItem);

        // 使用次数
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(sortedSeats[i].second));
        countItem->setTextAlignment(Qt::AlignCenter);
        countItem->setForeground(QColor("#1565C0"));
        hotSeatsTable->setItem(i, 2, countItem);

        // 所在区域
        QString zoneInfo = sortedSeats[i].first.mid(0, 3) + "区";
        QTableWidgetItem* zoneItem = new QTableWidgetItem(zoneInfo);
        zoneItem->setTextAlignment(Qt::AlignCenter);
        hotSeatsTable->setItem(i, 3, zoneItem);
    }
}

void StatisticsWidget::updateUsageChart() {
    UsageChartWidget* chart = static_cast<UsageChartWidget*>(usageChartWidget);
    chart->data.clear();

    LibraryManager& mgr = LibraryManager::instance();
    QString campusId = campusCombo->currentData().toString();

    for (auto& campus : mgr.getCampuses()) {
        if (campusId != "all" && campus.id != campusId) continue;

        for (auto& floor : campus.floors) {
            int total = floor.getTotalCount();
            int used = total - floor.getAvailableCount();
            double rate = total > 0 ? (double)used / total * 100 : 0;

            QString key = QString("%1F").arg(floor.level);
            if (campusId == "all") {
                key = QString("%1-%2F").arg(campus.name.left(2)).arg(floor.level);
            }
            chart->data[key] = rate;
        }

        if (campusId != "all") break;
    }

    chart->update();
}

void StatisticsWidget::exportData() {
    QString fileName = QFileDialog::getSaveFileName(this, "导出数据",
                                                    "library_statistics.csv",
                                                    "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // BOM for Excel
    out << "\xEF\xBB\xBF";
    out << "时间,座位号,用户ID,用户名,操作\n";

    LibraryManager& mgr = LibraryManager::instance();
    QList<UsageRecord> history = mgr.getAllHistory();

    for (const auto& rec : history) {
        out << rec.startTime.toString("yyyy-MM-dd HH:mm:ss") << ","
            << rec.seatId << ","
            << rec.userId << ","
            << rec.userName << ","
            << rec.action << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出成功",
                             QString("数据已导出到:\n%1\n\n共 %2 条记录")
                                 .arg(fileName).arg(history.size()));
}
