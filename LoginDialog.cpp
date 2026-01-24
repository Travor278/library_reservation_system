/*
 * LoginDialog.cpp
 * 登录对话框实现
 */

#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
    setWindowTitle("图书馆座位预约系统 - 登录");
    setFixedSize(520, 680);

    setStyleSheet(
        "QDialog { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "       stop:0 #667eea, stop:0.5 #764ba2, stop:1 #f093fb); "
        "}"
        );
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(35, 35, 35, 35);
    mainLayout->setSpacing(0);

    // 顶部标题区
    QWidget* headerWidget = new QWidget();
    headerWidget->setFixedHeight(100);
    headerWidget->setStyleSheet("background-color: transparent;");

    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    QLabel* titleLabel = new QLabel("📚 图书馆座位预约系统");
    titleLabel->setStyleSheet("color: white; font-size: 26px; font-weight: bold; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* subtitleLabel = new QLabel("Library Seat Reservation System");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px; background: transparent;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    mainLayout->addWidget(headerWidget);
    mainLayout->addSpacing(20);

    // 内容区（白色卡片）
    stackedWidget = new QStackedWidget();
    stackedWidget->setStyleSheet("QStackedWidget { background-color: white; border-radius: 16px; }");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(40);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 10);
    stackedWidget->setGraphicsEffect(shadow);

    /* 输入框样式 - 加大字体和内边距 */
    QString inputStyle =
        "QLineEdit { "
        "   padding: 15px 18px; "
        "   border: 2px solid #D0D0D0; "
        "   border-radius: 8px; "
        "   font-size: 18px; "          // 字体大小
        "   font-family: 'Microsoft YaHei', 'Arial'; "
        "   background-color: #FAFAFA; "
        "   color: #222222; "           // 深黑色文字
        "   letter-spacing: 1px; "      // 字间距
        "}"
        "QLineEdit:focus { "
        "   border-color: #667eea; "
        "   background-color: white; "
        "}"
        "QLineEdit::placeholder { "
        "   color: #999999; "
        "   font-size: 16px; "
        "}";

    QString labelStyle = "font-size: 16px; color: #333; font-weight: 600; background: transparent;";

    QString buttonStyle =
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2); "
        "   color: white; padding: 16px; "
        "   border: none; border-radius: 10px; font-size: 18px; font-weight: bold; }"
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5a6fd6, stop:1 #6a4190); }";

    /* 登陆界面 */
    loginPage = new QWidget();
    loginPage->setStyleSheet("background-color: white; border-radius: 16px;");
    QVBoxLayout* loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setSpacing(12);
    loginLayout->setContentsMargins(45, 40, 45, 40);

    QLabel* loginTitle = new QLabel("用户登录");
    loginTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #333; background: transparent;");
    loginTitle->setAlignment(Qt::AlignCenter);
    loginLayout->addWidget(loginTitle);
    loginLayout->addSpacing(25);

    QLabel* idLabel = new QLabel("学号/工号");
    idLabel->setStyleSheet(labelStyle);
    loginLayout->addWidget(idLabel);

    loginUserIdEdit = new QLineEdit();
    loginUserIdEdit->setPlaceholderText("请输入学号或工号");
    loginUserIdEdit->setStyleSheet(inputStyle);
    loginUserIdEdit->setMinimumHeight(55);
    loginLayout->addWidget(loginUserIdEdit);
    loginLayout->addSpacing(8);

    QLabel* pwdLabel = new QLabel("密码");
    pwdLabel->setStyleSheet(labelStyle);
    loginLayout->addWidget(pwdLabel);

    loginPasswordEdit = new QLineEdit();
    loginPasswordEdit->setPlaceholderText("请输入密码");
    loginPasswordEdit->setEchoMode(QLineEdit::Password);
    loginPasswordEdit->setStyleSheet(inputStyle);
    loginPasswordEdit->setMinimumHeight(55);
    loginLayout->addWidget(loginPasswordEdit);

    loginLayout->addSpacing(25);

    loginBtn = new QPushButton("登 录");
    loginBtn->setStyleSheet(buttonStyle);
    loginBtn->setMinimumHeight(55);
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(loginPasswordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
    loginLayout->addWidget(loginBtn);

    loginLayout->addSpacing(15);

    QHBoxLayout* switchLayout = new QHBoxLayout();
    QLabel* noAccountLabel = new QLabel("还没有账号？");
    noAccountLabel->setStyleSheet("color: #666; font-size: 14px; background: transparent;");
    toRegisterBtn = new QPushButton("立即注册");
    toRegisterBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #667eea; "
        "font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { color: #764ba2; text-decoration: underline; }");
    connect(toRegisterBtn, &QPushButton::clicked, this, &LoginDialog::switchToRegister);
    switchLayout->addStretch();
    switchLayout->addWidget(noAccountLabel);
    switchLayout->addWidget(toRegisterBtn);
    switchLayout->addStretch();
    loginLayout->addLayout(switchLayout);

    loginLayout->addStretch();

    QLabel* tipLabel = new QLabel("测试账号: 2024001 / 123456");
    tipLabel->setStyleSheet("color: #888; font-size: 13px; background: transparent;");
    tipLabel->setAlignment(Qt::AlignCenter);
    loginLayout->addWidget(tipLabel);

    /* 注册页面 */
    registerPage = new QWidget();
    registerPage->setStyleSheet("background-color: white; border-radius: 16px;");
    QVBoxLayout* regLayout = new QVBoxLayout(registerPage);
    regLayout->setSpacing(6);
    regLayout->setContentsMargins(45, 30, 45, 30);

    QLabel* regTitle = new QLabel("新用户注册");
    regTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #333; background: transparent;");
    regTitle->setAlignment(Qt::AlignCenter);
    regLayout->addWidget(regTitle);
    regLayout->addSpacing(15);

    // 注册页面的输入框样式（稍小一点以适应更多字段）
    QString regInputStyle =
        "QLineEdit { "
        "   padding: 12px 15px; "
        "   border: 2px solid #D0D0D0; "
        "   border-radius: 8px; "
        "   font-size: 16px; "
        "   font-family: 'Microsoft YaHei', 'Arial'; "
        "   background-color: #FAFAFA; "
        "   color: #222222; "
        "   letter-spacing: 1px; "
        "}"
        "QLineEdit:focus { border-color: #667eea; background-color: white; }"
        "QLineEdit::placeholder { color: #999; font-size: 14px; }";

    QString regLabelStyle = "font-size: 14px; color: #333; font-weight: 600; background: transparent;";

    QLabel* regIdLabel = new QLabel("学号/工号");
    regIdLabel->setStyleSheet(regLabelStyle);
    regLayout->addWidget(regIdLabel);
    regUserIdEdit = new QLineEdit();
    regUserIdEdit->setPlaceholderText("请输入学号或工号");
    regUserIdEdit->setStyleSheet(regInputStyle);
    regUserIdEdit->setMinimumHeight(48);
    regLayout->addWidget(regUserIdEdit);

    QLabel* regNameLabel = new QLabel("姓名");
    regNameLabel->setStyleSheet(regLabelStyle);
    regLayout->addWidget(regNameLabel);
    regNameEdit = new QLineEdit();
    regNameEdit->setPlaceholderText("请输入真实姓名");
    regNameEdit->setStyleSheet(regInputStyle);
    regNameEdit->setMinimumHeight(48);
    regLayout->addWidget(regNameEdit);

    QLabel* regPwdLabel = new QLabel("密码");
    regPwdLabel->setStyleSheet(regLabelStyle);
    regLayout->addWidget(regPwdLabel);
    regPasswordEdit = new QLineEdit();
    regPasswordEdit->setPlaceholderText("请设置密码（至少6位）");
    regPasswordEdit->setEchoMode(QLineEdit::Password);
    regPasswordEdit->setStyleSheet(regInputStyle);
    regPasswordEdit->setMinimumHeight(48);
    regLayout->addWidget(regPasswordEdit);

    QLabel* regConfirmLabel = new QLabel("确认密码");
    regConfirmLabel->setStyleSheet(regLabelStyle);
    regLayout->addWidget(regConfirmLabel);
    regConfirmEdit = new QLineEdit();
    regConfirmEdit->setPlaceholderText("请再次输入密码");
    regConfirmEdit->setEchoMode(QLineEdit::Password);
    regConfirmEdit->setStyleSheet(regInputStyle);
    regConfirmEdit->setMinimumHeight(48);
    regLayout->addWidget(regConfirmEdit);

    QLabel* regTypeLabel = new QLabel("身份");
    regTypeLabel->setStyleSheet(regLabelStyle);
    regLayout->addWidget(regTypeLabel);
    regTypeCombo = new QComboBox();
    regTypeCombo->addItem("本科生", Student);
    regTypeCombo->addItem("研究生", Graduate);
    regTypeCombo->addItem("教师", Teacher);
    regTypeCombo->setStyleSheet(
        "QComboBox { "
        "   padding: 12px 15px; "
        "   border: 2px solid #D0D0D0; "
        "   border-radius: 8px; "
        "   background-color: #FAFAFA; "
        "   color: #222; "
        "   font-size: 16px; "
        "}"
        "QComboBox:focus { border-color: #667eea; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView { "
        "   background-color: white; "
        "   color: #333; "
        "   selection-background-color: #667eea; "
        "   selection-color: white; "
        "}");
    regTypeCombo->setMinimumHeight(48);
    regLayout->addWidget(regTypeCombo);

    regLayout->addSpacing(15);

    registerBtn = new QPushButton("注 册");
    registerBtn->setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #11998e, stop:1 #38ef7d); "
        "   color: white; padding: 14px; "
        "   border: none; border-radius: 10px; font-size: 17px; font-weight: bold; }"
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0e8a7d, stop:1 #2ed66c); }");
    registerBtn->setMinimumHeight(52);
    connect(registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    regLayout->addWidget(registerBtn);

    QHBoxLayout* backLayout = new QHBoxLayout();
    QLabel* hasAccountLabel = new QLabel("已有账号？");
    hasAccountLabel->setStyleSheet("color: #666; font-size: 14px; background: transparent;");
    toLoginBtn = new QPushButton("返回登录");
    toLoginBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #667eea; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { color: #764ba2; }");
    connect(toLoginBtn, &QPushButton::clicked, this, &LoginDialog::switchToLogin);
    backLayout->addStretch();
    backLayout->addWidget(hasAccountLabel);
    backLayout->addWidget(toLoginBtn);
    backLayout->addStretch();
    regLayout->addLayout(backLayout);

    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(registerPage);

    mainLayout->addWidget(stackedWidget);
}

void LoginDialog::onLogin() {
    QString userId = loginUserIdEdit->text().trimmed();
    QString password = loginPasswordEdit->text();

    if (userId.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入学号和密码");
        return;
    }

    LibraryManager& mgr = LibraryManager::instance();
    if (mgr.login(userId, password)) {
        emit loginSuccess();
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

void LoginDialog::onRegister() {
    QString userId = regUserIdEdit->text().trimmed();
    QString name = regNameEdit->text().trimmed();
    QString password = regPasswordEdit->text();
    QString confirm = regConfirmEdit->text();
    UserType type = static_cast<UserType>(regTypeCombo->currentData().toInt());

    if (userId.isEmpty() || name.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写完整信息");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "提示", "密码长度至少6位");
        return;
    }

    if (password != confirm) {
        QMessageBox::warning(this, "提示", "两次密码输入不一致");
        return;
    }

    LibraryManager& mgr = LibraryManager::instance();
    if (mgr.registerUser(userId, password, name, type)) {
        QMessageBox::information(this, "成功", "注册成功，请登录");
        switchToLogin();
        loginUserIdEdit->setText(userId);
    } else {
        QMessageBox::warning(this, "注册失败", "该学号已被注册");
    }
}

// 注册页面
void LoginDialog::switchToRegister() {
    stackedWidget->setCurrentWidget(registerPage);
    setFixedSize(520, 750);
}

// 登录页面
void LoginDialog::switchToLogin() {
    stackedWidget->setCurrentWidget(loginPage);
    setFixedSize(520, 680);
}
