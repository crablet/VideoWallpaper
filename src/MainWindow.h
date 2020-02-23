#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QtGlobal>
#include <QSettings>
#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>

#include <vlc/vlc.h>

#include <Windows.h>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void InitializeUi();
    void InitializeSettings();
    void InitializeLibVlc();
    void InitializeConnect();

    void DestoryLibVlc() noexcept;

    HWND GetDesktopHwnd() const noexcept;

    // 设置是否开机启动
    // doSetting: true -> 开机启动，false -> 开机不启动
    void SetRunAtStartup(bool doSetting) const noexcept;

    void closeEvent(QCloseEvent *event) override;

signals:
    void VideoListCountChanged(int count);  // 当列表控件中的项目个数发生变化时会触发此信号，处理函数可以用来控制按钮的可使用性等等

private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *modeSettingsLayout, *videoControlLayout;
    QWidget *modeSettingsWidget, *videoControlWidget;
    QListWidget *videoListWidget;

    QComboBox *modeComboBox;
    QLabel *modeLabel;
    QCheckBox *runAtStartupCheckBox;
    QPushButton *volumeButton, *addVideoButton, *deleteVideoButton;
    QPushButton *playOrStopButton, *playNextButton, *stopPlayingButton, *playPreviousButton;
    QSlider *volumeSlider;

    QSystemTrayIcon *tray;

    QSettings *settings;

    libvlc_instance_t *vlcInstance;
    libvlc_media_list_t *videoList;
    libvlc_media_list_player_t *videoPlayer;
};
#endif // MAINWINDOW_H
