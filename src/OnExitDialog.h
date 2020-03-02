#ifndef ONEXITDIALOG_H
#define ONEXITDIALOG_H

#pragma execution_character_set("utf-8")

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QWidget>
#include <QPushButton>

#include "Constants.h"

// 当点击关闭按钮时会出现的对话框，但若配置已保存则不会出现
class OnExitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OnExitDialog(QWidget *parent = nullptr);

private:
    void InitializeUi();
    void InitializeConnect();

public:
    enum Result
    {
        Dummy = 27,                 // 仅是为了占位，让后面的枚举值和QDialog中定义的不重复
        BackgroundWithoutSaving,    // 后台运行且不保存配置
        BackgroundAndSave,          // 后台运行且保存配置
        ExitWithoutSaving,          // 直接退出且不保存配置
        ExitAndSave,                // 直接退出且保存配置
        Cancel,                     // 取消操作
    };

private:
    QVBoxLayout *layout;
    QCheckBox *saveConfigCheckbox;
    QWidget *buttonsBox;
    QHBoxLayout *buttonsBoxLayout;
    QPushButton *backgroundButton, *exitButton, *cancelButton;
};

#endif // ONEXITDIALOG_H
