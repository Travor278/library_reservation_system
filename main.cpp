#include <QApplication>
#include <QStyleFactory>
#include <QFont>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("图书馆座位预约系统");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("高级程序设计实践");
    
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);
    
    app.setStyleSheet(
        "QToolTip { "
        "   background-color: #37474F; "
        "   color: white; "
        "   border: 1px solid #263238; "
        "   padding: 10px; "
        "   border-radius: 6px; "
        "   font-size: 14px; "
        "   font-family: 'Microsoft YaHei'; "
        "}"
        "QMessageBox { background-color: white; }"
        "QMessageBox QLabel { color: #333; font-size: 14px; }"
        "QScrollBar:vertical { background: #ECEFF1; width: 10px; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #90A4AE; border-radius: 5px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
