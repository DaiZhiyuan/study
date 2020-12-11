#!/bin/bash
setenforce 0
rm -rvf /root/centos_tree
lorax --product="CentOS" --version="7" --release="160722" --source="/root/centos_repo" --bugurl="http://bugs.centos.org" --isfinal /root/centos_tree
rm -rvf /root/centos_tree/images/boot.iso

