###description###
for the pourpose of use below:
1. misc driver;
2. sempaphore/atomic/wait_queue;
3. app ops device.

###usage###

code compile
load Android SDK env...
cd api-test
make // api_test.ko
cd ApiTester
mm . // ApiTester

test
cp api_test.ko mnt/sdcard
insmod api_test.ko
cp ApiTester
chmod 777 ApiTester
./ApiTester
