## vim配置

.vimrc
```vimrc
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" vim setting
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set expandtab
set shiftwidth=4
set tabstop=4
set softtabstop=4
set shiftwidth=4

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" cscope setting
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
if has("cscope")
  set csprg=/usr/bin/cscope
  set csto=1
  set cst
  set nocsverb
  " add any database in current directory
  if filereadable("cscope.out")
      cs add cscope.out
  endif
  set csverb
endif

nmap <C-@>s :cs find s <C-R>=expand("<cword>")<CR><CR>
nmap <C-@>g :cs find g <C-R>=expand("<cword>")<CR><CR>
nmap <C-@>c :cs find c <C-R>=expand("<cword>")<CR><CR>
nmap <C-@>t :cs find t <C-R>=expand("<cword>")<CR><CR>
nmap <C-@>e :cs find e <C-R>=expand("<cword>")<CR><CR>
nmap <C-@>f :cs find f <C-R>=expand("<cfile>")<CR><CR>
nmap <C-@>i :cs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
nmap <C-@>d :cs find d <C-R>=expand("<cword>")<CR><CR>
```

## git配置

.git-prompt.sh
- /usr/share/git-core/contrib/completion/git-prompt.sh

.gitignore
```
#
# NOTE! Don't add files that are generated in specific
# subdirectories here. Add them in the ".gitignore" file
# in that subdirectory instead.
#
# NOTE! Please use 'git ls-files -i --exclude-standard'
# command after changing this file, to see if there are
# any tracked files which get ignored after the change.
#
# Normal rules
#
.*
*.o
*.o.*
*.a
*.s
*.ko
*.so
*.so.dbg
*.mod.c
*.i
*.lst
*.symtypes
*.order
*.elf
*.bin
*.tar
*.gz
*.bz2
*.lzma
*.xz
*.lz4
*.lzo
*.patch
*.gcno
modules.builtin
Module.symvers
*.dwo
*.su

#
# Top-level generic files
#
/tags
/TAGS
/linux
/vmlinux
/vmlinux.32
/vmlinux-gdb.py
/vmlinuz
/System.map
/Module.markers

#
# Debian directory (make deb-pkg)
#
/debian/

#
# tar directory (make tar*-pkg)
#
/tar-install/

#
# git files that we don't want to ignore even it they are dot-files
#
!.gitignore
!.mailmap

#
# Generated include files
#
include/config
include/generated
arch/*/include/generated

# stgit generated dirs
patches-*

# quilt's files
patches
series

# cscope files
cscope.*
ncscope.*

# gnu global files
GPATH
GRTAGS
GSYMS
GTAGS

# id-utils files
ID

*.orig
*~
\#*#

#
# Leavings from module signing
#
extra_certificates
signing_key.pem
signing_key.priv
signing_key.x509
x509.genkey

# Kconfig presets
all.config

# Kdevelop4
*.kdev4

kernel/x509_certificate_list
Documentation/networking/ifenslave
```

.gitconfig
```
[user]
    name = Daizhiyuan
    email = daizhiyuan@tsinghua.edu.cn
[core]
    quotepath = false
    editor = vim
[credential]
    helper = store
[color]
    ui = true
    diff = true
[i18n]
    commitencoding = utf-8
    logoutputencoding = gbk
[push]
    default = current
[alias]
    logo = log --graph --pretty=oneline
    logs = log --graph --pretty='%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit
    logl = log --graph --pretty='%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit --all
```

.bashrc
```
source ~/.git-prompt.sh
export PS1='[\u@\h \W$(__git_ps1 " (%s)")]\$ '
```

## tmux配置

.tmux.conf
```
# Send prefix
set-option -g prefix C-a
unbind-key C-a
bind-key C-a send-prefix

# Use Alt-arrow keys to switch panes
bind -n M-Left select-pane -L
bind -n M-Right select-pane -R
bind -n M-Up select-pane -U
bind -n M-Down select-pane -D

# Shift arrow to switch windows
bind -n S-Left previous-window
bind -n S-Right next-window

# Mouse mode
set -g mode-mouse on
set -g mouse-resize-pane on
set -g mouse-select-pane on
set -g mouse-select-window on

# Custom
set-option -g status-right "Jerry Dai %H:%M %d-%b-%y"

# Set easier window split keys
bind-key v split-window -h
bind-key h split-window -v

# Easy config reload
bind-key r source-file ~/.tmux.conf \; display-message "tmux.conf reloaded"
```

## Virtual Machine Instance

Kernel Command Line:
```
root=/dev/mapper/centos_centos7u5-root ro rd.lvm.lv=centos_centos7u5/root rd.lvm.lv=centos_centos7u5/swap rhgb quiet selinux=0 audit=0 cgroup_disable=memory,cpu,cpuacct,blkio,hugetlb elevator=deadline realloc=on nmi_watchdog=0 nosoftlockup disable_ipv6=1 clocksource=tsc tsc=reliable hpet=disable nohz=off highres=off nopti nopku spectre_v2=off spec_store_bypass_disable=off intel_idle.max_cstate=0 mce=ignore_ce
```

FileSystem Mount File
```
#
# /etc/fstab
# Created by anaconda on Thu Dec  5 12:24:37 2019
#
# Accessible filesystems, by reference, are maintained under '/dev/disk'
# See man pages fstab(5), findfs(8), mount(8) and/or blkid(8) for more info
#
/dev/mapper/centos_centos7u5-root /                       xfs     defaults,noatime,nodiratime,nobarrier 0 0
UUID=53af9761-1ff5-4f3d-829a-509cbe861ea4 /boot           xfs     defaults,noatime,nodiratime,nobarrier 0 0
/dev/mapper/centos_centos7u5-home /home                   xfs     defaults,noatime,nodiratime,nobarrier 0 0
/dev/mapper/centos_centos7u5-swap swap                    swap    defaults        0 0
```

## tools


- `cat /etc/profile.d/system-info.sh`
```
[mysql@jerrydai ~]$ cat /etc/profile.d/system-info.sh
#/bin/bash

# Welcome
welcome=$(uname -r)

# Memory
memory_total=$(free -m | awk '/Mem:/ { printf($2)}')
if [ $memory_total -gt 0 ]
then
    memory_usage=$(free -m | awk '/Mem:/ { printf("%3.1f%%", $3/$2*100)}')
else
    memory_usage=0.0%
fi

# Swap memory
swap_total=$(free -m | awk '/Swap:/ { printf($2)}')
if [ $swap_total -gt 0 ]
then
    swap_mem=$(free -m | awk '/Swap:/ { printf("%3.1f%%", $3/$2*100)}')
else
    swap_mem=0.0%
fi

# Usage
usageof=$(df -h / | awk '/\// {print $(NF-1)}')

# System load
load_average=$(awk '{print $1}' /proc/loadavg)

# WHO I AM
whoiam=$(whoami)

# Time
time_cur=$(date)

# Processes
processes=$(ps aux | wc -l)

# Users
user_num=$(users | wc -w)

# Ip address
ip_pre=$(/sbin/ip a|grep inet|grep -v 127.0.0.1|grep -v inet6 | awk '{print $2}' | head -1)
ip_address=${ip_pre%/*}

echo -e "\n"
echo -e "Welcome to $welcome\n"
echo -e "System information as of time: \t$time_cur\n"
echo -e "System load: \t\033[0;33;40m$load_average\033[0m"
echo -e "Processes: \t$processes"
echo -e "Memory used: \t$memory_usage"
echo -e "Swap used: \t$swap_mem"
echo -e "Usage On: \t$usageof"
echo -e "IP address: \t$ip_address"
echo -e "Users online: \t$user_num\n"
if [ "$whoiam"=="root" ]
then
        echo -e "\n"
else
        echo -e "To run a command as administrator(user \"root\"),use \"sudo <command>\"."
fi
```
- `cat hwcap.c`
```
#include <stdio.h>
#include <sys/auxv.h>
#include <asm/hwcap.h>

#define OPTIONS(flag) (hwcaps & flag?"\033[1;32;40msupport\033[0m":"\033[1;31;40mnonsupport\033[0m")

int main(void)
{
        long hwcaps= getauxval(AT_HWCAP);

        printf("   [cpuid] : Some CPU ID registers readable at user-level.                                  (%s)\n", OPTIONS(HWCAP_CPUID));
        printf(" [atomics] : Large System Extensions.                                                       (%s)\n", OPTIONS(HWCAP_ATOMICS));
        printf(" [evtstrm] : Generic timer is configured to generate 'events' at frequency of about 100KHz. (%s)\n", OPTIONS(HWCAP_EVTSTRM));
        printf("    [fphp] : Half-precision floating point.                                                 (%s)\n", OPTIONS(HWCAP_FPHP));
        printf("      [fp] : Single-precision and double-precision floating point.                          (%s)\n", OPTIONS(HWCAP_FP));
        printf("   [jscvt] : Javascript-style double->int convert.                                          (%s)\n", OPTIONS(HWCAP_JSCVT));
        printf("   [lrcpc] : Weaker release consistency.                                                    (%s)\n", OPTIONS(HWCAP_LRCPC));
        printf("   [dcpop] : Data cache clean to Point of Persistence.                                      (%s)\n", OPTIONS(HWCAP_DCPOP));
        printf("   [crc32] : CRC32/CRC32C instructions.                                                     (%s)\n", OPTIONS(HWCAP_CRC32));
        printf("    [sha1] : SHA-1 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA1));
        printf("    [sha2] : SHA-2 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA2));
        printf("    [sha3] : SHA-3 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA3));
        printf("  [sha512] : SHA512 instructions.                                                           (%s)\n", OPTIONS(HWCAP_SHA512));
        printf("     [aes] : AES instructions.                                                              (%s)\n", OPTIONS(HWCAP_AES));
        printf("     [sm3] : SM3 instructions.                                                              (%s)\n", OPTIONS(HWCAP_SM3));
        printf("     [sm4] : SM4 instructions.                                                              (%s)\n", OPTIONS(HWCAP_SM4));
        printf("   [asimd] : Advanced single-instruction-multiple-data.                                     (%s)\n", OPTIONS(HWCAP_ASIMD));
        printf(" [asimdhp] : Advanced SIMD half-precision floating point.                                   (%s)\n", OPTIONS(HWCAP_ASIMDHP));
        printf("   [pmull] : Polynomial Multiply Long instructions.                                         (%s)\n", OPTIONS(HWCAP_PMULL));
        printf(" [asimddp] : SIMD Dot Product.                                                              (%s)\n", OPTIONS(HWCAP_ASIMDDP));
        printf("[asimdrdm] : Rounding Double Multiply Accumulate/Subtract.                                  (%s)\n", OPTIONS(HWCAP_ASIMDRDM));
        printf("     [sve] : Scalable Vector Extension.                                                     (%s)\n", OPTIONS(HWCAP_SVE));
        printf("    [fcma] : Floating point complex number addition and multiplication.                     (%s)\n", OPTIONS(HWCAP_FCMA));

        return 0;
}
```
> gcc hwcap.c -o show_cpu_features

- `cat show_pci_lanes`
```
#!/bin/bash

root_port_list=""

for bridge in `lspci -n | grep 0604 | awk '{print $1}'`; do
    lspci -vvv  -s $bridge | grep "Root Port" 2>&1 > /dev/null  && root_port_list+="$bridge "
done

echo "Host Bridge:"
for root_port in $root_port_list; do
    lspci -vvv -s $root_port | grep 'LnkCap' | awk -F ',' '{print $2 $3}'
done
```