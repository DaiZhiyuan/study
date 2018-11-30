[TOC]

# 1. 使用内置的set和shopt命令设置bash选项

## 1.1 set

获取帮助信息：
```shell
$> type set
set is a shell builtin
$> help set
```
### 1.1.1 获取当前options

```shell
$> set -o
allexport       off
braceexpand     on
emacs           on
errexit         off
errtrace        off
functrace       off
hashall         on
histexpand      on
history         on
ignoreeof       off
interactive-comments    on
keyword         off
monitor         on
noclobber       off
noexec          off
noglob          off
nolog           off
notify          off
nounset         off
onecmd          off
physical        off
pipefail        off
posix           off
privileged      off
verbose         off
vi              off
xtrace          off
```

打开特定option：
> set -o [option]

关闭特定option：
> set +o [option]


name | key | comment
---|---|---
allexport |	-a | 从这个选项中被设置开始就自动标明要输出的新变量或修改过的变量，直至选项被复位 
braceexpand | -B | 打开花括号扩展，它是一个默认设置 
emacs | done |使用emacs内置编辑器进行命令行编辑，是一个默认设置 
errexit	| -e | 当命令返回一个非零退出状态（失败）时退出
histexpand | -H |执行历史替换时打开!和!!扩展，是一个默认设置 
history | done | 打开命令行历史、默认为打开 
ignoreeof | done | 禁止用EOF(Ctrl+D)键退出shell。必须键入exit才能退出。等价于设置shell变量IGNOREEOF=10
keyword | -k | 将关键字参数放到命令的环境中 
interactive-comments | done | 对于交互式shell，把#符后面的文本作为注释 
monitor	| -m | 设置作业控制 
noclobber | -C | 防止文件在重定向时被重写 
noexec | -n | 读命令，但不执行。用来检查脚本的语法。交互式运行时不开启 
noglob | -d | 禁止用路径名扩展。即关闭通配符 
notify | -b | 后台作业完成时通知用户 
nounset | -u | 扩展一个未设置的变量时显示一个错误信息 
onecmd | -t | 在读取和执行命令后退出 
physical | -P | 设置时，在键入cd或pwd禁止符号链接。用物理目录代替 
privileged | -p | 设置后，shell不读取.profile或ENV文件，且不从环境继承shell函数，将自动为setuid脚本开启特权 
verbose | -v | 为调试打开verbose模式 
vi | done | 使用vi内置编辑器进行命令行编辑 
xtrace | -x | 为调试打开echo模式 

## 1.2 shopt

shopt命令是set命令的一种替代，很多方面都和set命令一样，但它增加了很多选项。
- “-p” 选项来查看shopt选项的设置
- “-u” unset表示disable
- “-s” set表示enable

### 1.2.1 查看当前设置

```shell
$> shopt -p
shopt -u autocd
shopt -u cdable_vars
shopt -u cdspell
shopt -u checkhash
shopt -u checkjobs
shopt -s checkwinsize
shopt -s cmdhist
shopt -u compat31
shopt -u compat32
shopt -u compat40
shopt -u compat41
shopt -u direxpand
shopt -u dirspell
shopt -u dotglob
shopt -u execfail
shopt -s expand_aliases
shopt -u extdebug
shopt -s extglob
shopt -s extquote
shopt -u failglob
shopt -s force_fignore
shopt -u globstar
shopt -u gnu_errfmt
shopt -s histappend
shopt -u histreedit
shopt -u histverify
shopt -u hostcomplete
shopt -u huponexit
shopt -s interactive_comments
shopt -u lastpipe
shopt -u lithist
shopt -s login_shell
shopt -u mailwarn
shopt -u no_empty_cmd_completion
shopt -u nocaseglob
shopt -u nocasematch
shopt -u nullglob
shopt -s progcomp
shopt -s promptvars
shopt -u restricted_shell
shopt -u shift_verbose
shopt -s sourcepath
shopt -u xpg_echo
```


option | comment
---|---
cdable_vars	| 如果给cd内置命令的参数不是一个目录，就假设它是一个变量名，变量的值是将要转换到的目录
cdspell | 纠正cd命令中目录名的较小拼写错误。检查的错误包括颠倒顺序的字符，遗漏的字符以及重复的字符。如果知道一处修改，正确的路径就打印出，命令将继续。只用于交互式shell
checkhash | bash在试图执行一个命令前，先在哈希表中寻找，以确定命令是否存在。如果命令不存在，就执行正常路径搜索
checkwinsize | bash在每个命令后检查窗口大小，如果有必要，就更新LINES和COLUMNS的值
cmdhist | bash试图将一个多行命令的所有行保存在同一个历史项中。这使得多行命令的重新编辑更方便
dotglob | bash在文件名扩展的结果中包括以点（.）开头的文件名
execfail | 如果一个交互式shell不能执行指定给exec内置命令作为参数的文件，它不会退出。如果exec失败，一个交互式shell不会退出 
expand_aliases | 别名被扩展。默认为打开 
extglob	| 打开扩展的模式匹配特征（正常的表达式元字符来自Korn shell的文件名扩展） 
histappend | 当shell退出时，历史清单将添加到以HISTFILE变量的值命名的文件中，而不是覆盖文件 
histreedit | 如果readline正被使用，用户有机会重新编辑一个失败的历史替换 
histverify | 如果设置，且readline正被使用，历史替换的结果不会立即传递给shell解析器。而是将结果行装入readline编辑缓冲区中，允许进一步修改 
hostcomplete | 如果设置，且readine正被使用，当正在完成一个包含@的词时bash将试图执行主机名补全。默认为打开 
huponexit | 如果设置，当一个交互式登陆shell退出时，bash将发送一个SIGHUP（挂起信号）给所有的作业 
interactive_comments | 在一个交互式shell中，允许以#开头的词以及同一行中其他的字符被忽略。默认为打开 
lithist | 如果打开，且cmdhist选项也打开，多行命令将用嵌入的换行符保存到历史中，而无需在可能的地方用分号来分隔 
mailwarn | 如果设置，且bash用来检查邮件的文件自从上次检查后已经被访问，将显示消息“The mail in mailfile has been read” 
nocaseglob | 如果设置，当执行文件名扩展时，bash在不区分大小写的方式下匹配文件名 
nullglob | 如果设置，bash允许没有匹配任何文件的文件名模式扩展成一个空串，而不是它们本身 
promptvars | 如果设置，提示串在被扩展后再经历变量和参量扩展。默认为打开 
restricted_shell | 如果shell在受限模式下启动就设置这个选项。该值不能被改变。当执行启动文件时，不能复位该选项，允许启动文件发现shell是否是受限的 
sourcepath | 如果设置，source内置命令使用PATH的值来寻找包含作为参数提供的文件的目录。默认为打开 
source | 点（.）的别名
shift_verbose | 如果该选项设置，当移动计数超过位置参量个数时，shift内置命令将打印一个错误消息 

