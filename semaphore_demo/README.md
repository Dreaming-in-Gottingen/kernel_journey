## description ##

for the pourpose of use below:
1. misc driver;
2. sempaphore/atomic/wait_queue;
3. app ops device.

-----------------------------------
## usage ##

* **code compile**

1. load Android SDK env...
2. cd api-test
3. make // api_test.ko
4. cd ApiTester
5. mm . // ApiTester

* **test**
1. cp api_test.ko mnt/sdcard
3. insmod api_test.ko
3. cp ApiTester mnt/sdcard
4. chmod 777 ApiTester
5. logcat -s ApiTester&
6. ./ApiTester
7. key in some according to prompt ...
