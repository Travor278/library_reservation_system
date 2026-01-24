#include "LibraryManager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QTime>

LibraryManager::LibraryManager() : currentUser(nullptr), recordCounter(0) {
    initData();
    initAnnouncements();
    
    // 定时检查超时（每分钟）
    timeoutTimer = new QTimer(this);
    connect(timeoutTimer, &QTimer::timeout, this, &LibraryManager::checkTimeouts);
    timeoutTimer->start(60000);
    
    // 分钟定时器（用于更新时间显示）
    minuteTimer = new QTimer(this);
    connect(minuteTimer, &QTimer::timeout, this, &LibraryManager::onMinuteChanged);
    minuteTimer->start(1000);
    
    loadData();
}

LibraryManager::~LibraryManager() {
    saveData();
}

void LibraryManager::initAnnouncements() {
    Announcement a1;
    a1.id = "1";
    a1.title = "寒假图书馆开放通知";
    a1.content = "寒假期间图书馆正常开放，开放时间为7:00-22:30。请同学们合理安排学习时间。";
    a1.createTime = QDateTime::currentDateTime();
    a1.expireTime = QDateTime::currentDateTime().addDays(30);
    a1.isImportant = false;
    announcements.append(a1);
    
    Announcement a2;
    a2.id = "2";
    a2.title = "⚠️ 安全提醒";
    a2.content = "请勿携带电动车电池、易燃易爆危险品入馆！请勿在馆内使用大功率电器设备！";
    a2.createTime = QDateTime::currentDateTime();
    a2.expireTime = QDateTime::currentDateTime().addYears(1);
    a2.isImportant = true;
    announcements.append(a2);
    
    Announcement a3;
    a3.id = "3";
    a3.title = "图书馆清理通知";
    a3.content = "每周日下午14:00-16:00为图书馆清洁时间，期间暂停预约服务。";
    a3.createTime = QDateTime::currentDateTime();
    a3.expireTime = QDateTime::currentDateTime().addMonths(6);
    a3.isImportant = false;
    announcements.append(a3);
}

void LibraryManager::initData() {
    // 初始化用户
    users["admin"] = User("admin", "admin123", "管理员", Admin);
    users["2024001"] = User("2024001", "123456", "张三", Student);
    users["2024002"] = User("2024002", "123456", "李四", Student);
    users["T001"] = User("T001", "123456", "王老师", Teacher);
    
    initXiaoxiangCampus();
    initOtherCampuses();
}

void LibraryManager::initXiaoxiangCampus() {
    Campus xx("xiaoxiang", "潇湘校区馆", QColor(102, 126, 234)); // 紫蓝色
    
    // 一楼
    {
        Floor f(1);
        initFloor1Seats(f);
        xx.floors.append(f);
    }
    
    // 二楼
    {
        Floor f(2);
        initFloor2Seats(f);
        xx.floors.append(f);
    }
    
    // 三楼
    {
        Floor f(3);
        initFloor3Seats(f);
        xx.floors.append(f);
    }
    
    // 四楼
    {
        Floor f(4);
        initFloor4Seats(f);
        xx.floors.append(f);
    }
    
    // 五楼
    {
        Floor f(5);
        initFloor5Seats(f);
        xx.floors.append(f);
    }
    
    // 六楼
    {
        Floor f(6);
        initFloor6Seats(f);
        xx.floors.append(f);
    }
    
    // 七楼
    {
        Floor f(7);
        initFloor7Seats(f);
        xx.floors.append(f);
    }
    
    campuses.append(xx);
}

// 创建围绕桌子排列的座位
void LibraryManager::createTableSeats(Zone& zone, const QString& prefix, int startNum,
                                       int seatsPerTable, int numTables, int tablesPerRow) {
    int seatNum = startNum;
    int tableId = 0;
    
    for (int t = 0; t < numTables && seatNum <= zone.totalSeats; t++) {
        tableId++;
        int tableRow = t / tablesPerRow;
        int tableCol = t % tablesPerRow;
        
        // 每张桌子的座位（桌子两边各seatsPerTable/2个座位）
        for (int s = 0; s < seatsPerTable && seatNum <= zone.totalSeats; s++) {
            QString seatId = QString("%1%2").arg(prefix).arg(seatNum, 3, 10, QChar('0'));
            QString displayName = QString("%1%2").arg(zone.id).arg(seatNum, 3, 10, QChar('0'));
            
            int row = tableRow * 2 + (s < seatsPerTable/2 ? 0 : 1);
            int col = tableCol * (seatsPerTable/2 + 1) + (s % (seatsPerTable/2));
            bool hasPower = (s == 0 || s == seatsPerTable-1); // 桌子两端有电源
            
            Seat seat(seatId, displayName, row, col, hasPower, tableId);
            zone.seats.append(seat);
            seatNum++;
        }
    }
}

void LibraryManager::initFloor1Seats(Floor& floor) {
    // 一楼有C区（新书阅览区）和D区（传习书屋）
    // C区 - 新书阅览区（16个座位，2张桌子，每张8座）
    Zone zc("C", "C区", "新书阅览区", 0.75, 0.15, 0.15, 0.25, 16, true);
    createTableSeats(zc, "XF1C", 1, 8, 2, 2);
    floor.zones.append(zc);
    
    // D区 - 传习书屋（16个座位）
    Zone zd("D", "D区", "传习书屋", 0.08, 0.35, 0.15, 0.2, 16, true);
    createTableSeats(zd, "XF1D", 1, 8, 2, 2);
    floor.zones.append(zd);
}

void LibraryManager::initFloor2Seats(Floor& floor) {
    // 二楼A区、F区暂停预约，只有B区、C区可用
    // A区 - 暂停预约（中间偏左）
    Zone za("A", "A区", "A区", 0.25, 0.30, 0.18, 0.20, 0, false);
    floor.zones.append(za);

    // B区（右上）
    Zone zb("B", "B区", "B区", 0.55, 0.15, 0.18, 0.18, 26, true);
    createTableSeats(zb, "XF2B", 1, 6, 5, 3);
    floor.zones.append(zb);

    // C区（右下）
    Zone zc("C", "C区", "C区", 0.55, 0.45, 0.18, 0.18, 26, true);
    createTableSeats(zc, "XF2C", 1, 6, 5, 3);
    floor.zones.append(zc);

    // F区 - 暂停预约（左侧）
    Zone zf("F", "F区", "研修间", 0.08, 0.40, 0.12, 0.15, 0, false);
    floor.zones.append(zf);
}

void LibraryManager::initFloor3Seats(Floor& floor) {
    // 三楼有A、B、C、D、E区，F区研修间暂停
    // A区（中间，168座位）
    Zone za("A", "A区", "A区", 0.30, 0.20, 0.22, 0.25, 168, true);
    createTableSeats(za, "XF3A", 1, 6, 28, 4);
    floor.zones.append(za);

    // B区（右上，44座位）
    Zone zb("B", "B区", "B区", 0.60, 0.08, 0.25, 0.18, 44, true);
    createTableSeats(zb, "XF3B", 1, 6, 8, 4);
    floor.zones.append(zb);

    // C区（右中，18座位）
    Zone zc("C", "C区", "C区", 0.65, 0.35, 0.15, 0.18, 18, true);
    createTableSeats(zc, "XF3C", 1, 6, 3, 1);
    floor.zones.append(zc);

    // D区（中下，136座位）
    Zone zd("D", "D区", "D区", 0.35, 0.55, 0.20, 0.30, 136, true);
    createTableSeats(zd, "XF3D", 1, 6, 23, 3);
    floor.zones.append(zd);

    // E区（左下，136座位）
    Zone ze("E", "E区", "E区", 0.08, 0.55, 0.18, 0.30, 136, true);
    createTableSeats(ze, "XF3E", 1, 6, 23, 3);
    floor.zones.append(ze);

    // F区 - 研修间（左中，暂停预约）
    Zone zf("F", "F区", "研修间预约", 0.08, 0.30, 0.12, 0.15, 0, false);
    floor.zones.append(zf);
}

void LibraryManager::initFloor4Seats(Floor& floor) {
    // 四楼布局
    // A区（中间）
    Zone za("A", "A区", "A区", 0.30, 0.15, 0.22, 0.25, 120, true);
    createTableSeats(za, "XF4A", 1, 6, 20, 4);
    floor.zones.append(za);

    // B区（右上）
    Zone zb("B", "B区", "B区", 0.60, 0.10, 0.20, 0.18, 60, true);
    createTableSeats(zb, "XF4B", 1, 6, 10, 3);
    floor.zones.append(zb);

    // C区（右中）
    Zone zc("C", "C区", "C区", 0.65, 0.38, 0.15, 0.18, 50, true);
    createTableSeats(zc, "XF4C", 1, 6, 9, 3);
    floor.zones.append(zc);

    // D区（中下）
    Zone zd("D", "D区", "D区", 0.40, 0.55, 0.22, 0.28, 80, true);
    createTableSeats(zd, "XF4D", 1, 6, 14, 3);
    floor.zones.append(zd);

    // E区（左下）
    Zone ze("E", "E区", "E区", 0.08, 0.45, 0.18, 0.28, 80, true);
    createTableSeats(ze, "XF4E", 1, 6, 14, 3);
    floor.zones.append(ze);

    // F区 - 研修间（左中，暂停）
    Zone zf("F", "F区", "研修间预约", 0.08, 0.20, 0.12, 0.15, 0, false);
    floor.zones.append(zf);
}

void LibraryManager::initFloor5Seats(Floor& floor) {
    // 五楼布局
    // A区（左上）
    Zone za("A", "A区", "A区", 0.25, 0.12, 0.22, 0.28, 100, true);
    createTableSeats(za, "XF5A", 1, 6, 17, 4);
    floor.zones.append(za);

    // B区（右上）
    Zone zb("B", "B区", "B区", 0.55, 0.12, 0.22, 0.22, 80, true);
    createTableSeats(zb, "XF5B", 1, 6, 14, 3);
    floor.zones.append(zb);

    // C区（右中）
    Zone zc("C", "C区", "C区", 0.65, 0.38, 0.15, 0.18, 50, true);
    createTableSeats(zc, "XF4C", 1, 6, 9, 3);
    floor.zones.append(zc);

    // D区（左下，往上移动避免与入口重叠）
    Zone zd("D", "D区", "D区", 0.08, 0.45, 0.22, 0.28, 80, true);
    createTableSeats(zd, "XF5D", 1, 6, 14, 3);
    floor.zones.append(zd);
}

void LibraryManager::initFloor6Seats(Floor& floor) {
    // 六楼布局
    // A区（左上）
    Zone za("A", "A区", "A区", 0.25, 0.12, 0.22, 0.28, 100, true);
    createTableSeats(za, "XF6A", 1, 6, 17, 4);
    floor.zones.append(za);

    // B区（右上）
    Zone zb("B", "B区", "B区", 0.55, 0.12, 0.22, 0.22, 80, true);
    createTableSeats(zb, "XF6B", 1, 6, 14, 3);
    floor.zones.append(zb);

    // C区（右中）
    Zone zc("C", "C区", "C区", 0.65, 0.38, 0.15, 0.18, 50, true);
    createTableSeats(zc, "XF4C", 1, 6, 9, 3);
    floor.zones.append(zc);

    // D区（左下，往上移动避免与入口重叠）
    Zone zd("D", "D区", "D区", 0.08, 0.45, 0.22, 0.28, 80, true);
    createTableSeats(zd, "XF6D", 1, 6, 14, 3);
    floor.zones.append(zd);
}

void LibraryManager::initFloor7Seats(Floor& floor) {
    // 七楼布局
    Zone za("A", "A区", "A区", 0.22, 0.15, 0.15, 0.2, 80, true);
    createTableSeats(za, "XF7A", 1, 6, 14, 3);
    floor.zones.append(za);
    
    Zone zb("B", "B区", "B区", 0.45, 0.12, 0.12, 0.15, 60, true);
    createTableSeats(zb, "XF7B", 1, 6, 10, 3);
    floor.zones.append(zb);
    
    Zone zc("C", "C区", "C区", 0.45, 0.35, 0.1, 0.12, 40, true);
    createTableSeats(zc, "XF7C", 1, 6, 7, 2);
    floor.zones.append(zc);
    
    Zone zd("D", "D区", "D区", 0.35, 0.55, 0.15, 0.2, 80, true);
    createTableSeats(zd, "XF7D", 1, 6, 14, 3);
    floor.zones.append(zd);
    
    Zone zf("F", "F区", "研修间预约", 0.12, 0.25, 0.08, 0.08, 0, false);
    floor.zones.append(zf);
}

void LibraryManager::initOtherCampuses() {
    // 岳麓山校区馆 - 蓝绿色
    {
        Campus yl("yuelu", "岳麓山校区馆", QColor(0, 188, 212));
        for (int i = 1; i <= 5; i++) {
            Floor f(i);
            Zone za("A", "A区", "阅览区A", 0.2, 0.2, 0.25, 0.25, 80, true);
            createTableSeats(za, QString("YL%1A").arg(i), 1, 6, 14, 4);
            f.zones.append(za);
            
            Zone zb("B", "B区", "阅览区B", 0.5, 0.2, 0.25, 0.25, 80, true);
            createTableSeats(zb, QString("YL%1B").arg(i), 1, 6, 14, 4);
            f.zones.append(zb);
            
            Zone zc("C", "C区", "自习区", 0.35, 0.50, 0.25, 0.22, 60, true);
            createTableSeats(zc, QString("YL%1C").arg(i), 1, 6, 10, 3);
            f.zones.append(zc);
            
            yl.floors.append(f);
        }
        campuses.append(yl);
    }
    
    // 天心校区馆 - 橙色
    {
        Campus tx("tianxin", "天心校区馆", QColor(255, 152, 0));
        for (int i = 1; i <= 4; i++) {
            Floor f(i);
            Zone za("A", "A区", "主阅览区", 0.15, 0.15, 0.3, 0.3, 100, true);
            createTableSeats(za, QString("TX%1A").arg(i), 1, 6, 17, 4);
            f.zones.append(za);
            
            Zone zb("B", "B区", "自习区", 0.55, 0.15, 0.3, 0.3, 100, true);
            createTableSeats(zb, QString("TX%1B").arg(i), 1, 6, 17, 4);
            f.zones.append(zb);
            
            tx.floors.append(f);
        }
        campuses.append(tx);
    }
    
    // 杏林校区馆 - 绿色
    {
        Campus xl("xinglin", "杏林校区馆", QColor(76, 175, 80));
        for (int i = 1; i <= 3; i++) {
            Floor f(i);
            Zone za("A", "A区", "医学阅览", 0.1, 0.1, 0.35, 0.4, 120, true);
            createTableSeats(za, QString("XL%1A").arg(i), 1, 6, 20, 4);
            f.zones.append(za);
            
            Zone zb("B", "B区", "自习区", 0.55, 0.1, 0.35, 0.4, 120, true);
            createTableSeats(zb, QString("XL%1B").arg(i), 1, 6, 20, 4);
            f.zones.append(zb);
            
            Zone zc("C", "C区", "电子阅览", 0.38, 0.55, 0.35, 0.25, 50, true);
            createTableSeats(zc, QString("XL%1C").arg(i), 1, 6, 9, 3);
            f.zones.append(zc);
            
            xl.floors.append(f);
        }
        campuses.append(xl);
    }
}

Campus* LibraryManager::getCampus(const QString& name) {
    for (auto& c : campuses) {
        if (c.name == name) return &c;
    }
    return nullptr;
}

Campus* LibraryManager::getCampusById(const QString& id) {
    for (auto& c : campuses) {
        if (c.id == id) return &c;
    }
    return nullptr;
}

/* 时间判断相关方法 */
bool LibraryManager::isOpenTime() const {
    QTime now = QTime::currentTime();
    return now >= Config::OPEN_TIME && now <= Config::CLOSE_TIME;
}

bool LibraryManager::canReserve() const {
    QTime now = QTime::currentTime();
    return now >= Config::OPEN_TIME && now <= Config::RESERVATION_END;
}

bool LibraryManager::canReserveTomorrow() const {
    QTime now = QTime::currentTime();
    return now >= QTime(18, 0); // 18:00后可预约明天
}

QString LibraryManager::getTimeStatus() const {
    QTime now = QTime::currentTime();
    
    if (now < Config::OPEN_TIME) {
        return QString("图书馆尚未开放，开放时间: %1").arg(Config::OPEN_TIME.toString("HH:mm"));
    } else if (now > Config::CLOSE_TIME) {
        return "图书馆已闭馆";
    } else if (now > Config::RESERVATION_END) {
        return QString("今日预约已结束，闭馆时间: %1").arg(Config::CLOSE_TIME.toString("HH:mm"));
    } else {
        return QString("预约开放中，截止时间: %1").arg(Config::RESERVATION_END.toString("HH:mm"));
    }
}

int LibraryManager::getTodayVisitorCount(const QString& campusId) {
    Campus* campus = getCampusById(campusId);
    if (campus) {
        campus->resetDailyStats();
        return campus->dailyVisitors;
    }
    return 0;
}

/* 用户认证 */
bool LibraryManager::login(const QString& userId, const QString& password) {
    if (!users.contains(userId)) {
        emit operationFailed("用户不存在");
        return false;
    }
    
    User& user = users[userId];
    if (user.password != password) {
        emit operationFailed("密码错误");
        return false;
    }
    
    // 重置统计
    user.resetMonthlyStats();
    user.resetWeeklyStats();
    
    if (user.isBanned()) {
        emit operationFailed(user.getBanReason());
        return false;
    }
    
    currentUser = &user;
    emit userLoggedIn(currentUser);
    return true;
}

void LibraryManager::logout() {
    currentUser = nullptr;
    emit userLoggedOut();
}

bool LibraryManager::registerUser(const QString& userId, const QString& password, const QString& name, UserType type) {
    if (users.contains(userId)) {
        emit operationFailed("用户ID已存在");
        return false;
    }
    
    users[userId] = User(userId, password, name, type);
    saveData();
    return true;
}

User* LibraryManager::getUser(const QString& userId) {
    if (users.contains(userId)) {
        return &users[userId];
    }
    return nullptr;
}

/* 座位操作方法 */
bool LibraryManager::reserveSeat(const QString& campusId, int floor, const QString& zoneId, const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    currentUser->resetMonthlyStats();
    currentUser->resetWeeklyStats();
    
    if (currentUser->isBanned()) {
        emit operationFailed(currentUser->getBanReason());
        return false;
    }
    
    if (!canReserve()) {
        emit operationFailed(getTimeStatus());
        return false;
    }
    
    if (getUserCurrentSeat() != nullptr) {
        emit operationFailed("您已有正在使用或预约的座位");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat) {
        emit operationFailed("座位不存在");
        return false;
    }
    
    if (seat->status != Available) {
        emit operationFailed("座位不可用");
        return false;
    }
    
    seat->status = Reserved;
    seat->currentUserId = currentUser->userId;
    seat->reserveTime = QDateTime::currentDateTime();
    seat->usageCount++;
    
    // 添加到常用座位
    currentUser->addFavoriteSeat(seatId);
    
    addRecord(seatId, "预约");
    
    // 获取入馆序号
    Campus* campus = getCampusById(campusId);
    int visitorNum = campus ? campus->addVisitor() : 0;
    
    emit reservationSuccess(seatId, visitorNum);
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::signIn(const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat) {
        emit operationFailed("座位不存在");
        return false;
    }
    
    if (seat->currentUserId != currentUser->userId) {
        emit operationFailed("这不是您预约的座位");
        return false;
    }
    
    if (seat->status != Reserved) {
        emit operationFailed("座位状态错误");
        return false;
    }
    
    // 检查是否超时
    if (seat->isSignInTimeout()) {
        seat->status = Available;
        seat->currentUserId.clear();
        addViolation(currentUser);
        addRecord(seatId, "签到超时(违约)");
        emit operationFailed("签到超时，预约已取消，记录违约一次，信用分-10");
        emit dataChanged();
        saveData();
        return false;
    }
    
    seat->status = Occupied;
    seat->signInTime = QDateTime::currentDateTime();
    seat->isTomorrowReservation = false;  // 签到后清除标记
    currentUser->totalVisits++;

    // 获取入馆序号
    int visitorNum = 0;
    for (auto& campus : campuses) {
        for (auto& floor : campus.floors) {
            for (auto& zone : floor.zones) {
                if (zone.findSeat(seatId)) {
                    visitorNum = campus.addVisitor();
                    goto found;  // 找到后跳出三层循环
                }
            }
        }
    }
    found:

    addRecord(seatId, "签到");
    emit signInSuccess(seatId, visitorNum);
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::signOut(const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat || seat->currentUserId != currentUser->userId) {
        emit operationFailed("座位错误");
        return false;
    }
    
    // 计算使用时长并增加信用分
    if (seat->signInTime.isValid()) {
        int hours = seat->signInTime.secsTo(QDateTime::currentDateTime()) / 3600;
        currentUser->totalHours += hours;
        int creditGain = qMin(hours * Config::CREDIT_PER_HOUR, 5); // 每次最多加5分
        currentUser->creditScore = qMin(currentUser->creditScore + creditGain, Config::MAX_CREDIT);
    }
    
    seat->status = Available;
    seat->currentUserId.clear();
    seat->reserveTime = QDateTime();
    seat->signInTime = QDateTime();
    seat->leaveTime = QDateTime();
    
    addRecord(seatId, "签退");
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::setTemporaryLeave(const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat || seat->currentUserId != currentUser->userId) {
        emit operationFailed("座位错误");
        return false;
    }
    
    if (seat->status != Occupied) {
        emit operationFailed("只有在使用中的座位才能暂离");
        return false;
    }
    
    seat->status = TemporaryLeave;
    seat->leaveTime = QDateTime::currentDateTime();
    
    addRecord(seatId, "暂离");
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::returnFromLeave(const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat || seat->currentUserId != currentUser->userId) {
        emit operationFailed("座位错误");
        return false;
    }
    
    if (seat->status != TemporaryLeave) {
        emit operationFailed("座位状态错误");
        return false;
    }
    
    if (seat->isLeaveTimeout()) {
        seat->status = Available;
        seat->currentUserId.clear();
        addViolation(currentUser);
        addRecord(seatId, "暂离超时(违约)");
        emit operationFailed("暂离超时，座位已释放，记录违约一次，信用分-10");
        emit dataChanged();
        saveData();
        return false;
    }
    
    seat->status = Occupied;
    seat->leaveTime = QDateTime();
    
    addRecord(seatId, "返回");
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::cancelReservation(const QString& seatId) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }
    
    // 检查本周取消次数
    currentUser->resetWeeklyStats();
    if (!currentUser->canCancelThisWeek()) {
        emit operationFailed("本周取消预约次数已用完（每周最多1次）");
        return false;
    }
    
    Seat* seat = findSeat(seatId);
    if (!seat || seat->currentUserId != currentUser->userId) {
        emit operationFailed("座位错误");
        return false;
    }
    
    if (seat->status != Reserved) {
        emit operationFailed("只能取消预约状态的座位");
        return false;
    }
    
    seat->status = Available;
    seat->currentUserId.clear();
    seat->reserveTime = QDateTime();
    
    currentUser->weeklyCancellations++;
    currentUser->lastCancelTime = QDateTime::currentDateTime();
    
    addRecord(seatId, "取消预约");
    emit dataChanged();
    saveData();
    return true;
}

bool LibraryManager::reserveSeatForDate(const QString& campusId, int floor,
                                        const QString& zoneId, const QString& seatId,
                                        bool isTomorrow) {
    if (!currentUser) {
        emit operationFailed("请先登录");
        return false;
    }

    currentUser->resetMonthlyStats();
    currentUser->resetWeeklyStats();

    if (currentUser->isBanned()) {
        emit operationFailed(currentUser->getBanReason());
        return false;
    }

    // 根据预约日期检查时间
    if (isTomorrow) {
        if (!canReserveTomorrow()) {
            emit operationFailed("18:00后才能预约明天的座位");
            return false;
        }
    } else {
        if (!canReserve()) {
            emit operationFailed(getTimeStatus());
            return false;
        }
    }

    if (getUserCurrentSeat() != nullptr) {
        emit operationFailed("您已有正在使用或预约的座位");
        return false;
    }

    Seat* seat = findSeat(seatId);
    if (!seat) {
        emit operationFailed("座位不存在");
        return false;
    }

    if (seat->status != Available) {
        emit operationFailed("座位不可用");
        return false;
    }

    seat->status = Reserved;
    seat->currentUserId = currentUser->userId;
    seat->reserveTime = QDateTime::currentDateTime();
    seat->isTomorrowReservation = isTomorrow;  // 需要在 Seat 结构中添加这个字段
    seat->usageCount++;

    currentUser->addFavoriteSeat(seatId);

    QString actionStr = isTomorrow ? "预约(明天)" : "预约";
    addRecord(seatId, actionStr);

    Campus* campus = getCampusById(campusId);
    int visitorNum = campus ? campus->addVisitor() : 0;

    emit reservationSuccess(seatId, visitorNum);
    emit dataChanged();
    saveData();
    return true;
}

/* 查询方法 */
Seat* LibraryManager::findSeat(const QString& seatId) {
    for (auto& campus : campuses) {
        for (auto& floor : campus.floors) {
            for (auto& zone : floor.zones) {
                for (auto& seat : zone.seats) {
                    if (seat.id == seatId) return &seat;
                }
            }
        }
    }
    return nullptr;
}

Seat* LibraryManager::getUserCurrentSeat() {
    if (!currentUser) return nullptr;
    
    for (auto& campus : campuses) {
        for (auto& floor : campus.floors) {
            for (auto& zone : floor.zones) {
                for (auto& seat : zone.seats) {
                    if (seat.currentUserId == currentUser->userId &&
                        (seat.status == Reserved || seat.status == Occupied || seat.status == TemporaryLeave)) {
                        return &seat;
                    }
                }
            }
        }
    }
    return nullptr;
}

QList<Seat*> LibraryManager::getRecommendedSeats(const QString& campusId, int maxCount) {
    QList<Seat*> result;
    Campus* campus = getCampusById(campusId);
    if (!campus) return result;
    
    // 优先推荐：1.用户常用座位 2.有电源的座位 3.使用率高的座位
    QList<Seat*> allAvailable;
    
    for (auto& floor : campus->floors) {
        for (auto& zone : floor.zones) {
            if (!zone.isAvailable) continue;
            for (auto& seat : zone.seats) {
                if (seat.status == Available) {
                    allAvailable.append(&seat);
                }
            }
        }
    }
    
    // 按推荐度排序
    std::sort(allAvailable.begin(), allAvailable.end(), [this](Seat* a, Seat* b) {
        int scoreA = 0, scoreB = 0;
        
        // 常用座位加分
        if (currentUser && currentUser->favoriteSeatIds.contains(a->id)) scoreA += 100;
        if (currentUser && currentUser->favoriteSeatIds.contains(b->id)) scoreB += 100;
        
        // 有电源加分
        if (a->hasPower) scoreA += 50;
        if (b->hasPower) scoreB += 50;
        
        // 使用率加分
        scoreA += a->usageCount;
        scoreB += b->usageCount;
        
        return scoreA > scoreB;
    });
    
    for (int i = 0; i < qMin(maxCount, allAvailable.size()); i++) {
        result.append(allAvailable[i]);
    }
    
    return result;
}

QList<Seat*> LibraryManager::getUserFavoriteSeats() {
    QList<Seat*> result;
    if (!currentUser) return result;
    
    for (const QString& seatId : currentUser->favoriteSeatIds) {
        Seat* seat = findSeat(seatId);
        if (seat) {
            result.append(seat);
        }
    }
    return result;
}

QList<UsageRecord> LibraryManager::getUserHistory(const QString& userId) {
    QList<UsageRecord> result;
    for (const auto& rec : history) {
        if (rec.userId == userId) {
            result.append(rec);
        }
    }
    return result;
}

QList<UsageRecord> LibraryManager::getAllHistory() {
    return history;
}

/* 辅助函数 */
void LibraryManager::addRecord(const QString& seatId, const QString& action) {
    if (!currentUser) return;
    
    UsageRecord rec;
    rec.recordId = generateRecordId();
    rec.seatId = seatId;
    rec.userId = currentUser->userId;
    rec.userName = currentUser->name;
    rec.startTime = QDateTime::currentDateTime();
    rec.action = action;
    
    // 查找座位所属信息
    Seat* seat = findSeat(seatId);
    if (seat) {
        rec.seatName = seat->displayName;
    }
    
    history.prepend(rec);
    
    while (history.size() > 1000) {
        history.removeLast();
    }
}

void LibraryManager::addViolation(User* user) {
    if (!user) return;
    
    user->creditScore = qMax(0, user->creditScore - Config::CREDIT_VIOLATION_DEDUCT);
    user->monthlyViolations++;
    user->lastViolationTime = QDateTime::currentDateTime();
    
    // 检查是否需要封禁
    if (user->monthlyViolations >= Config::MAX_VIOLATION_PER_MONTH) {
        user->banEndTime = QDateTime::currentDateTime().addDays(Config::BAN_DAYS);
    }
}

QString LibraryManager::generateRecordId() {
    return QString("R%1%2").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss")).arg(++recordCounter, 4, 10, QChar('0'));
}

void LibraryManager::checkTimeouts() {
    bool changed = false;
    
    for (auto& campus : campuses) {
        for (auto& floor : campus.floors) {
            for (auto& zone : floor.zones) {
                for (auto& seat : zone.seats) {
                    // 检查暂离超时
                    if (seat.status == TemporaryLeave && seat.isLeaveTimeout()) {
                        User* user = getUser(seat.currentUserId);
                        if (user) {
                            addViolation(user);
                        }
                        seat.status = Available;
                        seat.currentUserId.clear();
                        changed = true;
                    }
                    
                    // 检查预约超时
                    if (seat.status == Reserved && seat.isSignInTimeout()) {
                        User* user = getUser(seat.currentUserId);
                        if (user) {
                            addViolation(user);
                        }
                        seat.status = Available;
                        seat.currentUserId.clear();
                        changed = true;
                    }
                }
            }
        }
    }
    
    if (changed) {
        emit dataChanged();
        saveData();
    }
}

void LibraryManager::onMinuteChanged() {
    emit timeStatusChanged();
}

/* 数据持久化 */
bool LibraryManager::saveData() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    
    QJsonObject root;
    
    // 保存用户
    QJsonArray usersArray;
    for (auto it = users.begin(); it != users.end(); ++it) {
        QJsonObject userObj;
        userObj["userId"] = it->userId;
        userObj["password"] = it->password;
        userObj["name"] = it->name;
        userObj["type"] = (int)it->type;
        userObj["creditScore"] = it->creditScore;
        userObj["monthlyViolations"] = it->monthlyViolations;
        userObj["weeklyCancellations"] = it->weeklyCancellations;
        userObj["totalVisits"] = it->totalVisits;
        userObj["totalHours"] = it->totalHours;
        
        QJsonArray favorites;
        for (const auto& fav : it->favoriteSeatIds) {
            favorites.append(fav);
        }
        userObj["favorites"] = favorites;
        
        usersArray.append(userObj);
    }
    root["users"] = usersArray;
    
    // 保存历史记录
    QJsonArray historyArray;
    int count = 0;
    for (const auto& rec : history) {
        if (count++ > 500) break;
        QJsonObject recObj;
        recObj["recordId"] = rec.recordId;
        recObj["seatId"] = rec.seatId;
        recObj["seatName"] = rec.seatName;
        recObj["userId"] = rec.userId;
        recObj["userName"] = rec.userName;
        recObj["startTime"] = rec.startTime.toString(Qt::ISODate);
        recObj["action"] = rec.action;
        historyArray.append(recObj);
    }
    root["history"] = historyArray;
    
    QFile file(path + "/library_v3_data.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        file.close();
        return true;
    }
    return false;
}

bool LibraryManager::loadData() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(path + "/library_v2_data.json");
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull()) return false;
    
    QJsonObject root = doc.object();
    
    // 加载用户
    QJsonArray usersArray = root["users"].toArray();
    for (const auto& val : usersArray) {
        QJsonObject obj = val.toObject();
        QString id = obj["userId"].toString();
        if (!users.contains(id)) {
            users[id] = User();
        }
        users[id].userId = id;
        users[id].password = obj["password"].toString();
        users[id].name = obj["name"].toString();
        users[id].type = (UserType)obj["type"].toInt();
        users[id].creditScore = obj["creditScore"].toInt(Config::INITIAL_CREDIT);
        users[id].monthlyViolations = obj["monthlyViolations"].toInt();
        users[id].weeklyCancellations = obj["weeklyCancellations"].toInt();
        users[id].totalVisits = obj["totalVisits"].toInt();
        users[id].totalHours = obj["totalHours"].toInt();
        
        QJsonArray favorites = obj["favorites"].toArray();
        for (const auto& fav : favorites) {
            users[id].favoriteSeatIds.append(fav.toString());
        }
    }
    
    // 加载历史
    history.clear();
    QJsonArray historyArray = root["history"].toArray();
    for (const auto& val : historyArray) {
        QJsonObject obj = val.toObject();
        UsageRecord rec;
        rec.recordId = obj["recordId"].toString();
        rec.seatId = obj["seatId"].toString();
        rec.seatName = obj["seatName"].toString();
        rec.userId = obj["userId"].toString();
        rec.userName = obj["userName"].toString();
        rec.startTime = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
        rec.action = obj["action"].toString();
        history.append(rec);
    }
    
    return true;
}
