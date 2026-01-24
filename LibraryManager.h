#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include "DataModel.h"
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class LibraryManager : public QObject {
    Q_OBJECT
    
public:
    static LibraryManager& instance() {
        static LibraryManager inst;
        return inst;
    }
    
    // 数据访问
    QList<Campus>& getCampuses() { return campuses; }
    Campus* getCampus(const QString& name);
    Campus* getCampusById(const QString& id);
    QList<Announcement>& getAnnouncements() { return announcements; }
    
    // 用户管理
    bool login(const QString& userId, const QString& password);
    void logout();
    bool registerUser(const QString& userId, const QString& password, const QString& name, UserType type);
    User* getCurrentUser() { return currentUser; }
    User* getUser(const QString& userId);
    bool isLoggedIn() const { return currentUser != nullptr; }
    
    // 座位操作
    bool reserveSeat(const QString& campusId, int floor, const QString& zoneId, const QString& seatId);
    bool reserveTomorrow(const QString& campusId, int floor, const QString& zoneId, const QString& seatId);
    bool signIn(const QString& seatId);
    bool signOut(const QString& seatId);
    bool setTemporaryLeave(const QString& seatId);
    bool returnFromLeave(const QString& seatId);
    bool cancelReservation(const QString& seatId);
    bool reserveSeatForDate(const QString& campusId, int floor, const QString& zoneId,
                            const QString& seatId, bool isTomorrow);
    
    // 查询
    Seat* findSeat(const QString& seatId);
    Seat* getUserCurrentSeat();
    QList<UsageRecord> getUserHistory(const QString& userId);
    QList<UsageRecord> getAllHistory();
    QList<Seat*> getRecommendedSeats(const QString& campusId, int maxCount = 5);
    QList<Seat*> getUserFavoriteSeats();
    QList<Announcement> getAnnouncements() const { return announcements; }
    
    // 时间检查
    bool isOpenTime() const;
    bool canReserve() const;
    bool canReserveTomorrow() const;
    QString getTimeStatus() const;
    int getTodayVisitorCount(const QString& campusId);
    
    // 数据持久化
    bool saveData();
    bool loadData();
    
signals:
    void dataChanged();
    void userLoggedIn(User* user);
    void userLoggedOut();
    void reservationSuccess(const QString& seatId, int visitorNumber);
    void signInSuccess(const QString& seatId, int visitorNumber);
    void operationFailed(const QString& message);
    void timeStatusChanged();
    
private slots:
    void checkTimeouts();
    void onMinuteChanged();
    
private:
    LibraryManager();
    ~LibraryManager();
    LibraryManager(const LibraryManager&) = delete;
    LibraryManager& operator=(const LibraryManager&) = delete;
    
    void initData();
    void initXiaoxiangCampus();
    void initOtherCampuses();
    void initAnnouncements();
    void initFloor1Seats(Floor& floor);
    void initFloor2Seats(Floor& floor);
    void initFloor3Seats(Floor& floor);
    void initFloor4Seats(Floor& floor);
    void initFloor5Seats(Floor& floor);
    void initFloor6Seats(Floor& floor);
    void initFloor7Seats(Floor& floor);
    
    void createTableSeats(Zone& zone, const QString& prefix, int startNum, 
                          int seatsPerTable, int numTables, int tablesPerRow);
    
    void addRecord(const QString& seatId, const QString& action);
    void addViolation(User* user);
    QString generateRecordId();
    
    QList<Campus> campuses;
    QMap<QString, User> users;
    QList<UsageRecord> history;
    QList<Announcement> announcements;
    User* currentUser;
    QTimer* timeoutTimer;
    QTimer* minuteTimer;
    int recordCounter;
};

#endif // LIBRARYMANAGER_H
