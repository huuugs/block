#!/data/data/com.termux/files/usr/bin/bash
cd /data/data/com.termux/files/home/aaa/termux-npu-bridge
gradle "$@" 2>&1 | tee build.log
