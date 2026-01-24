#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QColor>
#include <QMap>
#include <QTime>

/* 系统常量配置 */
namespace Config {
    // 开放时间
    const QTime OPEN_TIME(7, 0);           // 开馆时间 7:00
    const QTime RESERVATION_END(21, 30);   // 预约结束时间 21:30
    const QTime CLOSE_TIME(22, 30);        // 闭馆时间 22:30
    
    // 规则配置
    const int SIGN_IN_TIMEOUT = 15;        // 签到超时时间（分钟）
    const int TEMP_LEAVE_TIMEOUT = 30;     // 暂离超时时间（分钟）
    const int MAX_CANCEL_PER_WEEK = 1;     // 每周最多取消预约次数
    const int MAX_VIOLATION_PER_MONTH = 3; // 每月最多违约次数
    const int BAN_DAYS = 3;                // 违约封禁天数
    const int CREDIT_PER_HOUR = 1;         // 每小时增加信用分
    const int CREDIT_VIOLATION_DEDUCT = 10;// 违约扣除信用分
    const int MIN_CREDIT_TO_RESERVE = 70;  // 最低预约信用分
    const int MAX_CREDIT = 100;            // 最大信用分
    const int INITIAL_CREDIT = 100;        // 初始信用分
}

/* 座位状态枚举 */
enum SeatStatus {
    Available = 0,      // 绿色：可用
    Occupied,           // 灰色：占用中
    Reserved,           // 蓝色：已预约未签到
    TemporaryLeave,     // 黄色：暂离
    Broken              // 红色：故障/维护
};

/* 用户类型枚举 */
enum UserType {
    Student = 0,
    Teacher,
    Graduate,
    Admin
};

/* 公告类 */
struct Announcement {
    QString id;
    QString title;
    QString content;
    QDateTime createTime;
    QDateTime expireTime;
    bool isImportant;
};

/* 操作结构类 */
struct UsageRecord {
    QString recordId;
    QString seatId;
    QString seatName;
    QString userId;
    QString userName;
    QDateTime startTime;
    QDateTime endTime;
    QString action;
    QString campusId;
    QString campusName;
    int floorLevel;
    QString zoneName;
};

/* 用户类 */
class User {
public:
    QString userId;
    QString password;
    QString name;
    UserType type;
    int creditScore;
    int monthlyViolations;      // 本月违约次数
    int weeklyCancellations;    // 本周取消次数
    QDateTime banEndTime;       // 封禁结束时间
    QDateTime lastCancelTime;   // 上次取消时间
    QDateTime lastViolationTime;// 上次违约时间
    QStringList favoriteSeatIds;// 常用座位
    int totalVisits;            // 总到馆次数
    int totalHours;             // 总使用小时数
    
    User() : type(Student), creditScore(Config::INITIAL_CREDIT), 
             monthlyViolations(0), weeklyCancellations(0), totalVisits(0), totalHours(0) {}
    
    User(const QString& id, const QString& pwd, const QString& n, UserType t = Student)
        : userId(id), password(pwd), name(n), type(t), 
          creditScore(Config::INITIAL_CREDIT), monthlyViolations(0), 
          weeklyCancellations(0), totalVisits(0), totalHours(0) {}
    
    // 检查是否被封禁
    bool isBanned() const {
        if (banEndTime.isValid() && QDateTime::currentDateTime() < banEndTime) return true;
        if (creditScore < Config::MIN_CREDIT_TO_RESERVE) return true;
        return false;
    }
    
    QString getBanReason() const {
        if (banEndTime.isValid() && QDateTime::currentDateTime() < banEndTime) {
            return QString("违约次数过多，封禁至 %1").arg(banEndTime.toString("yyyy-MM-dd HH:mm"));
        }
        if (creditScore < Config::MIN_CREDIT_TO_RESERVE) {
            return QString("信用分不足（当前%1，需要%2）").arg(creditScore).arg(Config::MIN_CREDIT_TO_RESERVE);
        }
        return "";
    }
    
    // 检查本周是否还能取消预约
    bool canCancelThisWeek() const {
        if (!lastCancelTime.isValid()) return true;
        
        QDate today = QDate::currentDate();
        QDate lastCancel = lastCancelTime.date();
        
        // 检查是否在同一周
        int todayWeek = today.weekNumber();
        int lastWeek = lastCancel.weekNumber();
        
        if (today.year() != lastCancel.year() || todayWeek != lastWeek) {
            return true;
        }
        return weeklyCancellations < Config::MAX_CANCEL_PER_WEEK;
    }
    
    // 重置月度/周度统计
    void resetMonthlyStats() {
        QDate today = QDate::currentDate();
        if (lastViolationTime.isValid() && lastViolationTime.date().month() != today.month()) {
            monthlyViolations = 0;
        }
    }
    
    void resetWeeklyStats() {
        QDate today = QDate::currentDate();
        if (lastCancelTime.isValid()) {
            int todayWeek = today.weekNumber();
            int lastWeek = lastCancelTime.date().weekNumber();
            if (today.year() != lastCancelTime.date().year() || todayWeek != lastWeek) {
                weeklyCancellations = 0;
            }
        }
    }
    
    QString getTypeName() const {
        switch(type) {
            case Student: return "本科生";
            case Teacher: return "教师";
            case Graduate: return "研究生";
            case Admin: return "管理员";
            default: return "未知";
        }
    }
    
    // 添加常用座位
    void addFavoriteSeat(const QString& seatId) {
        if (!favoriteSeatIds.contains(seatId)) {
            favoriteSeatIds.prepend(seatId);
            if (favoriteSeatIds.size() > 10) {
                favoriteSeatIds.removeLast();
            }
        } else {
            favoriteSeatIds.removeAll(seatId);
            favoriteSeatIds.prepend(seatId);
        }
    }
};

/* 座位类 */
class Seat {
public:
    QString id;             // 座位ID，如 "XF3A001"
    QString displayName;    // 显示名称，如 "A001"
    SeatStatus status;
    QString currentUserId;
    QDateTime reserveTime;
    QDateTime signInTime;
    QDateTime leaveTime;
    bool hasPower;
    int row, col;           // 在区域中的位置
    int tableGroup;         // 所属桌子组（用于围绕桌子排列）
    int usageCount;         // 使用次数统计
    bool isTomorrowReservation; // 是否是预约明天的座位

    Seat() : status(Available), hasPower(false), row(0), col(0), tableGroup(0),
        usageCount(0), isTomorrowReservation(false) {}

    Seat(const QString& _id, const QString& _displayName, int r = 0, int c = 0, bool power = false, int table = 0)
        : id(_id), displayName(_displayName), status(Available), hasPower(power),
        row(r), col(c), tableGroup(table), usageCount(0), isTomorrowReservation(false) {}

    QColor getStatusColor() const {
        switch(status) {
        case Available: return QColor(76, 175, 80);      // 绿色
        case Occupied: return QColor(120, 120, 120);     // 深灰色
        case Reserved: return QColor(33, 150, 243);      // 蓝色
        case TemporaryLeave: return QColor(255, 193, 7); // 黄色
        case Broken: return QColor(244, 67, 54);         // 红色
        default: return QColor(200, 200, 200);
        }
    }

    QString getStatusName() const {
        switch(status) {
        case Available: return "可用";
        case Occupied: return "使用中";
        case Reserved: return isTomorrowReservation ? "已预约(明天)" : "已预约";
        case TemporaryLeave: return "暂离";
        case Broken: return "维护中";
        default: return "未知";
        }
    }

    bool isLeaveTimeout() const {
        if (status != TemporaryLeave) return false;
        return leaveTime.secsTo(QDateTime::currentDateTime()) > Config::TEMP_LEAVE_TIMEOUT * 60;
    }

    bool isSignInTimeout() const {
        if (status != Reserved) return false;
        if (!reserveTime.isValid()) return false;

        if (isTomorrowReservation) {
            // 预约明天的座位：检查是否超过次日8:30
            QDateTime deadline = QDateTime(reserveTime.date().addDays(1), QTime(8, 30));
            return QDateTime::currentDateTime() > deadline;
        } else {
            // 预约今天的座位：15分钟内签到
            return reserveTime.secsTo(QDateTime::currentDateTime()) > Config::SIGN_IN_TIMEOUT * 60;
        }
    }

    int getRemainingSignInMinutes() const {
        if (status != Reserved) return 0;
        if (!reserveTime.isValid()) return 0;

        if (isTomorrowReservation) {
            // 预约明天：计算到次日8:30的剩余时间
            QDateTime deadline = QDateTime(reserveTime.date().addDays(1), QTime(8, 30));
            int secs = QDateTime::currentDateTime().secsTo(deadline);
            return qMax(0, secs / 60);
        } else {
            // 预约今天：15分钟倒计时
            int elapsed = reserveTime.secsTo(QDateTime::currentDateTime()) / 60;
            return qMax(0, Config::SIGN_IN_TIMEOUT - elapsed);
        }
    }

    int getRemainingLeaveMinutes() const {
        if (status != TemporaryLeave) return 0;
        int elapsed = leaveTime.secsTo(QDateTime::currentDateTime()) / 60;
        return qMax(0, Config::TEMP_LEAVE_TIMEOUT - elapsed);
    }

    // 获取签到截止时间的描述
    QString getSignInDeadlineDesc() const {
        if (status != Reserved) return "";

        if (isTomorrowReservation) {
            return QString("明早8:30前");
        } else {
            int remaining = getRemainingSignInMinutes();
            return QString("%1分钟内").arg(remaining);
        }
    }
};

/* 区域类 */
class Zone {
public:
    QString id;
    QString name;
    QString fullName;       // 完整名称，如 "新书阅览区"
    QList<Seat> seats;
    double x, y, w, h;
    int totalSeats;
    bool isAvailable;       // 区域是否开放
    
    Zone() : x(0), y(0), w(0.1), h(0.1), totalSeats(0), isAvailable(true) {}
    
    Zone(const QString& _id, const QString& _name, const QString& _fullName,
         double _x, double _y, double _w, double _h, int _total = 100, bool _available = true)
        : id(_id), name(_name), fullName(_fullName.isEmpty() ? _name : _fullName),
          x(_x), y(_y), w(_w), h(_h), totalSeats(_total), isAvailable(_available) {}
    
    int getAvailableCount() const {
        if (!isAvailable) return 0;
        int count = 0;
        for (const auto& s : seats) {
            if (s.status == Available) count++;
        }
        return count;
    }
    
    int getTotalCount() const { return seats.size(); }
    
    QColor getZoneColor() const {
        if (!isAvailable) return QColor(158, 158, 158); // 灰色表示不可用
        if (seats.isEmpty()) return QColor(200, 200, 200);
        
        double ratio = (double)getAvailableCount() / getTotalCount();
        
        if (ratio > 0.6) return QColor(139, 195, 74);      // 浅绿
        else if (ratio > 0.3) return QColor(255, 235, 59); // 黄色
        else if (ratio > 0.1) return QColor(255, 152, 0);  // 橙色
        else return QColor(239, 83, 80);                   // 红色
    }
    
    Seat* findSeat(const QString& seatId) {
        for (auto& seat : seats) {
            if (seat.id == seatId) return &seat;
        }
        return nullptr;
    }
};

/* 楼层类 */
class Floor {
public:
    int level;
    QString name;
    QList<Zone> zones;
    int totalSeats;
    
    Floor() : level(0), totalSeats(0) {}
    Floor(int l) : level(l), name(QString("%1楼").arg(l)), totalSeats(0) {}
    
    int getAvailableCount() const {
        int count = 0;
        for (const auto& z : zones) count += z.getAvailableCount();
        return count;
    }
    
    int getTotalCount() const {
        int count = 0;
        for (const auto& z : zones) count += z.getTotalCount();
        return count;
    }
    
    Zone* findZone(const QString& zoneId) {
        for (auto& zone : zones) {
            if (zone.id == zoneId) return &zone;
        }
        return nullptr;
    }
};

/* 校区/分馆类 */
class Campus {
public:
    QString id;
    QString name;
    QColor themeColor;      // 主题颜色
    QList<Floor> floors;
    int dailyVisitors;      // 今日入馆人数
    QDate lastResetDate;    // 上次重置日期
    
    Campus() : dailyVisitors(0) {}
    Campus(const QString& _id, const QString& _name, const QColor& _color)
        : id(_id), name(_name), themeColor(_color), dailyVisitors(0), lastResetDate(QDate::currentDate()) {}
    
    int getAvailableCount() const {
        int count = 0;
        for (const auto& f : floors) count += f.getAvailableCount();
        return count;
    }
    
    int getTotalCount() const {
        int count = 0;
        for (const auto& f : floors) count += f.getTotalCount();
        return count;
    }
    
    Floor* findFloor(int level) {
        for (auto& floor : floors) {
            if (floor.level == level) return &floor;
        }
        return nullptr;
    }
    
    // 重置每日统计
    void resetDailyStats() {
        QDate today = QDate::currentDate();
        if (lastResetDate != today) {
            dailyVisitors = 0;
            lastResetDate = today;
        }
    }
    
    // 增加入馆人数并返回当前是第几位
    int addVisitor() {
        resetDailyStats();
        return ++dailyVisitors;
    }
};

#endif // DATAMODEL_H
