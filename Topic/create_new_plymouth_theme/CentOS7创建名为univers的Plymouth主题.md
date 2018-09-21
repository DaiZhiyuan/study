***CentOS7创建名为univers的Plymouth主题***

[TOC]

# 1 简介
Plymouth是一个Linux支持的启动动画，使用DRM或KMS驱动实现。

## 1.1 准备PNG图片
```
$> wget http://www.weesk.com/wallpaper/art-digital/espace-univers/une-planete-espace-univers-art-digital.jpg -O splash.jpg
$> convert splash.jpg splash.png
$> file splash.png 
splash.png: PNG image data, 1920 x 1200, 8-bit/color RGB, non-interlaced
```

## 1.2 创建univers主题

### 1.2.1 准备目录结构
```
$> mkdir /usr/share/plymouth/themes/univers
$> cp splash.png /usr/share/plymouth/themes/univers
$> touch /usr/share/plymouth/univers/univers.plymouth
```

### 1.2.2 编辑univers.plymouth主题描述文件：
```
[Plymouth Theme]
Name=Univers
Description=Univers CentOS 7 Theme
ModuleName=script

[script]
ImageDir=/usr/share/plymouth/themes/univers
ScriptFile=/usr/share/plymouth/themes/univers/univers.script
```

### 1.2.3 编辑univers.script：
```
wallpaper_image = Image("splash.png");
screen_width = Window.GetWidth();
screen_height = Window.GetHeight();
resized_wallpaper_image = wallpaper_image.Scale(screen_width,screen_height);
wallpaper_sprite = Sprite(resized_wallpaper_image);
wallpaper_sprite.SetZ(-100);
```

##  1.3 重新创建initrd/ramdisk
```
$> plymouth-set-default-theme -R univers
```
