== Not WORK!! ==

don't fork this repo now!!

== Build ==

export PATH=$PATH:b2g-toolchains/arm-linux-androideabi-4.6.3/linux-x86/bin/

cd kernel
./build aa66 clean   
make TARGET_PRODUCT=aa66 MTK_ROOT_CUSTOM=../mediatek/custom all
