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
    void InitializeLibVlc();
    void InitializeConnect();

    void DestoryLibVlc();

    HWND GetDesktopHwnd() const noexcept;

private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *modeSettingsLayout, *videoControlLayout;
    QWidget *modeSettingsWidget, *videoControlWidget;
    QListWidget *videoListWidget;

    QComboBox *modeComboBox;
    QLabel *modeLabel;
    QCheckBox *runAtStartupCheckBox;
    QPushButton *volumeButton, *addVideoButton, *deleteVideoButton, *playOrStopButton;
    QSlider *volumeSlider;

    libvlc_instance_t *vlcInstance;
    libvlc_media_list_t *videoList;
    libvlc_media_list_player_t *videoPlayer;
};
#endif // MAINWINDOW_H
