#include "OnExitDialog.h"

OnExitDialog::OnExitDialog(QWidget *parent) : QDialog(parent)
{
    InitializeUi();
    InitializeConnect();
}

void OnExitDialog::InitializeUi()
{
    setWindowIcon(LogoIcon);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // 去掉问号标志

    ////////////////////////////////////////////////////////

    saveConfigCheckbox = new QCheckBox("记住我的选择", this);

    ////////////////////////////////////////////////////////

    backgroundButton = new QPushButton("后台运行");
    exitButton = new QPushButton("退出程序");
    cancelButton = new QPushButton("取消");

    buttonsBoxLayout = new QHBoxLayout(this);
    buttonsBoxLayout->addWidget(backgroundButton);
    buttonsBoxLayout->addWidget(exitButton);
    buttonsBoxLayout->addStretch();
    buttonsBoxLayout->addWidget(cancelButton);

    buttonsBox = new QWidget(this);
    buttonsBox->setLayout(buttonsBoxLayout);

    ////////////////////////////////////////////////////////

    layout = new QVBoxLayout(this);
    layout->addWidget(saveConfigCheckbox);
    layout->addWidget(buttonsBox);
}

void OnExitDialog::InitializeConnect()
{
    connect(backgroundButton, &QPushButton::clicked, [=]()
    {
        if (saveConfigCheckbox->isChecked())
        {
            this->done(BackgroundAndSave);
        }
        else
        {
            this->done(BackgroundWithoutSaving);
        }
    });

    connect(exitButton, &QPushButton::clicked, [=]()
    {
        if (saveConfigCheckbox->isChecked())
        {
            this->done(ExitAndSave);
        }
        else
        {
            this->done(ExitWithoutSaving);
        }
    });

    connect(cancelButton, &QPushButton::clicked, [=]()
    {
        this->done(Cancel);
    });
}
