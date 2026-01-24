#ifndef FLOORPLANWIDGET_H
#define FLOORPLANWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "DataModel.h"

/* 区域按钮 */
class ZoneButton : public QPushButton {
    Q_OBJECT
public:
    explicit ZoneButton(Zone* zone, QWidget* parent = nullptr);
    void updateInfo();
    void updateTooltip();  // 添加这行
    Zone* getZone() const { return zone; }

signals:
    void zoneClicked(Zone* zone);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    Zone* zone;
};

/* 楼层平面图组件 */
class FloorPlanWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit FloorPlanWidget(QWidget* parent = nullptr);
    void loadFloor(Floor* floor, const QString& campusName);
    void refresh();
    
signals:
    void zoneSelected(Zone* zone, const QString& floorName);
    void backRequested();
    
private slots:
    void onZoneClicked(Zone* zone);
    
private:
    void setupUI();
    void clearZones();
    void createZoneButtons();
    
    Floor* currentFloor;
    QString currentCampusName;
    
    QLabel* titleLabel;
    QLabel* infoLabel;
    QPushButton* btnBack;
    QWidget* planContainer;     // 平面图容器
    QList<ZoneButton*> zoneButtons;
    
    // 图例
    QWidget* legendWidget;
};

#endif // FLOORPLANWIDGET_H
