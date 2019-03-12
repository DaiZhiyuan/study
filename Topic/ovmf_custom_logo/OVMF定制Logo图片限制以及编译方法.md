***OVMF定制Logo图片限制以及编译方法***

- 1 OVMF LOGO格式
    - 1.1 图片格式限制
    - 1.2 图片大小限制
    - 1.3 图片分辨率限制
    - 1.3 JPG转换格式
    - 1.4 图片路径
- 2 OVMF编译方法
    - 2.1 建立EDKII编译环境
    - 2.2 安装编译依赖
    - 2.3 初始化编译环境和基本组件
    - 2.4 直接编译
    - 2.5 重新配置
    - 2.6 运行QEMU测试OVMF
    - 2.7 OVMF精简
    - 2.8 OVMF增加Debug调试信息

# 1 OVMF LOGO格式

## 1.1 图片格式限制

- 8bit位图
- 256色
- Windows 3.x format

验证方式：
> [root@centos7 ~]# file tsinghua.bmp
tsinghua.bmp: PC bitmap, Windows 3.x format, 500 x 204 x 8

## 1.2 图片大小限制

- 图片大小应该小于1024KB。

验证方式：
> [root@centos7 ~]# du -sh tsinghua.bmp
104K    tsinghua.bmp

## 1.3 图片分辨率限制
- 分辨率并无特殊需求，OVMF会居中显示。建议使用特定比例如：4：3、19:6等。

## 1.3 JPG转换格式

1. 安装
> $> yum install ImageMagick

2. 转换JPG到BMP
> $> convert /root/tsinghua.jpg -alpha off -type palette -compress none  BMP3:tsinghua.bmp

## 1.4 图片路径

- edk2/MdeModulePkg/Logo/Logo.bmp

# 2 OVMF编译方法

## 2.1 建立EDKII编译环境

下载源代码：
> #>  git clone https://github.com/tianocore/edk2.git

## 2.2 安装编译依赖

编译依赖列表：
- ACPI Source Language (ASL) Compiler Setup
- The NASM assembler
- Python2.7

> #>  yum -y install nasm acpica-tools python-devel libuuid-devel

## 2.3 初始化编译环境和基本组件

> #> source edksetup.sh BaseTools
> #> make -C BaseTools/

```bash
WORKSPACE: /root/edk2
EDK_TOOLS_PATH: /root/edk2/BaseTools
CONF_PATH: /root/edk2/Conf/
build_rule.txt
tools_def.txt       -->  Information about 3rd party tools 
target.txt            -->  Restricts a build to defined values

编辑target.txt文件
TARGET = RELEASE
ACTIVE_PLATFORM  = OvmfPkg/OvmfPkgX64.dsc
TOOL_CHAIN_TAG = GCC48
TARGET_ARCH  = X64

编译基本组件：
make -C edk2/BaseTools
配置Conf/target.txt 可直接使用build

```

## 2.4 直接编译

> #> git submodule update --force --init --recursive
>
> #> build -a X64 -p MdeModulePkg/MdeModulePkg.dsc -t GCC48 -b RELEASE
>
> #> build -a X64 -p OvmfPkg/OvmfPkgX64.dsc -DSECURE_BOOT_ENABLE -t GCC48 -b RELEASE --cmd-len=65536 --hash

## 2.5 重新配置

> #> . edksetup.sh --reconfig

## 2.6 运行QEMU测试OVMF

QEMU加入下列参数：
```bash
-drive if=pflash,format=raw,readonly,file=/usr/share/ovmf/OVMF_CODE.fd 
-drive if=pflash,format=raw,file=/usr/share/ovmf/OVMF_VARS.fd
```

## 2.7 OVMF精简

```patch
diff --git a/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c b/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c
index a9d2269..648a446 100644
--- a/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c
+++ b/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c
@@ -38,12 +38,12 @@ UiCustomizeFrontPage (
   //
   // Create "Select Language" menu with Oneof opcode.
   //
-  UiCreateLanguageMenu (HiiHandle, StartOpCodeHandle);
+  // UiCreateLanguageMenu (HiiHandle, StartOpCodeHandle);
 
   //
   // Create empty line.
   //
-  UiCreateEmptyLine(HiiHandle, StartOpCodeHandle);
+  // UiCreateEmptyLine(HiiHandle, StartOpCodeHandle);
 
   //
   // Find third party drivers which need to be shown in the front page.
diff --git a/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c b/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c
index b4f4dff..1ea378e 100644
--- a/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c
+++ b/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c
@@ -1110,7 +1110,7 @@ BootMaintCallback (
         // 1. Update the menus (including legacy munu) show in BootMiantenanceManager page.
         // 2. Re-scan the BootOption menus (including the legacy boot option).
         //
-        CustomizeMenus ();
+        // CustomizeMenus ();
         EfiBootManagerRefreshAllBootOption ();
         BOpt_GetBootOptions (Private);
         mFirstEnterBMMForm = TRUE;
diff --git a/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr b/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr
index a14074a..81962e0 100644
--- a/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr
+++ b/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr
@@ -38,6 +38,15 @@ formset
               flags = INTERACTIVE,
               key   = KEY_VALUE_TRIGGER_FORM_OPEN_ACTION;
        endif;
+       
+       // Jintide ->
+       goto FORM_BOOT_CHG_ID,
+             prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
+             help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
+             flags = INTERACTIVE,
+             key = FORM_BOOT_CHG_ID;
+       // Jintide -<
+        
 
     label LABEL_FORM_MAIN_START;
     //
@@ -69,11 +78,13 @@ formset
             flags = INTERACTIVE,
             key = FORM_BOOT_DEL_ID;
 
-       goto FORM_BOOT_CHG_ID,
-            prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
-            help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
-            flags = INTERACTIVE,
-            key = FORM_BOOT_CHG_ID;
+       // Jintide
+       // goto FORM_BOOT_CHG_ID,
+       //     prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
+       //     help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
+       //     flags = INTERACTIVE,
+       //     key = FORM_BOOT_CHG_ID;
+
   endform;
 
   form formid = FORM_DRIVER_SETUP_ID,
diff --git a/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c b/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c
index b2faa79..edd0e25 100644
--- a/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c
+++ b/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c
@@ -1669,7 +1669,7 @@ PlatformBootManagerWaitCallback (
   BootLogoUpdateProgress (
     White.Pixel,
     Black.Pixel,
-    L"Start boot option",
+    L"Starting OS",
     White.Pixel,
     (Timeout - TimeoutRemain) * 100 / Timeout,
     0
diff --git a/OvmfPkg/OvmfPkgX64.dsc b/OvmfPkg/OvmfPkgX64.dsc
index aa3efc5..0960016 100644
--- a/OvmfPkg/OvmfPkgX64.dsc
+++ b/OvmfPkg/OvmfPkgX64.dsc
@@ -704,7 +704,9 @@
   MdeModulePkg/Logo/LogoDxe.inf
   MdeModulePkg/Application/UiApp/UiApp.inf {
     <LibraryClasses>
-      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
+      # Jintide
+      # NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
+      # Jintide
       NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
       NULL|MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
 !ifdef $(CSM_ENABLE)
```

## 2.8 OVMF增加Debug调试信息

在编译时，选择编译DEBUG模式：

> #> build -a X64 -p OvmfPkg/OvmfPkgX64.dsc -DSECURE_BOOT_ENABLE -t GCC48 -b RELEASE --cmd-len=65536 --hash

QEMU启动参数增加：
```
  -debugcon file:/var/log/guestbios.log -global isa-debugcon.iobase=0x402 
```
GuestOS在启动时将OVMF的DEBUG信息输出到/var/log/guestbios.log文件中。