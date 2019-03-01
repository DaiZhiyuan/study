***OVMF定制Logo图片限制以及编译方法***

- 1 OVMF LOGO格式
    - 1.1 图片格式限制
    - 1.2 图片大小限制
    - 1.3 图片分辨率限制
    - 1.3 JPG转换格式
- 2 OVMF编译方法
    - 2.1 建立EDKII编译环境
    - 2.2 安装编译依赖
    - 2.3 初始化编译环境和基本组件
    - 2.4 直接编译
    - 2.5 重新配置
    - 2.6 运行QEMU测试OVMF
    - 2.7 OVMF其他修改

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
> #> build -a X64 -p OvmfPkg/OvmfPkgX64.dsc -DSECURE_BOOT_ENABLE -t GCC48 -b RELEASE --cmd-len=65536 --hash

## 2.5 重新配置

> #> . edksetup.sh --reconfig

## 2.6 运行QEMU测试OVMF

QEMU加入下列参数：
```bash
-drive if=pflash,format=raw,readonly,file=/usr/share/ovmf/OVMF_CODE.fd 
-drive if=pflash,format=raw,file=/usr/share/ovmf/OVMF_VARS.fd
```

## 2.7 OVMF其他修改


1. 精简菜单
```
diff -Nur before/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c after/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c
--- before/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c       2019-01-09 23:16:08.000000000 -0500
+++ after/MdeModulePkg/Application/UiApp/FrontPageCustomizedUi.c        2019-01-31 20:25:12.000000000 -0500
@@ -35,15 +35,17 @@
   IN VOID            *StartOpCodeHandle
   )
 {
+  // Jintide ->
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
+  // Jintide -<

   //
   // Find third party drivers which need to be shown in the front page.
diff -Nur before/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c after/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c
--- before/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c   2019-01-09 23:16:08.000000000 -0500
+++ after/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenance.c    2019-01-31 20:48:58.000000000 -0500
@@ -1110,7 +1110,9 @@
         // 1. Update the menus (including legacy munu) show in BootMiantenanceManager page.
         // 2. Re-scan the BootOption menus (including the legacy boot option).
         //
-        CustomizeMenus ();
+        // Jitide ->
+        // CustomizeMenus ();
+        // Jintide -<
         EfiBootManagerRefreshAllBootOption ();
         BOpt_GetBootOptions (Private);
         mFirstEnterBMMForm = TRUE;
diff -Nur before/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr after/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr
--- before/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr  2019-01-09 23:16:08.000000000 -0500
+++ after/MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManager.vfr   2019-01-31 21:45:56.000000000 -0500
@@ -39,6 +39,13 @@
               key   = KEY_VALUE_TRIGGER_FORM_OPEN_ACTION;
        endif;

+       // Jintide ->
+       goto FORM_BOOT_CHG_ID,
+            prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
+            help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
+            flags = INTERACTIVE,
+            key = FORM_BOOT_CHG_ID;
+        // Jintide -<
     label LABEL_FORM_MAIN_START;
     //
     // This is where we will dynamically add a Action type op-code to show
@@ -69,11 +76,13 @@
             flags = INTERACTIVE,
             key = FORM_BOOT_DEL_ID;

-       goto FORM_BOOT_CHG_ID,
-            prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
-            help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
-            flags = INTERACTIVE,
-            key = FORM_BOOT_CHG_ID;
+       // Jintide ->
+       //goto FORM_BOOT_CHG_ID,
+       //     prompt = STRING_TOKEN(STR_FORM_BOOT_CHG_TITLE),
+       //     help = STRING_TOKEN(STR_FORM_BOOT_IMMEDIATE_HELP),
+       //     flags = INTERACTIVE,
+       //     key = FORM_BOOT_CHG_ID;
+       // Jinitde -<
   endform;

   form formid = FORM_DRIVER_SETUP_ID,
diff -Nur before/OvmfPkg/OvmfPkgX64.dsc after/OvmfPkg/OvmfPkgX64.dsc
--- before/OvmfPkg/OvmfPkgX64.dsc       2019-02-01 02:14:06.580389000 -0500
+++ after/OvmfPkg/OvmfPkgX64.dsc        2019-02-01 01:32:02.000000000 -0500
@@ -698,7 +698,9 @@
   MdeModulePkg/Logo/LogoDxe.inf
   MdeModulePkg/Application/UiApp/UiApp.inf {
     <LibraryClasses>
-      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
+      # Jintide ->
+      # NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
+      # Jintide -<
       NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
       NULL|MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
 !ifdef $(CSM_ENABLE)
```

2. 强制OVMF不显示QEMU DVD-ROM设备
```
diff -Nur before/MdeModulePkg/Library/UefiBootManagerLib/BmBoot.c after/MdeModulePkg/Library/UefiBootManager
--- before/MdeModulePkg/Library/UefiBootManagerLib/BmBoot.c     2019-02-01 18:09:04.000000000 +0800
+++ after/MdeModulePkg/Library/UefiBootManagerLib/BmBoot.c      2019-02-01 18:08:12.000000000 +0800
@@ -2262,6 +2262,18 @@
         ASSERT_EFI_ERROR (Status);
       }
     }
+
+    // Jintide ->
+    if (EfiBootManagerFindLoadOption (&NvBootOptions[Index], BootOptions, BootOptionCount) == -1) {
+      if (StrnCmp(NvBootOptions[Index].Description, L"UEFI QEMU DVD-ROM", 17) == 0) {
+        Status = EfiBootManagerDeleteLoadOptionVariable (NvBootOptions[Index].OptionNumber, LoadOptionTypeB
+        //
+        // Deleting variable with current variable implementation shouldn't fail.
+        //
+        ASSERT_EFI_ERROR (Status);
+      }
+    }
+    // Jintide -<
   }

   //
@@ -2269,6 +2281,10 @@
   //
   for (Index = 0; Index < BootOptionCount; Index++) {
     if (EfiBootManagerFindLoadOption (&BootOptions[Index], NvBootOptions, NvBootOptionCount) == -1) {
+      // Jintide ->
+      if (StrnCmp(BootOptions[Index].Description, L"UEFI QEMU DVD-ROM", 17) == 0)
+        continue;
+      // Jintide -<
       EfiBootManagerAddLoadOptionVariable (&BootOptions[Index], (UINTN) -1);
       //
       // Try best to add the boot options so continue upon failure.

```