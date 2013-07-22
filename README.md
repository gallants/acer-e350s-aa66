== Build ==

export PATH=$PATH:b2g-toolchains/arm-linux-androideabi-4.6.3/linux-x86/bin/

cd kernel
./build aa66 clean   
make TARGET_PRODUCT=aa66 MTK_ROOT_CUSTOM=../mediatek/custom all

== Not Working!! ==

1. als/ps sensor [cm3625]
2. camera [imgsensor]
3. /dev/block/mmcblk1p1 disapears after reboot serval times
4. gsm/wcdma signal not stable

