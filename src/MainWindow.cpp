#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    InitializeUi();         // 初始化UI
    InitializeLibVlc();     // 初始化libvlc
    InitializeConnect();    // 初始化信号/槽
    InitializeSettings();   // 根据配置文件做正式运行前最后的设置
}

MainWindow::~MainWindow() noexcept
{
    DestoryLibVlc();
}

void MainWindow::InitializeUi()
{
    setWindowIcon(LogoIcon);

    ///////////////////////////////////////////////////////////

    modeLabel = new QLabel(ModeLabelText);
    modeComboBox = new QComboBox;
    modeComboBox->addItems({ "不循环", "列表循环", "单曲循环", "随机播放" });
    modeComboBox->setStyleSheet(ComboBoxStyle);

    aspectRatioLabel = new QLabel(AspectRatioLabelText);
    aspectRatioComboBox = new QComboBox;
    aspectRatioComboBox->addItems({ "默认", "填充", "适应", "拉伸", "16:9", "4:3" });
    aspectRatioComboBox->setStyleSheet(ComboBoxStyle);

    runAtStartupCheckBox = new QCheckBox(RunAtStartupCheckBoxText);
    runAtStartupCheckBox->setLayoutDirection(Qt::RightToLeft);

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
    addVideoButton->setToolTip(AddVideoButtonText);
    
    deleteVideoButton = new QToolButton;
    deleteVideoButton->setIcon(DeleteVideoButtonIcon);
    deleteVideoButton->setIconSize(ButtonIconSize);
    deleteVideoButton->setToolTip(DeleteVideoButtonText);
    deleteVideoButton->setDisabled(true);

    playOrPauseButton = new QToolButton;
    playOrPauseButton->setIcon(PlayButtonIcon);
    playOrPauseButton->setIconSize(ButtonIconSize);
    playOrPauseButton->setToolTip(PlayOrPauseButtonText_PLAY);
    playOrPauseButton->setDisabled(true);

    playPreviousButton = new QToolButton;
    playPreviousButton->setIcon(PlayPreviousButtonIcon);
    playPreviousButton->setIconSize(ButtonIconSize);
    playPreviousButton->setToolTip(PlayPreviousButtonText);
    playPreviousButton->setDisabled(true);

    stopPlayingButton = new QToolButton;
    stopPlayingButton->setIcon(StopPlayingButtonIcon);
    stopPlayingButton->setIconSize(ButtonIconSize);
    stopPlayingButton->setToolTip(StopPlayingButtonText);
    stopPlayingButton->setDisabled(true);

    playNextButton = new QToolButton;
    playNextButton->setIcon(PlayNextButtonIcon);
    playNextButton->setIconSize(ButtonIconSize);
    playNextButton->setToolTip(PlayNextButtonText);
    playNextButton->setDisabled(true);

    volumeButton = new QToolButton;
    volumeButton->setIcon(VolumeButtonIcon);
    volumeButton->setIconSize(ButtonIconSize);
    volumeButton->setToolTip(VolumeButtonText);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setTickInterval(1);
    volumeSlider->setValue(volumeSlider->maximum());
    volumeSlider->setToolTip(QString::number(volumeSlider->value()));

    videoControlLayout = new QHBoxLayout(this);
    videoControlLayout->addWidget(addVideoButton);      // 添加
    videoControlLayout->addWidget(deleteVideoButton);   // 删除
    videoControlLayout->addWidget(playOrPauseButton);   // 开始/暂停
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
    videoListWidget->setContextMenuPolicy(Qt::CustomContextMenu);   // 设置自定义右键菜单

    ///////////////////////////////////////////////////////////

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(modeSettingsWidget);
    mainLayout->addWidget(videoControlWidget);
    mainLayout->addWidget(videoListWidget);

    setLayout(mainLayout);

    ///////////////////////////////////////////////////////////

    auto *exitAction = new QAction(ExitActionText, this);
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

void MainWindow::InitializeConnect()
{
    // 添加视频
    connect(addVideoButton, &QToolButton::clicked, this, &MainWindow::AddVideo);

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
    connect(volumeSlider, &QSlider::sliderMoved, this, &MainWindow::SetVolume);

    // 选择列表循环播放的模式
    connect(modeComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &MainWindow::SetPlaybackMode);

    // 删除按钮
    connect(deleteVideoButton, &QToolButton::clicked, this, &MainWindow::DeleteVideo);

    // 播放列表的右键删除菜单
    connect(videoListWidget, &QListWidget::customContextMenuRequested, [=](const QPoint &pos)
    {
        auto *menu = new QMenu;
        QAction *action;
        if (videoListWidget->itemAt(pos))   // 如果点击的地方有项目，那就展示“删除”功能
        {
            action = new QAction("删除");
            connect(action, &QAction::triggered, this, &MainWindow::DeleteVideo);
        }
        else                                // 如果点击的地方没项目，那就展示“添加”功能
        {
            action = new QAction("添加");
            connect(action, &QAction::triggered, this, &MainWindow::AddVideo);
        }

        menu->addAction(action);
        menu->exec(QCursor::pos());

        delete action;
        delete menu;
    });

    // 只有选中播放列表中的项目才允许删除，防止误触
    connect(videoListWidget, &QListWidget::itemClicked, [=]()
    {
        deleteVideoButton->setEnabled(true);
    });

    // 双击列表中的某项即可播放此项
    connect(videoListWidget, &QListWidget::itemDoubleClicked, [=]()
    {
        emit ShouldInitializeThumbnailToolBar();

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

    connect(aspectRatioComboBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::SetAspectRatio);

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
        if (count)  // 有媒体可以播放，于是这些按钮变为可用
        {
            playOrPauseButton->setEnabled(true);
            deleteVideoButton->setEnabled(true);
            playPreviousButton->setEnabled(true);
            stopPlayingButton->setEnabled(true);
            playNextButton->setEnabled(true);

            // 这一块总是出现空指针错误，宁愿多判断一次也不调整逻辑了，不然控制流会很混乱
            if (playOrPauseThumbnailButton && playNextThumbnailButton && playPreviousThumbnailButton)
            {
                playOrPauseThumbnailButton->setEnabled(true);
                playNextThumbnailButton->setEnabled(true);
                playPreviousThumbnailButton->setEnabled(true);
            }
        }
        else        // 没有媒体可以播放，于是这些按钮变为不可用
        {
            playOrPauseButton->setDisabled(true);
            deleteVideoButton->setDisabled(true);
            playPreviousButton->setDisabled(true);
            stopPlayingButton->setDisabled(true);
            playNextButton->setDisabled(true);

            // 这一块总是出现空指针错误，宁愿多判断一次也不调整逻辑了，不然控制流会很混乱
            if (playOrPauseThumbnailButton && playNextThumbnailButton && playPreviousThumbnailButton)
            {
                playOrPauseThumbnailButton->setEnabled(false);
                playNextThumbnailButton->setEnabled(false);
                playPreviousThumbnailButton->setEnabled(false);
            }
        }
    });

    // 当前播放的项目发生了变化
    connect(this, &MainWindow::MediaListPlayerNextItemSet, [=]()
    {
        tray->setToolTip("正在播放 - " + QFileInfo(GetCurrentItemName()).fileName());   // 托盘提示改为当前播放项的名字
    });

    connect(this, &MainWindow::ShouldInitializeThumbnailToolBar, [=]()
    {
        static bool isThumbnainToolBarInitialized = false;
        if (!isThumbnainToolBarInitialized)
        {
            InitializeThumbnailToolBar();
            isThumbnainToolBarInitialized = true;

            // 用完就取消掉这个信号槽，既然初始化过了就已经没用了，提升性能
            disconnect(this, &MainWindow::ShouldInitializeThumbnailToolBar, nullptr, nullptr);
        }
    });
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
        const auto playbackMode = settings->value("PlaybackMode").toString();
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

    ReadVideoList();
    if (runAtStartupCheckBox->isChecked() && videoListWidget->count())   // 如果指明需要开机启动且之前有保存播放列表，那么就播放
    {
        libvlc_media_list_player_play(videoPlayer);
    }
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
            SaveVideoListAndQuitApp();
        }
        else
        {
            QMessageBox::information(this, "配置文件出错", "配置文件出错，请检查");
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
            settings->setValue("CloseClickedAction", "background");

            event->ignore();
            this->hide();
        }
        else if (result == OnExitDialog::ExitWithoutSaving)     // 直接退出且不保存配置
        {
            SaveVideoListAndQuitApp();
        }
        else if (result == OnExitDialog::ExitAndSave)           // 直接退出且保存配置
        {
            settings->setValue("CloseClickedAction", "exit");

            SaveVideoListAndQuitApp();
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
    emit ShouldInitializeThumbnailToolBar();

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

// 保存媒体列表到文件中
void MainWindow::SaveVideoList() noexcept
{
    QFile file(VideoListPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "读取失败", "无法打开" + QString(VideoListPath) + "文件，请检查");
    }

    QTextStream textStream(&file);
    textStream.setCodec("UTF-8");   // 这里和读取的时候一样都需要强制UTF-8，注意大小写
    const auto videoListWidgetCount = videoListWidget->count();
    for (int i = 0; i < videoListWidgetCount; ++i)
    {
        textStream << videoListWidget->item(i)->text() << endl;
    }
}

void MainWindow::SaveVideoListAndQuitApp() noexcept
{
    SaveVideoList();

    qApp->quit();
}

// 从配置文件中读取媒体列表
void MainWindow::ReadVideoList() noexcept
{
    if (QFile file(VideoListPath); !file.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        if (file.exists())
        {
            QMessageBox::information(this, "无法读取媒体列表", "无法读取媒体列表，请检查");
        }
    }
    else
    {
        QTextStream textStream(&file);
        textStream.setCodec("UTF-8");   // 这里和保存的时候一样都需要强制UTF-8，注意大小写

        while (!textStream.atEnd())
        {
            QString strRaw;
            textStream >> strRaw;
            if Q_UNLIKELY(strRaw.isEmpty() || strRaw[0].isSpace())    // 处理误读到空格符，换行符的问题
            {
                continue;
            }
            else
            {
                const auto path = strRaw.toStdString();
                auto *videoPath = libvlc_media_new_path(vlcInstance, path.c_str());
                auto *item = new QListWidgetItem(path.c_str());
                item->setToolTip(path.c_str());
                videoListWidget->addItem(item);

                libvlc_media_list_lock(videoList);
                libvlc_media_list_add_media(videoList, videoPath);
                libvlc_media_list_unlock(videoList);

                libvlc_media_release(videoPath);
            }
        }

        if Q_LIKELY(videoListWidget->count())
        {
            playOrPauseButton->setEnabled(true);
            deleteVideoButton->setEnabled(true);
            playPreviousButton->setEnabled(true);
            stopPlayingButton->setEnabled(true);
            playNextButton->setEnabled(true);
        }
    }
}

void MainWindow::EmitMediaListPlayerNextItemSet() noexcept
{
    emit MediaListPlayerNextItemSet();
}

void MainWindow::AddVideo() noexcept
{
    const auto fileNames = QFileDialog::getOpenFileNames(this, "选择媒体文件");
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
                auto *item = new QListWidgetItem(path);
                item->setToolTip(path);
                videoListWidget->addItem(item);

                libvlc_media_list_lock(videoList);
                libvlc_media_list_add_media(videoList, videoPath);
                libvlc_media_list_unlock(videoList);

                libvlc_media_release(videoPath);
            }
        }

        emit ShouldInitializeThumbnailToolBar();
        emit VideoListCountChanged(videoListWidget->count());
    }
}

void MainWindow::DeleteVideo() noexcept
{
    /************lock*************/
    libvlc_media_list_lock(videoList);

    // 如果删除的是当前播放的项目，则直接播放下一个或者停下来，不然即使删除了该项目也会继续播放
    if (libvlc_media_list_player_is_playing(videoPlayer)
     && videoListWidget->currentItem()->text() == GetCurrentItemName())
    {
        const auto currentIndexOfSongList = GetCurrentItemIndex();  // 先保存一下当前编号，一会列表就改了

        if Q_UNLIKELY(videoListWidget->count() == 1)  // 全都删完了就自动停下来
        {                                               // 判等于1是因为此时还未对videoListWidget进行删除操作
            libvlc_media_list_player_stop(videoPlayer);

            tray->setToolTip("无正在播放项目");    // 停下来了自然没有项目正在播放
        }
        else
        {
            libvlc_media_list_player_next(videoPlayer);
        }

        libvlc_media_list_remove_index(videoList, currentIndexOfSongList);  // 在播放列表中移除此项
    }
    else    // 如果删除的不是当前播放的项目，则根据名字寻找该项目在实际播放列表中的位置然后删除之
    {
        const auto currentItemName = videoListWidget->currentItem()->text();
        const auto count = libvlc_media_list_count(videoList);
        for (int i = 0; i < count; ++i)
        {
            if Q_UNLIKELY(currentItemName == GetItemNameAtIndex(i)) // 通过名字寻找要删除的项目的编号
            {
                libvlc_media_list_remove_index(videoList, i);

                break;
            }
        }
    }

    libvlc_media_list_unlock(videoList);
    /*************unlock*************/

    delete videoListWidget->takeItem(videoListWidget->currentRow());    // takeItem返回的指针需要手动释放

    emit VideoListCountChanged(videoListWidget->count());   // 发射信号通知列表项已发生改变
}

void MainWindow::SetAspectRatio(const QString &ratioText) noexcept
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
}

void MainWindow::SetPlaybackMode(const QString &mode) noexcept
{
    if (mode == "单曲循环")
    {
        libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_repeat);
        settings->setValue("PlaybackMode", "repeat");
    }
    else if (mode == "列表循环")
    {
        libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_loop);
        settings->setValue("PlaybackMode", "loop");
    }
    else if (mode == "不循环")
    {
        libvlc_media_list_player_set_playback_mode(videoPlayer, libvlc_playback_mode_default);
        settings->setValue("PlaybackMode", "default");
    }
    else
    {
        QMessageBox::information(this, "警告", "暂不支持“随机播放”");
    }
}

void MainWindow::SetVolume(int value) noexcept
{
    auto *player = libvlc_media_list_player_get_media_player(videoPlayer);
    libvlc_audio_set_volume(player, value);
    libvlc_media_player_release(player);

    if (value == 0) // 音量为0时记得将换成静音的图标
    {
        volumeButton->setIcon(VolumeButtonIcon_MUTE);
    }
    else            // 音量不为0时则换成有声音的图标（暂不处理音量小于0的情况，因为正常操作并不会出现）
    {
        volumeButton->setIcon(VolumeButtonIcon_UNMUTE);
    }

    volumeSlider->setToolTip(QString::number(value));   // 展示当前音量
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

// 获取位于播放列表index处的项目的名字
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
    QSettings Reg(RunAtStartupRegPath,QSettings::NativeFormat);
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
