#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include "LibraryManager.h"

class LoginDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit LoginDialog(QWidget* parent = nullptr);
    
signals:
    void loginSuccess();
    
private slots:
    void onLogin();
    void onRegister();
    void switchToRegister();
    void switchToLogin();
    
private:
    void setupUI();
    
    QStackedWidget* stackedWidget;
    
    // 登录页面
    QWidget* loginPage;
    QLineEdit* loginUserIdEdit;
    QLineEdit* loginPasswordEdit;
    QPushButton* loginBtn;
    QPushButton* toRegisterBtn;
    
    // 注册页面
    QWidget* registerPage;
    QLineEdit* regUserIdEdit;
    QLineEdit* regPasswordEdit;
    QLineEdit* regConfirmEdit;
    QLineEdit* regNameEdit;
    QComboBox* regTypeCombo;
    QPushButton* registerBtn;
    QPushButton* toLoginBtn;
};

#endif // LOGINDIALOG_H
