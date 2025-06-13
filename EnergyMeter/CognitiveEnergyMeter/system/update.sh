COMPILER_LIB=../../compiler/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/lib
COMPILER_INC=../../compiler/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/include

cp -r ./mysql $COMPILER_INC
cp ./lib/libmysqlclient.so $COMPILER_LIB
cp ./lib/libmysqlclient.a $COMPILER_LIB
cp ./lib/libz.so $COMPILER_LIB

