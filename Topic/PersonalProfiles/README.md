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
