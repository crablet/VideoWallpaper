#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma execution_character_set("utf-8")

#include <cstdlib>  // std::free
#include <string>   // std::string::c_str, std::to_string
#include <cmath>    // std::gcd

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QCheckBox>
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
#include <QPoint>
#include <QAction>
#include <QCloseEvent>
#include <QToolButton>
#include <QSize>
#include <QString>
#include <QByteArray>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#include <vlc/vlc.h>

#include "WindowsTools.h"
#include "Constants.h"
#include "OnExitDialog.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() noexcept;

private:
    void InitializeUi();
    void InitializeSettings();
    void InitializeLibVlc();
    void InitializeConnect();
    void InitializeThumbnailToolBar();

    void DestoryLibVlc() noexcept;

    QString GetCurrentItemName() noexcept;          // 获取正在播放的项目的名字，使用Windows表示法
    QString GetItemNameAtIndex(int index) noexcept; // 获取位于播放列表index处的项目的名字
    int GetCurrentItemIndex() noexcept;             // 获取正在播放的项目在播放列表中的index

    // 设置是否开机启动
    // doSetting: true -> 开机启动，false -> 开机不启动
    void SetRunAtStartup(bool doSetting) const noexcept;

    void closeEvent(QCloseEvent *event) override;

    // 保存媒体列表到文件中
    void SaveVideoList() noexcept;

    void SaveVideoListAndQuitApp() noexcept;

    // 从配置文件中读取媒体列表
    void ReadVideoList() noexcept;

    void EmitMediaListPlayerNextItemSet() noexcept;

    // 此函数声明成友元是为了user_data中能传this指针然后通过this访问private的EmitMediaListPlayerNextItemSet
    friend int libvlc_event_attach(libvlc_event_manager_t *p_event_manager,
                                   libvlc_event_type_t i_event_type,
                                   libvlc_callback_t f_callback,
                                   void *user_data);

signals:
    void VideoListCountChanged(int count);  // 当列表控件中的项目个数发生变化时会触发此信号，处理函数可以用来控制按钮的可使用性等等
    void MediaListPlayerNextItemSet();      // 正在播放的项目切换时会发出的信号
    void ShouldInitializeThumbnailToolBar();

public slots:
    // 当点击开始/暂停按钮时的动作，需要同步主界面上和ThumbnailToolBar上按钮的状态
    void OnPlayOrPauseClicked() noexcept;

    void AddVideo() noexcept;
    void DeleteVideo() noexcept;

    void SetAspectRatio(const QString &ratioText) noexcept;
    void SetPlaybackMode(const QString &mode) noexcept;
    void SetVolume(int value) noexcept;

private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *modeSettingsLayout, *videoControlLayout;
    QWidget *modeSettingsWidget, *videoControlWidget;
    QListWidget *videoListWidget;

    QComboBox *modeComboBox, *aspectRatioComboBox;
    QLabel *modeLabel, *aspectRatioLabel;
    QCheckBox *runAtStartupCheckBox;
    QToolButton *volumeButton, *addVideoButton, *deleteVideoButton;
    QToolButton *playOrPauseButton, *playNextButton, *stopPlayingButton, *playPreviousButton;
    QSlider *volumeSlider;

    QSystemTrayIcon *tray;

    QWinThumbnailToolBar *thumbnailToolBar;
    QWinThumbnailToolButton *playOrPauseThumbnailButton, *playNextThumbnailButton, *playPreviousThumbnailButton;

    QSettings *settings;

    libvlc_instance_t *vlcInstance;
    libvlc_media_list_t *videoList;
    libvlc_media_list_player_t *videoPlayer;
    libvlc_event_manager_t *videoPlayerEventManager;
};
#endif // MAINWINDOW_H
