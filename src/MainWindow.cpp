#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    InitializeUi();         // 初始化UI
    InitializeLibVlc();     // 初始化libvlc
    InitializeConnect();    // 初始化信号/槽
    InitializeSettings();   // 根据配置文件做正式运行前最后的设置
}

MainWindow::~MainWindow()
{
    DestoryLibVlc();
}

void MainWindow::InitializeUi()
{
    setWindowIcon(LogoIcon);

    ///////////////////////////////////////////////////////////

    modeLabel = new QLabel("模式：");
    modeComboBox = new QComboBox;
    modeComboBox->addItems({ "不循环", "列表循环", "单曲循环", "随机播放" });

    aspectRatioLabel = new QLabel("尺寸：");
    aspectRatioComboBox = new QComboBox;
    aspectRatioComboBox->addItems({ "默认", "填充", "适应", "拉伸", "16:9", "4:3" });

    runAtStartupCheckBox = new QCheckBox("开机启动");

    modeSettingsLayout = new QHBoxLayout(this);
    modeSettingsLayout->addWidget(modeLabel);
    modeSettingsLayout->addWidget(modeComboBox);
    modeSettingsLayout->addStretch();
    modeSettingsLayout->addWidget(aspectRatioLabel);
    modeSettingsLayout->addWidget(aspectRatioComboBox);
    modeSettingsLayout->addStretch();
    modeSettingsLayout->addWidget(runAtStartupCheckBox);

    modeSettingsWidget = new QWidget(this);
    modeSettingsWidget->setLayout(modeSettingsLayout);

    ///////////////////////////////////////////////////////////

    addVideoButton = new QToolButton;
    addVideoButton->setIcon(AddVideoButtonIcon);
    addVideoButton->setIconSize(ButtonIconSize);
    addVideoButton->setToolTip("添加");
    
    deleteVideoButton = new QToolButton;
    deleteVideoButton->setIcon(DeleteVideoButtonIcon);
    deleteVideoButton->setIconSize(ButtonIconSize);
    deleteVideoButton->setToolTip("删除");
    deleteVideoButton->setDisabled(true);

    playOrPauseButton = new QToolButton;
    playOrPauseButton->setIcon(PlayButtonIcon);
    playOrPauseButton->setIconSize(ButtonIconSize);
    playOrPauseButton->setToolTip("播放");
    playOrPauseButton->setDisabled(true);

    playPreviousButton = new QToolButton;
    playPreviousButton->setIcon(PlayPreviousButtonIcon);
    playPreviousButton->setIconSize(ButtonIconSize);
    playPreviousButton->setToolTip("上一个");
    playPreviousButton->setDisabled(true);

    stopPlayingButton = new QToolButton;
    stopPlayingButton->setIcon(StopPlayingButtonIcon);
    stopPlayingButton->setIconSize(ButtonIconSize);
    stopPlayingButton->setToolTip("停止");
    stopPlayingButton->setDisabled(true);

    playNextButton = new QToolButton;
    playNextButton->setIcon(PlayNextButtonIcon);
    playNextButton->setIconSize(ButtonIconSize);
    playNextButton->setToolTip("下一个");
    playNextButton->setDisabled(true);

    volumeButton = new QToolButton;
    volumeButton->setIcon(VolumeButtonIcon);
    volumeButton->setIconSize(ButtonIconSize);
    volumeButton->setToolTip("静音");

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setTickInterval(1);
    volumeSlider->setValue(volumeSlider->maximum());

    videoControlLayout = new QHBoxLayout(this);
    videoControlLayout->addWidget(addVideoButton);      // 添加
    videoControlLayout->addWidget(deleteVideoButton);   // 删除
    videoControlLayout->addWidget(playOrPauseButton);    // 开始/暂停
    videoControlLayout->addWidget(playPreviousButton);  // 上一个
    videoControlLayout->addWidget(stopPlayingButton);   // 停止
    videoControlLayout->addWidget(playNextButton);      // 下一个
    videoControlLayout->addStretch();                   // 中间弹性留白
    videoControlLayout->addWidget(volumeButton);        // 静音
    videoControlLayout->addWidget(volumeSlider);        // 音量条

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

    ///////////////////////////////////////////////////////////

    auto *exitAction = new QAction("退出", this);
    connect(exitAction, &QAction::triggered, [=]()
    {
        qApp->quit();
    });

    auto *trayMenu = new QMenu(this);
    trayMenu->addAction(exitAction);

    tray = new QSystemTrayIcon(this);
    tray->setContextMenu(trayMenu);
    tray->setIcon(TrayIcon);
    tray->setToolTip("无正在播放项目");
    tray->show();
}

void MainWindow::InitializeSettings()
{
    QDir dirChecker;
    if (!dirChecker.exists("userdata")) // 确保userdata文件夹存在，以后可以换成std::filesystem
    {
        dirChecker.mkdir("userdata");
    }

    settings = new QSettings("userdata/config.ini", QSettings::IniFormat, this);

    // 开机启动
    if (!settings->contains("RunAtStartup"))    // 如果不存在RunAtStartup，可能是首次运行，则做默认初始化
    {
        settings->setValue("RunAtStartup", false);  // 默认不进行开机启动
    }
    else    // 存在RunAtStartup配置，则根据配置情况执行
    {
        SetRunAtStartup(settings->value("RunAtStartup").toBool());  // 根据配置文件设置是否开机启动
    }

    // 列表播放模式
    if (!settings->contains("PlaybackMode"))    // 如果不存在PlaybackMode，可能是首次运行，则做默认初始化
    {
        settings->setValue("PlaybackMode", "default");  // 默认为“不循环”，即libvlc_playback_mode_default
        modeComboBox->setCurrentText("不循环");
    }
    else    // 存在PlaybackMode配置，则根据配置情况执行
    {
        auto playbackMode = settings->value("PlaybackMode").toString();
        if (playbackMode == "repeat")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_repeat);
            modeComboBox->setCurrentText("单曲循环");
        }
        else if (playbackMode == "loop")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_loop);
            modeComboBox->setCurrentText("列表循环");
        }
        else if (playbackMode == "default")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_default);
            modeComboBox->setCurrentText("不循环");
        }
        else
        {
            // 暂不支持“随机播放”
            modeComboBox->setCurrentText("随机播放");
        }

        // 以上一大段和modeComboBox的信号处理函数重合很大，不太好，需要改进
    }
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

    videoPlayerEventManager = libvlc_media_list_player_event_manager(videoPlayer);
    libvlc_event_attach(videoPlayerEventManager, 
                        libvlc_MediaListPlayerNextItemSet, 
                        [](const struct libvlc_event_t*, void *ptr) 
                        {
                            static_cast<MainWindow*>(ptr)->EmitMediaListPlayerNextItemSet(); 
                        }, 
                        this);
    // 注意：在C库中使用C++的成员函数作为回调函数，可以在C库设置回调函数的函数的void*数据项中传this指针，
    // 然后通过this再调用真正的回调函数，将此C库函数作为此类的友元函数可以减少破坏封装的风险，
    // 这样只需要将此回调函数声明为private，否则则需要将此回调函数作为public
}

void MainWindow::InitializeThumbnailToolBar()
{
    thumbnailToolBar = new QWinThumbnailToolBar(this);
    thumbnailToolBar->setWindow(this->windowHandle());

    playPreviousThumbnailButton = new QWinThumbnailToolButton(thumbnailToolBar);
    playPreviousThumbnailButton->setIcon(PlayPreviousButtonIcon);
    playPreviousThumbnailButton->setToolTip("上一个");
    playPreviousThumbnailButton->setEnabled(true);
    connect(playPreviousThumbnailButton, &QWinThumbnailToolButton::clicked, [=]()
    {
        libvlc_media_list_player_previous(videoPlayer);
    });

    playOrPauseThumbnailButton = new QWinThumbnailToolButton(thumbnailToolBar);
    playOrPauseThumbnailButton->setIcon(PlayButtonIcon);
    playOrPauseThumbnailButton->setToolTip("开始");
    playOrPauseThumbnailButton->setEnabled(true);
    connect(playOrPauseThumbnailButton, &QWinThumbnailToolButton::clicked, this, &MainWindow::OnPlayOrPauseClicked);

    playNextThumbnailButton = new QWinThumbnailToolButton(thumbnailToolBar);
    playNextThumbnailButton->setIcon(PlayNextButtonIcon);
    playNextThumbnailButton->setToolTip("下一个");
    playNextThumbnailButton->setEnabled(true);
    connect(playNextThumbnailButton, &QWinThumbnailToolButton::clicked, [=]()
    {
        libvlc_media_list_player_next(videoPlayer);
    });

    thumbnailToolBar->addButton(playPreviousThumbnailButton);
    thumbnailToolBar->addButton(playOrPauseThumbnailButton);
    thumbnailToolBar->addButton(playNextThumbnailButton);
}

void MainWindow::DestoryLibVlc() noexcept
{
    libvlc_media_list_player_release(videoPlayer);
    libvlc_media_list_release(videoList);
    libvlc_release(vlcInstance);
}

void MainWindow::InitializeConnect()
{
    // 添加视频
    connect(addVideoButton, &QToolButton::clicked, [=]()
    {
        auto fileNames = QFileDialog::getOpenFileNames(this, "选择媒体文件");
        if (!fileNames.empty())
        {
            for (const auto &fileName : fileNames)
            {
                // 解决对于路径Qt使用/而libvlc只认本地规则（即\\）的问题，path即为转换后的路径
                const auto path = QDir::toNativeSeparators(fileName);

                // 当列表中没有重复项时才允许添加进去，这里做了个去重
                if (videoListWidget->findItems(path, Qt::MatchFixedString | Qt::MatchCaseSensitive).empty())
                {
                    // 先转为std::string，然后再使用c_str()取得const char*
                    // 注意不要连写path = toStdString().c_str()，当心变量生命周期
                    const auto stdStringPath = path.toStdString();
                    auto *videoPath = libvlc_media_new_path(vlcInstance, stdStringPath.c_str());
                    videoListWidget->addItem(path);

                    libvlc_media_list_lock(videoList);
                    libvlc_media_list_add_media(videoList, videoPath);
                    libvlc_media_list_unlock(videoList);

                    libvlc_media_release(videoPath);
                }
            }

            emit VideoListCountChanged(videoListWidget->count());
        }
    });

    // 播放/暂停视频
    connect(playOrPauseButton, &QToolButton::clicked, this, &MainWindow::OnPlayOrPauseClicked);

    // 播放上一个
    connect(playPreviousButton, &QToolButton::clicked, [=]()
    {
        libvlc_media_list_player_previous(videoPlayer);
    });

    // 播放下一个
    connect(playNextButton, &QToolButton::clicked, [=]()
    {
        libvlc_media_list_player_next(videoPlayer);
    });

    // 停止播放
    connect(stopPlayingButton, &QToolButton::clicked, [=]()
    {
        libvlc_media_list_player_stop(videoPlayer);

        tray->setToolTip("无正在播放项目");    // 停下来了自然没有项目正在播放
    });

    // 静音按钮
    connect(volumeButton, &QToolButton::clicked, [=]()
    {
        auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
        const auto isMute = libvlc_audio_get_mute(player);
        libvlc_audio_set_mute(player, !isMute);
        libvlc_media_player_release(player);


        if (isMute) // 如果原先是静音状态，那么点击此按钮时将有声音
        {
            volumeButton->setIcon(QIcon(":/icons/volume-down-fill.png"));
        }
        else        // 如果原先不是静音状态，那么点击此按钮时将静音
        {
            volumeButton->setIcon(QIcon(":/icons/volume-mute-fill.png"));
        }
    });

    // 音量条
    connect(volumeSlider, &QSlider::sliderMoved, [=](int value)
    {
        auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
        libvlc_audio_set_volume(player, value);
        libvlc_media_player_release(player);

        if (value == 0) // 音量为0时记得将换成静音的图标
        {
            volumeButton->setIcon(QIcon(":/icons/volume-mute-fill.png"));
        }
        else            // 音量不为0时则换成有声音的图标（暂不处理音量小于0的情况，因为正常操作并不会出现）
        {
            volumeButton->setIcon(QIcon(":/icons/volume-down-fill.png"));
        }
    });

    // 选择列表循环播放的模式
    connect(modeComboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [=](const QString &currentText)
    {
        if (currentText == "单曲循环")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_repeat);
            settings->setValue("PlaybackMode", "repeat");
        }
        else if (currentText == "列表循环")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_loop);
            settings->setValue("PlaybackMode", "loop");
        }
        else if (currentText == "不循环")
        {
            libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_default);
            settings->setValue("PlaybackMode", "default");
        }
        else
        {
            // 暂不支持“随机播放”
        }
    });

    // 删除按钮
    connect(deleteVideoButton, &QToolButton::clicked, [=]()
    {
        /************lock*************/
        libvlc_media_list_lock(videoList);

        const auto currentIndexOfSongList = GetCurrentItemIndex();  // 先保存一下当前编号，一会列表就改了

        // 如果删除的是当前播放的项目，则直接播放下一个或者停下来，不然即使删除了该项目也会继续播放
        if (videoListWidget->currentItem()->text() == GetCurrentItemName())
        {
            if (Q_UNLIKELY(videoListWidget->count() == 1))  // 全都删完了就自动停下来
            {                                               // 判等于1是因为此时还未对videoListWidget进行删除操作
                libvlc_media_list_player_stop(videoPlayer);

                tray->setToolTip("无正在播放项目");    // 停下来了自然没有项目正在播放
            }
            else
            {
                libvlc_media_list_player_next(videoPlayer);
            }
        }
        libvlc_media_list_remove_index(videoList, currentIndexOfSongList);  // 在播放列表中移除此项

        libvlc_media_list_unlock(videoList);
        /*************unlock*************/

        delete videoListWidget->takeItem(videoListWidget->currentRow());    // takeItem返回的指针需要手动释放

        emit VideoListCountChanged(videoListWidget->count());   // 发射信号通知列表项已发生改变
    });

    // 只有选中播放列表中的项目才允许删除，防止误触
    connect(videoListWidget, &QListWidget::itemClicked, [=]()
    {
        deleteVideoButton->setEnabled(true);
    });

    // 双击列表中的某项即可播放此项
    connect(videoListWidget, &QListWidget::itemDoubleClicked, [=]()
    {
        const auto currentItemName = videoListWidget->currentItem()->text();
        const auto count = libvlc_media_list_count(videoList);
        for (int i = 0; i < count; ++i)
        {
            if Q_UNLIKELY(currentItemName == GetItemNameAtIndex(i)) // 通过名字寻找要播放的项目的编号
            {
                libvlc_media_list_player_play_item_at_index(videoPlayer, i);

                break;
            }
        }
    });

    // 注册是否开机启动
    connect(runAtStartupCheckBox, &QCheckBox::stateChanged, [=](int state)
    {
        // 勾选此框 -> Qt::Checked -> 设置开机启动
        // 否则同理
        SetRunAtStartup(state == Qt::Checked);
    });

    connect(aspectRatioComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), 
        [=](const QString &ratioText)
        {
            auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
            if (ratioText == "默认")      // 同“适应”
            {
                libvlc_video_set_aspect_ratio(player, nullptr);
            }
            else if (ratioText == "填充") // 图片等比缩放，优先适应最小边
            {
                if (libvlc_media_list_player_is_playing(videoPlayer))
                {
                    unsigned int videoWidth = 0, videoHeight = 0;
                    libvlc_video_get_size(player, 0, &videoWidth, &videoHeight);

                    const auto width = GetSystemMetrics(SM_CXSCREEN);   // 不能是SM_CXFULLSCREEN
                    const auto height = GetSystemMetrics(SM_CYSCREEN);  // 不能是SM_CYFULLSCREEN
                    if (width < height)
                    {
                        const double scale = 1.0 * width / videoWidth;
                        videoHeight = static_cast<unsigned int>(1.0 * videoHeight * scale);
                        videoWidth = width;
                    }
                    else
                    {
                        const double scale = 1.0 * height / videoHeight;
                        videoWidth = static_cast<unsigned int>(1.0 * videoWidth * scale);
                        videoHeight = height;
                    }

                    const auto gcd = std::gcd(videoWidth, videoHeight);
                    const auto ratioString = std::to_string(videoWidth / gcd) + ':' + std::to_string(videoHeight / gcd);
                    libvlc_video_set_aspect_ratio(player, ratioString.c_str());
                }
                else
                {
                    QMessageBox::information(this, "提示", "在没有正在播放的项目时此选项无效");
                }
            }
            else if (ratioText == "适应") // 图片等比缩放，保持图片比例的同时最大化显示图片
            {
                libvlc_video_set_aspect_ratio(player, nullptr);
            }
            else if (ratioText == "拉伸") // 图片根据屏幕显示分辨率拉伸，让一张图片就占满桌面
            {
                const auto width = GetSystemMetrics(SM_CXSCREEN);   // 不能是SM_CXFULLSCREEN
                const auto height = GetSystemMetrics(SM_CYSCREEN);  // 不能是SM_CYFULLSCREEN
                const auto gcd = std::gcd(width, height);

                const auto ratioString = std::to_string(width / gcd) + ':' + std::to_string(height / gcd);
                libvlc_video_set_aspect_ratio(player, ratioString.c_str());
            }
            else
            {
                const auto ratioTextStdString = ratioText.toStdString();
                libvlc_video_set_aspect_ratio(player, ratioTextStdString.c_str());
            }

            libvlc_media_player_release(player);
        });

    // 来自托盘的信号的处理
    connect(tray, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger) // 单击即显示窗口
        {
            this->showNormal();
        }
    });

    // 当列表项变化之时应该做的处理
    connect(this, &MainWindow::VideoListCountChanged, [=](int count)
    {
        static bool isThumbnainToolBarInitialized = false;
        if (!isThumbnainToolBarInitialized)
        {
            InitializeThumbnailToolBar();
            isThumbnainToolBarInitialized = true;
        }

        if (count)  // 有媒体可以播放，于是这些按钮变为可用
        {
            playOrPauseButton->setEnabled(true);
            deleteVideoButton->setEnabled(true);
            playPreviousButton->setEnabled(true);
            stopPlayingButton->setEnabled(true);
            playNextButton->setEnabled(true);

            playOrPauseThumbnailButton->setEnabled(true);
            playNextThumbnailButton->setEnabled(true);
            playPreviousThumbnailButton->setEnabled(true);
        }
        else        // 没有媒体可以播放，于是这些按钮变为不可用
        {
            playOrPauseButton->setDisabled(true);
            deleteVideoButton->setDisabled(true);
            playPreviousButton->setDisabled(true);
            stopPlayingButton->setDisabled(true);
            playNextButton->setDisabled(true);

            playOrPauseThumbnailButton->setEnabled(false);
            playNextThumbnailButton->setEnabled(false);
            playPreviousThumbnailButton->setEnabled(false);
        }
    });

    // 当前播放的项目发生了变化
    connect(this, &MainWindow::MediaListPlayerNextItemSet, [=]()
    {
        tray->setToolTip("正在播放 - " + QFileInfo(GetCurrentItemName()).fileName());   // 托盘提示改为当前播放项的名字
    });
}

// 点击窗体右上角关闭按钮时会触发的事件，若已有配置则根据配置执行，若没有配置则弹框询问动作
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (settings->contains("CloseClickedAction"))   // 如果已经写入了配置文件，则根据配置文件来决定行为
    {
        const auto action = settings->value("CloseClickedAction").toString();
        if (action == "background")
        {
            event->ignore();
            this->hide();
        }
        else if (action == "exit")
        {
            qApp->quit();
        }
        else
        {
            // 配置文件出错
        }
    }
    else    // 还没写配置文件，则弹框询问
    {
        OnExitDialog dialog;
        const auto result = dialog.exec();
        if (result == OnExitDialog::BackgroundWithoutSaving)    // 后台运行且不保存配置
        {
            event->ignore();
            this->hide();
        }
        else if (result == OnExitDialog::BackgroundAndSave)     // 后台运行且保存配置
        {
            event->ignore();
            this->hide();

            settings->setValue("CloseClickedAction", "background");
        }
        else if (result == OnExitDialog::ExitWithoutSaving)     // 直接退出且不保存配置
        {
            qApp->quit();
        }
        else if (result == OnExitDialog::ExitAndSave)           // 直接退出且保存配置
        {
            qApp->quit();

            settings->setValue("CloseClickedAction", "exit");
        }
        else if (result == OnExitDialog::Cancel)                // 取消操作
        {
            event->ignore();    // 取消关闭窗口的操作
        }
        else
        {
            event->ignore();    // 用户直接点击了X
        }
    }
}

// 当点击开始/暂停按钮时的动作，需要同步主界面上和ThumbnailToolBar上按钮的状态
void MainWindow::OnPlayOrPauseClicked() noexcept
{
    if (!libvlc_media_list_player_is_playing(videoPlayer))  // 连第一遍播放都还没开始的，先让其播放
    {
        libvlc_media_list_player_play(videoPlayer);

        playOrPauseButton->setIcon(PauseButtonIcon);
        playOrPauseButton->setToolTip("暂停");

        playOrPauseThumbnailButton->setIcon(PauseButtonIcon);
        playOrPauseThumbnailButton->setToolTip("暂停");
    }
    else // 已经开始播放的
    {
        libvlc_media_list_player_pause(videoPlayer);

        playOrPauseButton->setIcon(PlayButtonIcon);
        playOrPauseButton->setToolTip("播放");

        playOrPauseThumbnailButton->setIcon(PlayButtonIcon);
        playOrPauseThumbnailButton->setToolTip("播放");
    }
}

void MainWindow::EmitMediaListPlayerNextItemSet() noexcept
{
    emit MediaListPlayerNextItemSet();
}

HWND MainWindow::GetDesktopHwnd() const noexcept
{
    auto hWnd = FindWindow(L"Progman", L"Program Manager");
    SendMessageTimeout(hWnd, 0x52C, 0, 0, SMTO_NORMAL, 1000, nullptr);	// 不知道是否可以为空指针

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

// 获取正在播放的项目的名字，使用Windows表示法
QString MainWindow::GetCurrentItemName() noexcept
{
    auto *player = libvlc_media_list_player_get_media_player(videoPlayer);  // 需要手动释放
    auto *media = libvlc_media_player_get_media(player);                    // 需要手动释放
    auto *rawName = libvlc_media_get_mrl(media);                            // 需要手动释放（吗？）

    const auto currentName = QDir::toNativeSeparators(
                             QString::fromUtf8(
                             QByteArray::fromPercentEncoding(rawName + 8))); // + 8是为了跳过开头的file:///

    //std::free(rawName);   // 为什么会出错？
    libvlc_media_release(media);
    libvlc_media_player_release(player);

    return currentName;
}

// 根据获取位于播放列表index出的项目的名字
QString MainWindow::GetItemNameAtIndex(int index) noexcept
{
    libvlc_media_list_lock(videoList);

    auto *media = libvlc_media_list_item_at_index(videoList, index);
    auto *rawName = libvlc_media_get_mrl(media);

    libvlc_media_release(media);

    libvlc_media_list_unlock(videoList);

    return QDir::toNativeSeparators(
           QString::fromUtf8(
           QByteArray::fromPercentEncoding(rawName + 8)));;
}

// 获取正在播放的项目在播放列表中的index
int MainWindow::GetCurrentItemIndex() noexcept
{
    auto *player = libvlc_media_list_player_get_media_player(videoPlayer);  // 需要手动释放
    auto *media = libvlc_media_player_get_media(player);

    const int index = libvlc_media_list_index_of_item(videoList, media);

    libvlc_media_player_release(player);
    libvlc_media_release(media);

    return index;
}

// 设置是否开机启动
// doSetting: true -> 开机启动，false -> 开机不启动
void MainWindow::SetRunAtStartup(bool doSetting) const noexcept
{
    QSettings
        Reg(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
            QSettings::NativeFormat);
    if (doSetting)
    {
        // 给后面的值加上双引号是为了符合Windows默认的规范
        Reg.setValue("VideoWallpaper",
                     "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"");
        settings->setValue("RunAtStartup", true);
    }
    else
    {
        Reg.remove("VideoWallpaper");
        settings->setValue("RunAtStartup", false);
    }

    runAtStartupCheckBox->setChecked(doSetting);    // 更新一下复选框的勾选状态
}
