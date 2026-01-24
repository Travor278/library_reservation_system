#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include "DataModel.h"

/* 统计分析组件 */
class StatisticsWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit StatisticsWidget(QWidget* parent = nullptr);
    void refresh();
    
signals:
    void backRequested();
    
private slots:
    void onFilterChanged();
    void exportData();
    
private:
    void setupUI();
    void updateStatistics();
    void updateHotSeats();
    void updateUsageChart();
    
    // 筛选控件
    QComboBox* campusCombo;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QPushButton* btnRefresh;
    QPushButton* btnExport;
    QPushButton* btnBack;
    
    // 统计信息显示
    QLabel* totalReservationsLabel;
    QLabel* totalSignInsLabel;
    QLabel* avgUsageTimeLabel;
    QLabel* peakHourLabel;
    
    // 热门座位表格
    QTableWidget* hotSeatsTable;
    
    // 各楼层使用率
    QWidget* usageChartWidget;
};

#endif // STATISTICSWIDGET_H
