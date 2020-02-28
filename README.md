# VideoWallpaper
 一款基于libvlc和Qt的开源动态壁纸软件

## 预览
![无法预览请打开此链接：https://s2.ax1x.com/2020/02/20/3eUOKO.gif](https://s2.ax1x.com/2020/02/20/3eUOKO.gif)

## 功能
 使用该软件可以将Windows桌面壁纸更换成您喜欢的动图、视频等多媒体文件。软件使用libvlc作为后端解码，理论上能支持所有能使用vlc播放器播放的多媒体文件。

## 下载
 * [备用地址](https://pan.baidu.com/s/1970WzJKsXA8YAg03Z1SR8Q) 提取码：343c
 * 下载VideoWallpaper.7z解压后双击VideoWallpaper.exe即可使用，如果无法打开则需下载vc_redist.x64.7z解压后双击安装VC运行库
 * 软件处于早期开发阶段，编译好的二进制文件的发布可能略有延迟，若想体验最新版可以根据下面的编译教程自行编译体验。

## 构建
 * 环境：Windows7及以上系统、Qt5.x、[libvlc3.0.8 win64版](http://download.videolan.org/pub/videolan/vlc/3.0.8/win64/vlc-3.0.8-win64.7z)
 * 编译器：支持C++17或以上
 * VideoWallpaper.pro文件已经配置好依赖，首先先按照正常编译Qt项目的流程编译（使用Qt Creator或Qt VS Tools打开该pro文件载入项目然后进行编译），然后将libvlc.dll、libvlccore.dll和plugins文件夹放在和生成的exe文件同一目录中即可运行
 * libvlc.dll、libvlccore.dll和plugins文件夹可在libvlc项目文件夹或vlc安装目录下获得
 * 默认配置下仅支持64位系统，若要在32位系统下使用，请下载[libvlc3.0.8 win32版](http://download.videolan.org/pub/videolan/vlc/3.0.8/win32/vlc-3.0.8-win32.7z)然后将其中的sdk文件夹拷贝覆盖原项目中的sdk文件夹，libvlc.dll、libvlccore.dll和plugins文件夹也使用32位版的进行配置，方法同上

## 联系
 邮箱：love4uandgzfc@gmail.com


