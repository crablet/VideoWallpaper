#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QSize>
#include <QIcon>

/***********constants of UI begin***********/

// 这里对所有QIcon类型都使用#define的原因是QIcon不能在绘图设备还没存在的时候建立，而这里是配置文件

#define LogoIcon                QIcon(":/icons/film-fill.png")
#define AddVideoButtonIcon      QIcon(":/icons/add-fill.png")
#define DeleteVideoButtonIcon   QIcon(":/icons/close-fill.png")
#define PlayButtonIcon          QIcon(":/icons/play-fill.png")
#define PauseButtonIcon         QIcon(":/icons/pause-fill.png")
#define PlayPreviousButtonIcon  QIcon(":/icons/skip-back-fill.png")
#define PlayNextButtonIcon      QIcon(":/icons/skip-forward-fill.png")
#define StopPlayingButtonIcon   QIcon(":/icons/stop-fill.png")
#define VolumeButtonIcon        QIcon(":/icons/volume-down-fill.png")
#define TrayIcon                QIcon(":/icons/film-fill.png")

inline const QSize ButtonIconSize = QSize(24, 24);

// 去除虚线
inline const QString ComboBoxStyle = "QComboBox QAbstractItemView { outline: 0px }";

/***********constants of UI end***********/



/***********constants of config begin***********/


/***********constants of config end***********/

#endif // CONSTANTS_H
