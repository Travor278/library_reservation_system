[English Version](./README_EN.md) | [中文文档](./README.md)
# Library Seat Reservation System v3.0

<table>
  <tr>
    <td width="33%"><img src="docs/校区首页.png" alt="Campus Home" width="100%" /></td>
    <td width="33%"><img src="docs/楼层平面图.png" alt="Floor Plan" width="100%" /></td>
    <td width="33%"><img src="docs/座位地图视图.png" alt="Seat View" width="100%" /></td>
  </tr>
</table>

## 1. System Overview
This is a desktop application developed based on the **Qt/C++** framework, utilizing the **MVC (Model-View-Controller)** architecture. 
It simulates a real-world library environment, providing features such as visual seat selection, reservation management (check-in/temporary leave/check-out), and a credit score system.

## 2. Key Features
* **Multi-Campus Support**: Supports 4 campus libraries (Xiaoxiang, Yuelushan, Tianxin, Xinglin), each with a unique theme color.
* **Visual Seat Selection**:
    * **Heatmap Mode**: Visualizes seat occupancy. Colors indicate crowding levels (Green > 60% free → Red < 10% free).
    * **Dual Views**: Switch seamlessly between **Map View** (Visual layout) and **List View** (Efficient filtering).
* **Comprehensive Reservation Process**:
    * **Dual-Date Booking**: Supports reservations for "Today" and "Tomorrow".
    * **Status Management**: Complete workflow including Check-in (15-min limit), Temporary Leave (30-min limit), and Check-out.
* **Credit Score System**: Users start with 100 points. Penalties for violations (e.g., no-show) and rewards for normal usage. Users with a score below 70 are restricted from booking.
* **Smart Features**: Designed with no global variables; utilizes JSON for data persistence (auto-save on exit).

## 3. Development Environment
* **OS**: Windows 10/11, macOS, Linux
* **Framework**: Qt 5.15+ or Qt 6.x
* **Compiler**: C++17 compatible (MinGW, MSVC, GCC)

## 4. Installation & Usage
1. Open `LibraryReservationSystem.pro` in **Qt Creator**.
2. Configure the Build Kit and click **Run**.
3. **Test Accounts**:
    * **Admin**: `admin` / `admin123`
    * **Student**: `2024001` / `123456`
    * **Teacher**: `T001` / `123456`

## 5. Documentation

For detailed design specifications and user manuals, please refer to [docs/图书馆座位预约系统_软件使用说明.pdf](docs/图书馆座位预约系统_软件使用说明.pdf).

## 6. Screenshots

### Login & Registration

![Login and Registration](docs/登陆注册界面.png)

### Welcome Notice

![Welcome Notice](docs/温馨提示.png)

### Campus & Floor Selection

![Campus Home](docs/校区首页.png)

### Floor Map

![Floor Plan](docs/楼层平面图.png)

### Seat Detail View

![Seat Map View](docs/座位地图视图.png)

### Statistics & Data Export

![Statistics and Export](docs/统计数据及导出1.png)

## 7. System Design

### Business Workflow

![Flowchart](docs/流程图.png)

### Data Flow Diagram

![Data Flow](docs/数据流图.png)

### Class Diagram

![Class Diagram](docs/系统类图.png)
