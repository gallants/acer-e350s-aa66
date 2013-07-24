== Build ==

export PATH=$PATH:b2g-toolchains/arm-linux-androideabi-4.6.3/linux-x86/bin/

cd kernel
./build aa66 clean   
make TARGET_PRODUCT=aa66 MTK_ROOT_CUSTOM=../mediatek/custom all

== All Working!! ==

** But Camera Sensors and AF Lens are not stock driver