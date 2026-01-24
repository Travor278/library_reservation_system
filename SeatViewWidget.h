#ifndef SEATVIEWWIDGET_H
#define SEATVIEWWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QComboBox>
#include "DataModel.h"

/* 单个座位按钮 */
class SeatButton : public QPushButton {
    Q_OBJECT
public:
    explicit SeatButton(Seat* seat, QWidget* parent = nullptr);
    void updateStatus();
    void updateTooltip();
    Seat* getSeat() const { return seat; }

signals:
    void seatClicked(Seat* seat);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    Seat* seat;
    bool isHovered;
};

/* 座位视图组件 */
class SeatViewWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit SeatViewWidget(QWidget* parent = nullptr);
    void loadZone(Zone* zone, const QString& floorName);
    void refresh();
    
signals:
    void backRequested();
    void seatSelected(Seat* seat);
    void seatSelectedForDate(Seat* seat, bool isTomorrow);
    
private slots:
    void onSeatClicked(Seat* seat);
    void toggleViewMode();
    void onFilterChanged();
    
private:
    void setupUI();
    void setupMapView();
    void setupListView();
    void showSeatInfo(Seat* seat);
    void updateDateButtons();
    void updateTimeStatus();
    bool canReserveNow();
    
    Zone* currentZone;
    QString currentFloorName;
    
    // UI组件
    QLabel* titleLabel;
    QLabel* infoLabel;
    QPushButton* btnBack;
    QPushButton* btnToggle;
    QComboBox* filterCombo;
    
    QStackedWidget* stackedWidget;
    QWidget* mapPage;
    QScrollArea* mapScrollArea;
    QWidget* mapContainer;
    QTableWidget* listTable;
    
    QList<SeatButton*> seatButtons;
    
    // 图例
    QWidget* legendWidget;

    // 日期选择相关
    QPushButton* btnToday;
    QPushButton* btnTomorrow;
    QLabel* timeRangeLabel;
    QLabel* reserveStatusLabel;
    QDate selectedDate;
};

#endif // SEATVIEWWIDGET_H
