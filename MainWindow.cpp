#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    InitializeUi();
    InitializeLibVlc();
    InitializeConnect();
}

MainWindow::~MainWindow()
{
    DestoryLibVlc();
}

void MainWindow::InitializeUi()
{
    modeLabel = new QLabel("模式：");
    modeComboBox = new QComboBox;
    modeComboBox->addItems({ "不循环", "列表循环", "单曲循环", "随机播放" });

    runAtStartupCheckBox = new QCheckBox("开机启动");

    modeSettingsLayout = new QHBoxLayout(this);
    modeSettingsLayout->addWidget(modeLabel);
    modeSettingsLayout->addWidget(modeComboBox);
    modeSettingsLayout->addStretch();
    modeSettingsLayout->addWidget(runAtStartupCheckBox);

    modeSettingsWidget = new QWidget(this);
    modeSettingsWidget->setLayout(modeSettingsLayout);

    ///////////////////////////////////////////////////////////

    addVideoButton = new QPushButton("添加");

    deleteVideoButton = new QPushButton("删除");
    deleteVideoButton->setDisabled(true);

    playOrStopButton = new QPushButton("开始/暂停");

    volumeButton = new QPushButton("静音");

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setTickInterval(1);

    videoControlLayout = new QHBoxLayout(this);
    videoControlLayout->addWidget(addVideoButton);
    videoControlLayout->addWidget(deleteVideoButton);
    videoControlLayout->addWidget(playOrStopButton);
    videoControlLayout->addStretch();
    videoControlLayout->addWidget(volumeButton);
    videoControlLayout->addWidget(volumeSlider);

    videoControlWidget = new QWidget(this);
    videoControlWidget->setLayout(videoControlLayout);

    ///////////////////////////////////////////////////////////

    videoListWidget = new QListWidget(this);

    ///////////////////////////////////////////////////////////

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(modeSettingsWidget);
    mainLayout->addWidget(videoControlWidget);
    mainLayout->addWidget(videoListWidget);

    setLayout(mainLayout);
}

void MainWindow::InitializeLibVlc()
{
    vlcInstance = libvlc_new(0, nullptr);
    videoList = libvlc_media_list_new(vlcInstance);
    videoPlayer = libvlc_media_list_player_new(vlcInstance);

    libvlc_media_list_player_set_media_list(videoPlayer, videoList);

    auto *innerPlayer = libvlc_media_list_player_get_media_player(videoPlayer);
    libvlc_media_player_set_hwnd(innerPlayer, GetDesktopHwnd());
    libvlc_media_player_release(innerPlayer);
}

void MainWindow::DestoryLibVlc()
{
    libvlc_media_list_player_release(videoPlayer);
    libvlc_media_list_release(videoList);
    libvlc_release(vlcInstance);
}

void MainWindow::InitializeConnect()
{
    // 添加视频
    connect(addVideoButton, &QPushButton::clicked, [=]()
        {
            auto fileNames = QFileDialog::getOpenFileNames(this, "选择媒体文件");
            if (!fileNames.empty())
            {
                for (const auto &r : fileNames)
                {
                    // 解决对于路径Qt使用/而libvlc只认本地规则（即\\）的问题，path即为转换后的路径
                    // 注意不要连写path = toStdString().c_str()，当心变量生命周期
                    const auto path = QDir::toNativeSeparators(r).toStdString();  
                    auto *videoPath = libvlc_media_new_path(vlcInstance, path.c_str());
                    videoListWidget->addItem(path.c_str());

                    libvlc_media_list_lock(videoList);
                    libvlc_media_list_add_media(videoList, videoPath);
                    libvlc_media_list_unlock(videoList);

                    libvlc_media_release(videoPath);
                }
            }
        }
    );

    // 播放/暂停视频
    connect(playOrStopButton, &QPushButton::clicked, [=]()
        {
            if (!libvlc_media_list_player_is_playing(videoPlayer))  // 连第一遍播放都还没开始的，先让其播放
            {
                libvlc_media_list_player_play(videoPlayer);
            }
            else // 已经开始播放的
            {
                // 根据文档，只需要调用这个函数就好，它会自动判断该暂停播放还是恢复播放
                libvlc_media_list_player_pause(videoPlayer);
            }
        }
    );

    // 静音按钮
    connect(volumeButton, &QPushButton::clicked, [=]()
        {
            auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
            libvlc_audio_set_mute(player, !libvlc_audio_get_mute(player));
            libvlc_media_player_release(player);
        });

    // 音量条
    connect(volumeSlider, &QSlider::sliderMoved, [=](int value)
        {
            auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
            libvlc_audio_set_volume(player, value);
            libvlc_media_player_release(player);
        });

    // 选择列表循环播放的模式
    connect(modeComboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [=](const QString &currentText)
        {
            if (currentText == "单曲循环")
            {
                libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_repeat);
            }
            else if (currentText == "列表循环")
            {
                libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_loop);
            }
            else if (currentText == "不循环")
            {
                libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_default);
            }
            else
            {
                // 暂不支持“随机播放”
            }
        });

    // 删除按钮
    connect(deleteVideoButton, &QPushButton::clicked, [=]()
        {
            const auto index = videoListWidget->currentRow();
            delete videoListWidget->takeItem(index);    // takeItem返回的指针需要手动释放

            libvlc_media_list_lock(videoList);
            libvlc_media_list_remove_index(videoList, index);
            libvlc_media_list_unlock(videoList);
        });

    // 只有点击过播放列表中的项目才允许删除，防止误触
    connect(videoListWidget, &QListWidget::itemClicked, [=]()
        {
            deleteVideoButton->setEnabled(true);
        });

    // 注册是否开机启动
    connect(runAtStartupCheckBox, &QCheckBox::stateChanged, [=](int state)
        {
            QSettings
                Reg(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
                    QSettings::NativeFormat);
            if (state == Qt::Checked)
            {
                // 给后面的值加上双引号是为了符合Windows默认的规范
                Reg.setValue("VideoWallpaper",
                             "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"");
            }
            else
            {
                Reg.remove("VideoWallpaper");
            }
        });
}

HWND MainWindow::GetDesktopHwnd() const noexcept
{
    auto hWnd = FindWindow(L"Progman", L"Program Manager");
    SendMessageTimeout(hWnd, 0x52C, 0, 0, SMTO_NORMAL, 1000, nullptr);	// can the last param be nullptr?

    HWND hWndWorkW = nullptr;
    do
    {
        hWndWorkW = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
        if (hWndWorkW)
        {
            if (FindWindowEx(hWndWorkW, nullptr, L"SHELLDLL_DefView", nullptr))
            {
                auto h = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
                while (h)
                {
                    SendMessage(h, WM_CLOSE, 0, 0);
                    h = FindWindowEx(nullptr, hWndWorkW, L"WorkerW", nullptr);
                }

                break;
            }
        }
    } while (true);

    return hWnd;
}
