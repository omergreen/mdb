# Debugging using rt-thread

- Clone it from git@github.com:RT-Thread/rt-thread.git 
- Apply the following patch:

```
diff --git a/bsp/mipssim/applications/main.c b/bsp/mipssim/applications/main.c
index fac004d83..ecc143077 100644
--- a/bsp/mipssim/applications/main.c
+++ b/bsp/mipssim/applications/main.c
@@ -8,10 +8,45 @@
  * 2018-05-10     zhuangwei    first version
  */
 
+#include <stdio.h>
+#include <stdlib.h>
+#include <string.h>
 #include <rtthread.h>
+#include "../stub.h"
+
+static rt_device_t serial;
+
+rt_size_t serial_write(const void* buffer, rt_size_t size) {
+    int ret;
+    while ((ret = rt_device_write(serial, 0, buffer, size)) == 0) {
+    }
+    return ret;
+}
+rt_size_t serial_read(void* buffer, rt_size_t size) {
+    int ret;
+    while ((ret = rt_device_read(serial, 0, buffer, size)) == 0) {
+    }
+    return ret;                                                                                                                                                                       
+}                                                                                                                                                                                     
+void _printf() {
+}                                                                                                                                                                                     
+
+void test() {
+}                                                                                                                                                                                     
 
 int main(int argc, char** argv)
 {   
+    serial = rt_device_find("uart");
+    rt_device_open(serial, 0);
+
+    unsigned long args[] = { (unsigned long)&serial_read, (unsigned long)&serial_write, (unsigned long)&_printf, (unsigned long)&malloc, (unsigned long)&realloc, (unsigned long)&free, (unsigned long)&test };
+
+    memcpy((void *)0xa0100000, _path_to_stub_bin, _path_to_stub_bin_len);
+    ((void (*)(void *))0xa0100000)(args);
 
+    test();
     return 0;
 }
diff --git a/bsp/mipssim/drivers/board.c b/bsp/mipssim/drivers/board.c
index a0ef220bc..fe90a7ad4 100644
--- a/bsp/mipssim/drivers/board.c
+++ b/bsp/mipssim/drivers/board.c
@@ -91,7 +91,7 @@ void rt_hw_board_init(void)
     /* init hardware UART device */
     rt_hw_uart_init();
     /* set console device */
-    rt_console_set_device("uart");
+    rt_console_set_device("uartno");
 #endif
 
 #ifdef RT_USING_HEAP
diff --git a/bsp/mipssim/rtconfig.h b/bsp/mipssim/rtconfig.h
index 6b253ec88..23dedc0c9 100644
--- a/bsp/mipssim/rtconfig.h
+++ b/bsp/mipssim/rtconfig.h
@@ -6,20 +6,18 @@
 
 /* RT-Thread Kernel */
 
-
 #define RT_NAME_MAX 8
 #define RT_ALIGN_SIZE 4
 #define RT_THREAD_PRIORITY_32
 #define RT_THREAD_PRIORITY_MAX 32
 #define RT_TICK_PER_SECOND 100
-#define RT_USING_OVERFLOW_CHECK
 #define RT_USING_HOOK
 #define RT_USING_IDLE_HOOK
 #define RT_IDLE_HOOK_LIST_SIZE 4
-#define IDLE_THREAD_STACK_SIZE 2048
+#define IDLE_THREAD_STACK_SIZE 4096
 #define RT_USING_TIMER_SOFT
 #define RT_TIMER_THREAD_PRIO 4
-#define RT_TIMER_THREAD_STACK_SIZE 2048
+#define RT_TIMER_THREAD_STACK_SIZE 512
 #define RT_DEBUG
 
 /* Inter-Thread communication */
@@ -40,9 +38,9 @@
 
 #define RT_USING_DEVICE
 #define RT_USING_CONSOLE
-#define RT_CONSOLEBUF_SIZE 512
-#define RT_CONSOLE_DEVICE_NAME "uart"
-#define RT_VER_NUM 0x40002
+#define RT_CONSOLEBUF_SIZE 128
+#define RT_CONSOLE_DEVICE_NAME "uartno"
+#define RT_VER_NUM 0x40003
 #define ARCH_MIPS
 
 /* RT-Thread Components */
@@ -64,7 +62,7 @@
 #define FINSH_USING_SYMTAB
 #define FINSH_USING_DESCRIPTION
 #define FINSH_THREAD_PRIORITY 20
-#define FINSH_THREAD_STACK_SIZE 4096
+#define FINSH_THREAD_STACK_SIZE 8192
 #define FINSH_CMD_SIZE 80
 #define FINSH_USING_MSH
 #define FINSH_USING_MSH_DEFAULT
@@ -77,9 +75,13 @@
 
 #define RT_USING_DEVICE_IPC
 #define RT_PIPE_BUFSZ 512
+#define RT_USING_SYSTEM_WORKQUEUE
+#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
+#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
 #define RT_USING_SERIAL
 #define RT_SERIAL_USING_DMA
 #define RT_SERIAL_RB_BUFSZ 64
+#define RT_USING_PIN
 
 /* Using USB */
 
@@ -87,8 +89,6 @@
 /* POSIX layer and C standard library */
 
 #define RT_USING_LIBC
-#define RT_USING_PTHREADS
-#define PTHREAD_NUM_MAX 8
 
 /* Network */
 
@@ -109,9 +109,6 @@
 
 /* Utilities */
 
-#define RT_USING_UTEST
-#define UTEST_THR_STACK_SIZE 4096
-#define UTEST_THR_PRIORITY 20
 
 /* RT-Thread MIPS CPU */

diff --git a/bsp/qemu-vexpress-a9/.config b/bsp/qemu-vexpress-a9/.config
index d333d8865..4b1e9f3f1 100644
--- a/bsp/qemu-vexpress-a9/.config
+++ b/bsp/qemu-vexpress-a9/.config
@@ -68,7 +68,7 @@ CONFIG_RT_USING_INTERRUPT_INFO=y
 CONFIG_RT_USING_CONSOLE=y
 CONFIG_RT_CONSOLEBUF_SIZE=256
 CONFIG_RT_CONSOLE_DEVICE_NAME="uart0"
-CONFIG_RT_VER_NUM=0x40002
+CONFIG_RT_VER_NUM=0x40003
 CONFIG_ARCH_ARM=y
 # CONFIG_RT_USING_CPU_FFS is not set
 CONFIG_ARCH_ARM_CORTEX_A=y
@@ -155,14 +155,15 @@ CONFIG_RT_SERIAL_RB_BUFSZ=64
 # CONFIG_RT_USING_HWTIMER is not set
 # CONFIG_RT_USING_CPUTIME is not set
 CONFIG_RT_USING_I2C=y
+# CONFIG_RT_I2C_DEBUG is not set
 CONFIG_RT_USING_I2C_BITOPS=y
+# CONFIG_RT_I2C_BITOPS_DEBUG is not set
 CONFIG_RT_USING_PIN=y
 # CONFIG_RT_USING_ADC is not set
 # CONFIG_RT_USING_PWM is not set
 CONFIG_RT_USING_MTD_NOR=y
 CONFIG_RT_USING_MTD_NAND=y
 CONFIG_RT_MTD_NAND_DEBUG=y
-# CONFIG_RT_USING_MTD is not set
 # CONFIG_RT_USING_PM is not set
 CONFIG_RT_USING_RTC=y
 # CONFIG_RT_USING_ALARM is not set
@@ -181,6 +182,7 @@ CONFIG_RT_USING_SFUD=y
 CONFIG_RT_SFUD_USING_SFDP=y
 CONFIG_RT_SFUD_USING_FLASH_INFO_TABLE=y
 # CONFIG_RT_SFUD_USING_QSPI is not set
+CONFIG_RT_SFUD_SPI_MAX_HZ=50000000
 # CONFIG_RT_DEBUG_SFUD is not set
 # CONFIG_RT_USING_ENC28J60 is not set
 # CONFIG_RT_USING_SPI_WIFI is not set
@@ -188,15 +190,9 @@ CONFIG_RT_USING_WDT=y
 # CONFIG_RT_USING_AUDIO is not set
 # CONFIG_RT_USING_SENSOR is not set
 # CONFIG_RT_USING_TOUCH is not set
-
-#
-# Using Hardware Crypto drivers
-#
 # CONFIG_RT_USING_HWCRYPTO is not set
-
-#
-# Using WiFi
-#
+# CONFIG_RT_USING_PULSE_ENCODER is not set
+# CONFIG_RT_USING_INPUT_CAPTURE is not set
 # CONFIG_RT_USING_WIFI is not set
 
 #
@@ -301,11 +297,6 @@ CONFIG_LWIP_NETIF_LOOPBACK=0
 CONFIG_RT_LWIP_USING_PING=y
 # CONFIG_RT_LWIP_DEBUG is not set
 
-#
-# Modbus master and slave stack
-#
-# CONFIG_RT_USING_MODBUS is not set
-
 #
 # AT commands
 #
@@ -336,10 +327,14 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_WEBCLIENT is not set
 # CONFIG_PKG_USING_WEBNET is not set
 # CONFIG_PKG_USING_MONGOOSE is not set
+# CONFIG_PKG_USING_MYMQTT is not set
+# CONFIG_PKG_USING_KAWAII_MQTT is not set
+# CONFIG_PKG_USING_BC28_MQTT is not set
 # CONFIG_PKG_USING_WEBTERMINAL is not set
 # CONFIG_PKG_USING_CJSON is not set
 # CONFIG_PKG_USING_JSMN is not set
 # CONFIG_PKG_USING_LIBMODBUS is not set
+# CONFIG_PKG_USING_FREEMODBUS is not set
 # CONFIG_PKG_USING_LJSON is not set
 # CONFIG_PKG_USING_EZXML is not set
 # CONFIG_PKG_USING_NANOPB is not set
@@ -361,6 +356,7 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_COAP is not set
 # CONFIG_PKG_USING_NOPOLL is not set
 # CONFIG_PKG_USING_NETUTILS is not set
+# CONFIG_PKG_USING_PPP_DEVICE is not set
 # CONFIG_PKG_USING_AT_DEVICE is not set
 # CONFIG_PKG_USING_ATSRV_SOCKET is not set
 # CONFIG_PKG_USING_WIZNET is not set
@@ -373,12 +369,27 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_ALI_IOTKIT is not set
 # CONFIG_PKG_USING_AZURE is not set
 # CONFIG_PKG_USING_TENCENT_IOTHUB is not set
+# CONFIG_PKG_USING_JIOT-C-SDK is not set
+# CONFIG_PKG_USING_UCLOUD_IOT_SDK is not set
+# CONFIG_PKG_USING_JOYLINK is not set
 # CONFIG_PKG_USING_NIMBLE is not set
 # CONFIG_PKG_USING_OTA_DOWNLOADER is not set
 # CONFIG_PKG_USING_IPMSG is not set
 # CONFIG_PKG_USING_LSSDP is not set
 # CONFIG_PKG_USING_AIRKISS_OPEN is not set
 # CONFIG_PKG_USING_LIBRWS is not set
+# CONFIG_PKG_USING_TCPSERVER is not set
+# CONFIG_PKG_USING_PROTOBUF_C is not set
+# CONFIG_PKG_USING_ONNX_PARSER is not set
+# CONFIG_PKG_USING_ONNX_BACKEND is not set
+# CONFIG_PKG_USING_DLT645 is not set
+# CONFIG_PKG_USING_QXWZ is not set
+# CONFIG_PKG_USING_SMTP_CLIENT is not set
+# CONFIG_PKG_USING_ABUP_FOTA is not set
+# CONFIG_PKG_USING_LIBCURL2RTT is not set
+# CONFIG_PKG_USING_CAPNP is not set
+# CONFIG_PKG_USING_RT_CJSON_TOOLS is not set
+# CONFIG_PKG_USING_AGILE_TELNET is not set
 
 #
 # security packages
@@ -386,6 +397,7 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_MBEDTLS is not set
 # CONFIG_PKG_USING_libsodium is not set
 # CONFIG_PKG_USING_TINYCRYPT is not set
+# CONFIG_PKG_USING_TFM is not set
 
 #
 # language packages
@@ -400,6 +412,8 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_OPENMV is not set
 # CONFIG_PKG_USING_MUPDF is not set
 # CONFIG_PKG_USING_STEMWIN is not set
+# CONFIG_PKG_USING_WAVPLAYER is not set
+# CONFIG_PKG_USING_TJPGD is not set
 
 #
 # tools packages
@@ -412,6 +426,13 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_QRCODE is not set
 # CONFIG_PKG_USING_ULOG_EASYFLASH is not set
 # CONFIG_PKG_USING_ADBD is not set
+# CONFIG_PKG_USING_COREMARK is not set
+# CONFIG_PKG_USING_DHRYSTONE is not set
+# CONFIG_PKG_USING_NR_MICRO_SHELL is not set
+# CONFIG_PKG_USING_CHINESE_FONT_LIBRARY is not set
+# CONFIG_PKG_USING_LUNAR_CALENDAR is not set
+# CONFIG_PKG_USING_BS8116A is not set
+# CONFIG_PKG_USING_URLENCODE is not set
 
 #
 # system packages
@@ -430,6 +451,11 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_DFS_YAFFS is not set
 # CONFIG_PKG_USING_LITTLEFS is not set
 # CONFIG_PKG_USING_THREAD_POOL is not set
+# CONFIG_PKG_USING_ROBOTS is not set
+# CONFIG_PKG_USING_EV is not set
+# CONFIG_PKG_USING_SYSWATCH is not set
+# CONFIG_PKG_USING_SYS_LOAD_MONITOR is not set
+# CONFIG_PKG_USING_PLCCORE is not set
 
 #
 # peripheral libraries and drivers
@@ -437,6 +463,7 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_SENSORS_DRIVERS is not set
 # CONFIG_PKG_USING_REALTEK_AMEBA is not set
 # CONFIG_PKG_USING_SHT2X is not set
+# CONFIG_PKG_USING_SHT3X is not set
 # CONFIG_PKG_USING_STM32_SDIO is not set
 # CONFIG_PKG_USING_ICM20608 is not set
 # CONFIG_PKG_USING_U8G2 is not set
@@ -445,10 +472,16 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_SX12XX is not set
 # CONFIG_PKG_USING_SIGNAL_LED is not set
 # CONFIG_PKG_USING_LEDBLINK is not set
+# CONFIG_PKG_USING_LITTLED is not set
+# CONFIG_PKG_USING_LKDGUI is not set
+# CONFIG_PKG_USING_NRF5X_SDK is not set
+# CONFIG_PKG_USING_NRFX is not set
 # CONFIG_PKG_USING_WM_LIBRARIES is not set
 # CONFIG_PKG_USING_KENDRYTE_SDK is not set
 # CONFIG_PKG_USING_INFRARED is not set
 # CONFIG_PKG_USING_ROSSERIAL is not set
+# CONFIG_PKG_USING_AGILE_BUTTON is not set
+# CONFIG_PKG_USING_AGILE_LED is not set
 # CONFIG_PKG_USING_AT24CXX is not set
 # CONFIG_PKG_USING_MOTIONDRIVER2RTT is not set
 # CONFIG_PKG_USING_AD7746 is not set
@@ -456,6 +489,17 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_I2C_TOOLS is not set
 # CONFIG_PKG_USING_NRF24L01 is not set
 # CONFIG_PKG_USING_TOUCH_DRIVERS is not set
+# CONFIG_PKG_USING_MAX17048 is not set
+# CONFIG_PKG_USING_RPLIDAR is not set
+# CONFIG_PKG_USING_AS608 is not set
+# CONFIG_PKG_USING_RC522 is not set
+# CONFIG_PKG_USING_EMBARC_BSP is not set
+# CONFIG_PKG_USING_EXTERN_RTC_DRIVERS is not set
+# CONFIG_PKG_USING_MULTI_RTIMER is not set
+# CONFIG_PKG_USING_MAX7219 is not set
+# CONFIG_PKG_USING_BEEP is not set
+# CONFIG_PKG_USING_EASYBLINK is not set
+# CONFIG_PKG_USING_PMS_SERIES is not set
 
 #
 # miscellaneous packages
@@ -466,12 +510,15 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_MINILZO is not set
 # CONFIG_PKG_USING_QUICKLZ is not set
 # CONFIG_PKG_USING_MULTIBUTTON is not set
+# CONFIG_PKG_USING_FLEXIBLE_BUTTON is not set
 # CONFIG_PKG_USING_CANFESTIVAL is not set
 # CONFIG_PKG_USING_ZLIB is not set
 # CONFIG_PKG_USING_DSTR is not set
 # CONFIG_PKG_USING_TINYFRAME is not set
 # CONFIG_PKG_USING_KENDRYTE_DEMO is not set
 # CONFIG_PKG_USING_DIGITALCTRL is not set
+# CONFIG_PKG_USING_UPACKER is not set
+# CONFIG_PKG_USING_UPARAM is not set
 
 #
 # samples: kernel and components samples
@@ -483,8 +530,14 @@ CONFIG_RT_USING_LWP=y
 # CONFIG_PKG_USING_HELLO is not set
 # CONFIG_PKG_USING_VI is not set
 # CONFIG_PKG_USING_NNOM is not set
+# CONFIG_PKG_USING_LIBANN is not set
+# CONFIG_PKG_USING_ELAPACK is not set
+# CONFIG_PKG_USING_ARMv7M_DWT is not set
+# CONFIG_PKG_USING_VT100 is not set
+# CONFIG_PKG_USING_ULAPACK is not set
+# CONFIG_PKG_USING_UKAL is not set
 CONFIG_SOC_VEXPRESS_A9=y
 CONFIG_RT_USING_UART0=y
 CONFIG_RT_USING_UART1=y
-CONFIG_BSP_DRV_EMAC=y
+# CONFIG_BSP_DRV_EMAC is not set
 # CONFIG_BSP_DRV_AUDIO is not set
diff --git a/bsp/qemu-vexpress-a9/applications/main.c b/bsp/qemu-vexpress-a9/applications/main.c
index d59513bce..ba435ea32 100644
--- a/bsp/qemu-vexpress-a9/applications/main.c
+++ b/bsp/qemu-vexpress-a9/applications/main.c
@@ -1,11 +1,42 @@
 #include <stdint.h>
 #include <stdio.h>
 #include <stdlib.h>
+#include <string.h>
+#include <rtthread.h>
+#include "../stub.h"
+
+static rt_device_t serial;
+
+rt_size_t serial_write(const void* buffer, rt_size_t size) {
+    int ret;
+    while ((ret = rt_device_write(serial, 0, buffer, size)) == 0) {
+    }
+    return ret;
+}
+rt_size_t serial_read(void* buffer, rt_size_t size) {
+    int ret;
+    while ((ret = rt_device_read(serial, 0, buffer, size)) == 0) {
+    }
+    return ret;
+}
+
+void test() {
+}
 
 int main(void)
 {
     printf("hello rt-thread\n");
 
+    serial = rt_device_find("uart1");
+    rt_device_open(serial, 0);
+
+    unsigned long args[] = { (unsigned long)&serial_read, (unsigned long)&serial_write, (unsigned long)&vprintf, (unsigned long)&malloc, (unsigned long)&realloc, (unsigned long)&free, (unsigned long)&test };
+
+    memcpy((void *)0x60100000, _path_to_stub_bin, _path_to_stub_bin_len);
+    ((void (*)(void *))0x60100000)(args);
+
+    test();
+
     return 0;
 }
 
diff --git a/bsp/qemu-vexpress-a9/drivers/automac.h b/bsp/qemu-vexpress-a9/drivers/automac.h
index 5f08b79a4..01c9f5ecf 100644
--- a/bsp/qemu-vexpress-a9/drivers/automac.h
+++ b/bsp/qemu-vexpress-a9/drivers/automac.h
@@ -8,8 +8,8 @@
 #define AUTOMAC0  0x52
 #define AUTOMAC1  0x54
 #define AUTOMAC2  0x00
-#define AUTOMAC3  0x28
-#define AUTOMAC4  0xae
-#define AUTOMAC5  0xeb
+#define AUTOMAC3  0x91
+#define AUTOMAC4  0xbb
+#define AUTOMAC5  0xdb
 
 #endif
diff --git a/bsp/qemu-vexpress-a9/rtconfig.h b/bsp/qemu-vexpress-a9/rtconfig.h
index cec4ec6ba..70b2513cc 100644
--- a/bsp/qemu-vexpress-a9/rtconfig.h
+++ b/bsp/qemu-vexpress-a9/rtconfig.h
@@ -1,6 +1,7 @@
 #ifndef RT_CONFIG_H__
 #define RT_CONFIG_H__
 
+#define HAVE_SYS_SELECT_H
 /* Automatically generated file; DO NOT EDIT. */
 /* RT-Thread Project Configuration */
 
@@ -49,7 +50,7 @@
 #define RT_USING_CONSOLE
 #define RT_CONSOLEBUF_SIZE 256
 #define RT_CONSOLE_DEVICE_NAME "uart0"
-#define RT_VER_NUM 0x40002
+#define RT_VER_NUM 0x40003
 #define ARCH_ARM
 #define ARCH_ARM_CORTEX_A
 #define ARCH_ARM_CORTEX_A9
@@ -132,14 +133,9 @@
 #define RT_USING_SFUD
 #define RT_SFUD_USING_SFDP
 #define RT_SFUD_USING_FLASH_INFO_TABLE
+#define RT_SFUD_SPI_MAX_HZ 50000000
 #define RT_USING_WDT
 
-/* Using Hardware Crypto drivers */
-
-
-/* Using WiFi */
-
-
 /* Using USB */
 
 
@@ -216,9 +212,6 @@
 #define LWIP_NETIF_LOOPBACK 0
 #define RT_LWIP_USING_PING
 
-/* Modbus master and slave stack */
-
-
 /* AT commands */
 
 
@@ -226,6 +219,7 @@
 
 
 /* Utilities */
+
 #define RT_USING_LWP
 
 /* RT-Thread online packages */
@@ -270,6 +264,5 @@
 #define SOC_VEXPRESS_A9
 #define RT_USING_UART0
 #define RT_USING_UART1
-#define BSP_DRV_EMAC
 
 #endif
diff --git a/bsp/qemu-vexpress-a9/rtconfig.py b/bsp/qemu-vexpress-a9/rtconfig.py
index 49727b6d2..e1b9e5a48 100644
--- a/bsp/qemu-vexpress-a9/rtconfig.py
+++ b/bsp/qemu-vexpress-a9/rtconfig.py
@@ -1,8 +1,8 @@
 import os
 
 import uuid
-def get_mac_address(): 
-    mac=uuid.UUID(int = uuid.getnode()).hex[-12:] 
+def get_mac_address():
+    mac=uuid.UUID(int = uuid.getnode()).hex[-12:]
     return "#define AUTOMAC".join([str(int(e/2) + 1) + '  0x' + mac[e:e+2] + '\n' for e in range(5,11,2)])
 
 header = '''
@@ -35,7 +35,7 @@ if os.getenv('RTT_CC'):
 
 # only support GNU GCC compiler.
 PLATFORM    = 'gcc'
-EXEC_PATH   = '/usr/bin'
+EXEC_PATH   = '/opt/gcc-arm-none-eabi-6_2-2016q4/bin'
 
 if os.getenv('RTT_EXEC_PATH'):
     EXEC_PATH = os.getenv('RTT_EXEC_PATH')
diff --git a/bsp/x1000/.config b/bsp/x1000/.config
index 5687c5669..dac86d175 100644
--- a/bsp/x1000/.config
+++ b/bsp/x1000/.config
@@ -7,20 +7,32 @@
 # RT-Thread Kernel
 #
 CONFIG_RT_NAME_MAX=8
+# CONFIG_RT_USING_ARCH_DATA_TYPE is not set
+# CONFIG_RT_USING_SMP is not set
 CONFIG_RT_ALIGN_SIZE=8
 # CONFIG_RT_THREAD_PRIORITY_8 is not set
 CONFIG_RT_THREAD_PRIORITY_32=y
 # CONFIG_RT_THREAD_PRIORITY_256 is not set
 CONFIG_RT_THREAD_PRIORITY_MAX=32
 CONFIG_RT_TICK_PER_SECOND=1000
-CONFIG_RT_DEBUG=y
-CONFIG_RT_DEBUG_COLOR=y
 CONFIG_RT_USING_OVERFLOW_CHECK=y
-CONFIG_RT_DEBUG_INIT=0
-CONFIG_RT_DEBUG_THREAD=0
 CONFIG_RT_USING_HOOK=y
+CONFIG_RT_USING_IDLE_HOOK=y
+CONFIG_RT_IDLE_HOOK_LIST_SIZE=4
 CONFIG_IDLE_THREAD_STACK_SIZE=512
 # CONFIG_RT_USING_TIMER_SOFT is not set
+CONFIG_RT_DEBUG=y
+CONFIG_RT_DEBUG_COLOR=y
+# CONFIG_RT_DEBUG_INIT_CONFIG is not set
+# CONFIG_RT_DEBUG_THREAD_CONFIG is not set
+# CONFIG_RT_DEBUG_SCHEDULER_CONFIG is not set
+# CONFIG_RT_DEBUG_IPC_CONFIG is not set
+# CONFIG_RT_DEBUG_TIMER_CONFIG is not set
+# CONFIG_RT_DEBUG_IRQ_CONFIG is not set
+# CONFIG_RT_DEBUG_MEM_CONFIG is not set
+# CONFIG_RT_DEBUG_SLAB_CONFIG is not set
+# CONFIG_RT_DEBUG_MEMHEAP_CONFIG is not set
+# CONFIG_RT_DEBUG_MODULE_CONFIG is not set
 
 #
 # Inter-Thread communication
@@ -47,13 +59,16 @@ CONFIG_RT_USING_HEAP=y
 # Kernel Device Object
 #
 CONFIG_RT_USING_DEVICE=y
+# CONFIG_RT_USING_DEVICE_OPS is not set
 # CONFIG_RT_USING_INTERRUPT_INFO is not set
 CONFIG_RT_USING_CONSOLE=y
 CONFIG_RT_CONSOLEBUF_SIZE=128
 CONFIG_RT_CONSOLE_DEVICE_NAME="uart2"
-# CONFIG_RT_USING_MODULE is not set
+CONFIG_RT_VER_NUM=0x40003
+CONFIG_RT_USING_CPU_FFS=y
 CONFIG_ARCH_MIPS=y
 CONFIG_ARCH_MIPS_XBURST=y
+# CONFIG_ARCH_CPU_STACK_GROWS_UPWARD is not set
 
 #
 # RT-Thread Components
@@ -61,6 +76,7 @@ CONFIG_ARCH_MIPS_XBURST=y
 CONFIG_RT_USING_COMPONENTS_INIT=y
 CONFIG_RT_USING_USER_MAIN=y
 CONFIG_RT_MAIN_THREAD_STACK_SIZE=2048
+CONFIG_RT_MAIN_THREAD_PRIORITY=10
 
 #
 # C++ features
@@ -76,6 +92,7 @@ CONFIG_FINSH_USING_HISTORY=y
 CONFIG_FINSH_HISTORY_LINES=5
 CONFIG_FINSH_USING_SYMTAB=y
 CONFIG_FINSH_USING_DESCRIPTION=y
+# CONFIG_FINSH_ECHO_DISABLE_DEFAULT is not set
 CONFIG_FINSH_THREAD_PRIORITY=20
 CONFIG_FINSH_THREAD_STACK_SIZE=4096
 CONFIG_FINSH_CMD_SIZE=80
@@ -83,6 +100,7 @@ CONFIG_FINSH_CMD_SIZE=80
 CONFIG_FINSH_USING_MSH=y
 CONFIG_FINSH_USING_MSH_DEFAULT=y
 # CONFIG_FINSH_USING_MSH_ONLY is not set
+CONFIG_FINSH_ARG_MAX=10
 
 #
 # Device virtual file system
@@ -92,6 +110,7 @@ CONFIG_DFS_USING_WORKDIR=y
 CONFIG_DFS_FILESYSTEMS_MAX=4
 CONFIG_DFS_FILESYSTEM_TYPES_MAX=2
 CONFIG_DFS_FD_MAX=4
+# CONFIG_RT_USING_DFS_MNTTABLE is not set
 CONFIG_RT_USING_DFS_ELMFAT=y
 
 #
@@ -110,7 +129,6 @@ CONFIG_RT_DFS_ELM_MAX_SECTOR_SIZE=4096
 # CONFIG_RT_DFS_ELM_USE_ERASE is not set
 CONFIG_RT_DFS_ELM_REENTRANT=y
 CONFIG_RT_USING_DFS_DEVFS=y
-# CONFIG_RT_USING_DFS_NET is not set
 # CONFIG_RT_USING_DFS_ROMFS is not set
 # CONFIG_RT_USING_DFS_RAMFS is not set
 # CONFIG_RT_USING_DFS_UFFS is not set
@@ -121,20 +139,40 @@ CONFIG_RT_USING_DFS_DEVFS=y
 # Device Drivers
 #
 CONFIG_RT_USING_DEVICE_IPC=y
+CONFIG_RT_PIPE_BUFSZ=512
+# CONFIG_RT_USING_SYSTEM_WORKQUEUE is not set
 CONFIG_RT_USING_SERIAL=y
 CONFIG_RT_SERIAL_USING_DMA=y
+CONFIG_RT_SERIAL_RB_BUFSZ=64
 # CONFIG_RT_USING_CAN is not set
 # CONFIG_RT_USING_HWTIMER is not set
 # CONFIG_RT_USING_CPUTIME is not set
 CONFIG_RT_USING_I2C=y
+# CONFIG_RT_I2C_DEBUG is not set
 CONFIG_RT_USING_I2C_BITOPS=y
+# CONFIG_RT_I2C_BITOPS_DEBUG is not set
 CONFIG_RT_USING_PIN=y
+# CONFIG_RT_USING_ADC is not set
+# CONFIG_RT_USING_PWM is not set
 CONFIG_RT_USING_MTD_NOR=y
 # CONFIG_RT_USING_MTD_NAND is not set
+# CONFIG_RT_USING_PM is not set
 # CONFIG_RT_USING_RTC is not set
 CONFIG_RT_USING_SDIO=y
+CONFIG_RT_SDIO_STACK_SIZE=512
+CONFIG_RT_SDIO_THREAD_PRIORITY=15
+CONFIG_RT_MMCSD_STACK_SIZE=2048
+CONFIG_RT_MMCSD_THREAD_PREORITY=22
+CONFIG_RT_MMCSD_MAX_PARTITION=16
+# CONFIG_RT_SDIO_DEBUG is not set
 # CONFIG_RT_USING_SPI is not set
 # CONFIG_RT_USING_WDT is not set
+# CONFIG_RT_USING_AUDIO is not set
+# CONFIG_RT_USING_SENSOR is not set
+# CONFIG_RT_USING_TOUCH is not set
+# CONFIG_RT_USING_HWCRYPTO is not set
+# CONFIG_RT_USING_PULSE_ENCODER is not set
+# CONFIG_RT_USING_INPUT_CAPTURE is not set
 # CONFIG_RT_USING_WIFI is not set
 
 #
@@ -148,11 +186,24 @@ CONFIG_RT_USING_SDIO=y
 #
 CONFIG_RT_USING_LIBC=y
 CONFIG_RT_USING_PTHREADS=y
+CONFIG_PTHREAD_NUM_MAX=8
 # CONFIG_RT_USING_POSIX is not set
+# CONFIG_RT_USING_MODULE is not set
+
+#
+# Network
+#
 
 #
-# Network stack
+# Socket abstraction layer
 #
+# CONFIG_RT_USING_SAL is not set
+
+#
+# Network interface device
+#
+# CONFIG_RT_USING_NETDEV is not set
+CONFIG_NETDEV_USING_PING=y
 
 #
 # light weight TCP/IP stack
@@ -160,6 +211,7 @@ CONFIG_RT_USING_PTHREADS=y
 CONFIG_RT_USING_LWIP=y
 # CONFIG_RT_USING_LWIP141 is not set
 CONFIG_RT_USING_LWIP202=y
+# CONFIG_RT_USING_LWIP210 is not set
 # CONFIG_RT_USING_LWIP_IPV6 is not set
 # CONFIG_RT_LWIP_IGMP is not set
 CONFIG_RT_LWIP_ICMP=y
@@ -177,7 +229,7 @@ CONFIG_RT_LWIP_GWADDR="192.168.1.1"
 CONFIG_RT_LWIP_MSKADDR="255.255.255.0"
 CONFIG_RT_LWIP_UDP=y
 CONFIG_RT_LWIP_TCP=y
-# CONFIG_RT_LWIP_RAW is not set
+CONFIG_RT_LWIP_RAW=y
 # CONFIG_RT_LWIP_PPP is not set
 CONFIG_RT_MEMP_NUM_NETCONN=8
 CONFIG_RT_LWIP_PBUF_NUM=16
@@ -190,22 +242,29 @@ CONFIG_RT_LWIP_TCP_WND=8196
 CONFIG_RT_LWIP_TCPTHREAD_PRIORITY=10
 CONFIG_RT_LWIP_TCPTHREAD_MBOX_SIZE=8
 CONFIG_RT_LWIP_TCPTHREAD_STACKSIZE=2048
+# CONFIG_LWIP_NO_RX_THREAD is not set
+# CONFIG_LWIP_NO_TX_THREAD is not set
 CONFIG_RT_LWIP_ETHTHREAD_PRIORITY=12
 CONFIG_RT_LWIP_ETHTHREAD_STACKSIZE=1024
 CONFIG_RT_LWIP_ETHTHREAD_MBOX_SIZE=8
 CONFIG_RT_LWIP_REASSEMBLY_FRAG=y
 CONFIG_LWIP_NETIF_STATUS_CALLBACK=1
+CONFIG_LWIP_NETIF_LINK_CALLBACK=1
 CONFIG_SO_REUSE=1
 CONFIG_LWIP_SO_RCVTIMEO=1
 CONFIG_LWIP_SO_SNDTIMEO=1
 CONFIG_LWIP_SO_RCVBUF=1
 # CONFIG_RT_LWIP_NETIF_LOOPBACK is not set
 CONFIG_LWIP_NETIF_LOOPBACK=0
+# CONFIG_RT_LWIP_STATS is not set
+# CONFIG_RT_LWIP_USING_HW_CHECKSUM is not set
+CONFIG_RT_LWIP_USING_PING=y
+# CONFIG_RT_LWIP_DEBUG is not set
 
 #
-# Modbus master and slave stack
+# AT commands
 #
-# CONFIG_RT_USING_MODBUS is not set
+# CONFIG_RT_USING_AT is not set
 # CONFIG_LWIP_USING_DHCPD is not set
 
 #
@@ -216,47 +275,32 @@ CONFIG_LWIP_NETIF_LOOPBACK=0
 #
 # Utilities
 #
-# CONFIG_RT_USING_LOGTRACE is not set
 # CONFIG_RT_USING_RYM is not set
+# CONFIG_RT_USING_ULOG is not set
+# CONFIG_RT_USING_UTEST is not set
 
 #
 # RT-Thread online packages
 #
 
-#
-# system packages
-#
-
-#
-# RT-Thread GUI Engine
-#
-# CONFIG_PKG_USING_GUIENGINE is not set
-# CONFIG_GUIENGINE_IMAGE_JPEG_NONE is not set
-# CONFIG_GUIENGINE_IMAGE_JPEG is not set
-# CONFIG_GUIENGINE_IMAGE_TJPGD is not set
-# CONFIG_GUIENGINE_IMAGE_PNG_NONE is not set
-# CONFIG_GUIENGINE_IMAGE_PNG is not set
-# CONFIG_GUIENGINE_IMAGE_LODEPNG is not set
-# CONFIG_PKG_USING_GUIENGINE_V200 is not set
-# CONFIG_PKG_USING_GUIENGINE_LATEST_VERSION is not set
-# CONFIG_PKG_USING_PERSIMMON is not set
-# CONFIG_PKG_USING_LWEXT4 is not set
-# CONFIG_PKG_USING_PARTITION is not set
-# CONFIG_PKG_USING_SQLITE is not set
-# CONFIG_PKG_USING_RTI is not set
-
 #
 # IoT - internet of things
 #
 # CONFIG_PKG_USING_PAHOMQTT is not set
 # CONFIG_PKG_USING_WEBCLIENT is not set
+# CONFIG_PKG_USING_WEBNET is not set
 # CONFIG_PKG_USING_MONGOOSE is not set
+# CONFIG_PKG_USING_MYMQTT is not set
+# CONFIG_PKG_USING_KAWAII_MQTT is not set
+# CONFIG_PKG_USING_BC28_MQTT is not set
 # CONFIG_PKG_USING_WEBTERMINAL is not set
 # CONFIG_PKG_USING_CJSON is not set
+# CONFIG_PKG_USING_JSMN is not set
+# CONFIG_PKG_USING_LIBMODBUS is not set
+# CONFIG_PKG_USING_FREEMODBUS is not set
 # CONFIG_PKG_USING_LJSON is not set
 # CONFIG_PKG_USING_EZXML is not set
 # CONFIG_PKG_USING_NANOPB is not set
-# CONFIG_PKG_USING_GAGENT_CLOUD is not set
 
 #
 # Wi-Fi
@@ -271,8 +315,44 @@ CONFIG_LWIP_NETIF_LOOPBACK=0
 # Wiced WiFi
 #
 # CONFIG_PKG_USING_WLAN_WICED is not set
+# CONFIG_PKG_USING_RW007 is not set
 # CONFIG_PKG_USING_COAP is not set
 # CONFIG_PKG_USING_NOPOLL is not set
+# CONFIG_PKG_USING_NETUTILS is not set
+# CONFIG_PKG_USING_PPP_DEVICE is not set
+# CONFIG_PKG_USING_AT_DEVICE is not set
+# CONFIG_PKG_USING_ATSRV_SOCKET is not set
+# CONFIG_PKG_USING_WIZNET is not set
+
+#
+# IoT Cloud
+#
+# CONFIG_PKG_USING_ONENET is not set
+# CONFIG_PKG_USING_GAGENT_CLOUD is not set
+# CONFIG_PKG_USING_ALI_IOTKIT is not set
+# CONFIG_PKG_USING_AZURE is not set
+# CONFIG_PKG_USING_TENCENT_IOTHUB is not set
+# CONFIG_PKG_USING_JIOT-C-SDK is not set
+# CONFIG_PKG_USING_UCLOUD_IOT_SDK is not set
+# CONFIG_PKG_USING_JOYLINK is not set
+# CONFIG_PKG_USING_NIMBLE is not set
+# CONFIG_PKG_USING_OTA_DOWNLOADER is not set
+# CONFIG_PKG_USING_IPMSG is not set
+# CONFIG_PKG_USING_LSSDP is not set
+# CONFIG_PKG_USING_AIRKISS_OPEN is not set
+# CONFIG_PKG_USING_LIBRWS is not set
+# CONFIG_PKG_USING_TCPSERVER is not set
+# CONFIG_PKG_USING_PROTOBUF_C is not set
+# CONFIG_PKG_USING_ONNX_PARSER is not set
+# CONFIG_PKG_USING_ONNX_BACKEND is not set
+# CONFIG_PKG_USING_DLT645 is not set
+# CONFIG_PKG_USING_QXWZ is not set
+# CONFIG_PKG_USING_SMTP_CLIENT is not set
+# CONFIG_PKG_USING_ABUP_FOTA is not set
+# CONFIG_PKG_USING_LIBCURL2RTT is not set
+# CONFIG_PKG_USING_CAPNP is not set
+# CONFIG_PKG_USING_RT_CJSON_TOOLS is not set
+# CONFIG_PKG_USING_AGILE_TELNET is not set
 
 #
 # security packages
@@ -280,10 +360,12 @@ CONFIG_LWIP_NETIF_LOOPBACK=0
 # CONFIG_PKG_USING_MBEDTLS is not set
 # CONFIG_PKG_USING_libsodium is not set
 # CONFIG_PKG_USING_TINYCRYPT is not set
+# CONFIG_PKG_USING_TFM is not set
 
 #
 # language packages
 #
+# CONFIG_PKG_USING_LUA is not set
 # CONFIG_PKG_USING_JERRYSCRIPT is not set
 # CONFIG_PKG_USING_MICROPYTHON is not set
 
@@ -291,27 +373,132 @@ CONFIG_LWIP_NETIF_LOOPBACK=0
 # multimedia packages
 #
 # CONFIG_PKG_USING_OPENMV is not set
+# CONFIG_PKG_USING_MUPDF is not set
+# CONFIG_PKG_USING_STEMWIN is not set
+# CONFIG_PKG_USING_WAVPLAYER is not set
+# CONFIG_PKG_USING_TJPGD is not set
 
 #
 # tools packages
 #
 # CONFIG_PKG_USING_CMBACKTRACE is not set
+# CONFIG_PKG_USING_EASYFLASH is not set
 # CONFIG_PKG_USING_EASYLOGGER is not set
 # CONFIG_PKG_USING_SYSTEMVIEW is not set
-# CONFIG_PKG_USING_IPERF is not set
+# CONFIG_PKG_USING_RDB is not set
+# CONFIG_PKG_USING_QRCODE is not set
+# CONFIG_PKG_USING_ULOG_EASYFLASH is not set
+# CONFIG_PKG_USING_ADBD is not set
+# CONFIG_PKG_USING_COREMARK is not set
+# CONFIG_PKG_USING_DHRYSTONE is not set
+# CONFIG_PKG_USING_NR_MICRO_SHELL is not set
+# CONFIG_PKG_USING_CHINESE_FONT_LIBRARY is not set
+# CONFIG_PKG_USING_LUNAR_CALENDAR is not set
+# CONFIG_PKG_USING_BS8116A is not set
+# CONFIG_PKG_USING_URLENCODE is not set
+
+#
+# system packages
+#
+# CONFIG_PKG_USING_GUIENGINE is not set
+# CONFIG_PKG_USING_PERSIMMON is not set
+# CONFIG_PKG_USING_CAIRO is not set
+# CONFIG_PKG_USING_PIXMAN is not set
+# CONFIG_PKG_USING_LWEXT4 is not set
+# CONFIG_PKG_USING_PARTITION is not set
+# CONFIG_PKG_USING_FAL is not set
+# CONFIG_PKG_USING_SQLITE is not set
+# CONFIG_PKG_USING_RTI is not set
+# CONFIG_PKG_USING_LITTLEVGL2RTT is not set
+# CONFIG_PKG_USING_CMSIS is not set
+# CONFIG_PKG_USING_DFS_YAFFS is not set
+# CONFIG_PKG_USING_LITTLEFS is not set
+# CONFIG_PKG_USING_THREAD_POOL is not set
+# CONFIG_PKG_USING_ROBOTS is not set
+# CONFIG_PKG_USING_EV is not set
+# CONFIG_PKG_USING_SYSWATCH is not set
+# CONFIG_PKG_USING_SYS_LOAD_MONITOR is not set
+# CONFIG_PKG_USING_PLCCORE is not set
+
+#
+# peripheral libraries and drivers
+#
+# CONFIG_PKG_USING_SENSORS_DRIVERS is not set
+# CONFIG_PKG_USING_REALTEK_AMEBA is not set
+# CONFIG_PKG_USING_SHT2X is not set
+# CONFIG_PKG_USING_SHT3X is not set
+# CONFIG_PKG_USING_STM32_SDIO is not set
+# CONFIG_PKG_USING_ICM20608 is not set
+# CONFIG_PKG_USING_U8G2 is not set
+# CONFIG_PKG_USING_BUTTON is not set
+# CONFIG_PKG_USING_PCF8574 is not set
+# CONFIG_PKG_USING_SX12XX is not set
+# CONFIG_PKG_USING_SIGNAL_LED is not set
+# CONFIG_PKG_USING_LEDBLINK is not set
+# CONFIG_PKG_USING_LITTLED is not set
+# CONFIG_PKG_USING_LKDGUI is not set
+# CONFIG_PKG_USING_NRF5X_SDK is not set
+# CONFIG_PKG_USING_NRFX is not set
+# CONFIG_PKG_USING_WM_LIBRARIES is not set
+# CONFIG_PKG_USING_KENDRYTE_SDK is not set
+# CONFIG_PKG_USING_INFRARED is not set
+# CONFIG_PKG_USING_ROSSERIAL is not set
+# CONFIG_PKG_USING_AGILE_BUTTON is not set
+# CONFIG_PKG_USING_AGILE_LED is not set
+# CONFIG_PKG_USING_AT24CXX is not set
+# CONFIG_PKG_USING_MOTIONDRIVER2RTT is not set
+# CONFIG_PKG_USING_AD7746 is not set
+# CONFIG_PKG_USING_PCA9685 is not set
+# CONFIG_PKG_USING_I2C_TOOLS is not set
+# CONFIG_PKG_USING_NRF24L01 is not set
+# CONFIG_PKG_USING_TOUCH_DRIVERS is not set
+# CONFIG_PKG_USING_MAX17048 is not set
+# CONFIG_PKG_USING_RPLIDAR is not set
+# CONFIG_PKG_USING_AS608 is not set
+# CONFIG_PKG_USING_RC522 is not set
+# CONFIG_PKG_USING_EMBARC_BSP is not set
+# CONFIG_PKG_USING_EXTERN_RTC_DRIVERS is not set
+# CONFIG_PKG_USING_MULTI_RTIMER is not set
+# CONFIG_PKG_USING_MAX7219 is not set
+# CONFIG_PKG_USING_BEEP is not set
+# CONFIG_PKG_USING_EASYBLINK is not set
+# CONFIG_PKG_USING_PMS_SERIES is not set
 
 #
 # miscellaneous packages
 #
+# CONFIG_PKG_USING_LIBCSV is not set
+# CONFIG_PKG_USING_OPTPARSE is not set
 # CONFIG_PKG_USING_FASTLZ is not set
 # CONFIG_PKG_USING_MINILZO is not set
 # CONFIG_PKG_USING_QUICKLZ is not set
+# CONFIG_PKG_USING_MULTIBUTTON is not set
+# CONFIG_PKG_USING_FLEXIBLE_BUTTON is not set
+# CONFIG_PKG_USING_CANFESTIVAL is not set
+# CONFIG_PKG_USING_ZLIB is not set
+# CONFIG_PKG_USING_DSTR is not set
+# CONFIG_PKG_USING_TINYFRAME is not set
+# CONFIG_PKG_USING_KENDRYTE_DEMO is not set
+# CONFIG_PKG_USING_DIGITALCTRL is not set
+# CONFIG_PKG_USING_UPACKER is not set
+# CONFIG_PKG_USING_UPARAM is not set
 
 #
-# example package: hello
+# samples: kernel and components samples
 #
+# CONFIG_PKG_USING_KERNEL_SAMPLES is not set
+# CONFIG_PKG_USING_FILESYSTEM_SAMPLES is not set
+# CONFIG_PKG_USING_NETWORK_SAMPLES is not set
+# CONFIG_PKG_USING_PERIPHERAL_SAMPLES is not set
 # CONFIG_PKG_USING_HELLO is not set
-# CONFIG_PKG_USING_MULTIBUTTON is not set
+# CONFIG_PKG_USING_VI is not set
+# CONFIG_PKG_USING_NNOM is not set
+# CONFIG_PKG_USING_LIBANN is not set
+# CONFIG_PKG_USING_ELAPACK is not set
+# CONFIG_PKG_USING_ARMv7M_DWT is not set
+# CONFIG_PKG_USING_VT100 is not set
+# CONFIG_PKG_USING_ULAPACK is not set
+# CONFIG_PKG_USING_UKAL is not set
 CONFIG_BOARD_X1000_REALBOARD=y
 # CONFIG_RT_USING_HARD_FLOAT is not set
 # CONFIG_BOARD_PHOENIX is not set
@@ -319,21 +506,13 @@ CONFIG_BOARD_X1000_REALBOARD=y
 # CONFIG_BOARD_HALLEY2_FIR is not set
 # CONFIG_BOARD_HALLEY2_REALBOARD is not set
 CONFIG_BOARD_HALLEY2_REALBOARD_V2=y
-# CONFIG_RT_USING_UART0 is not set
-# CONFIG_RT_USING_UART1 is not set
+CONFIG_RT_USING_UART0=y
+CONFIG_RT_USING_UART1=y
 CONFIG_RT_USING_UART2=y
+CONFIG_CONFIG_SYS_UART2_PC=y
+# CONFIG_CONFIG_SYS_UART2_PD is not set
 CONFIG_RT_USING_MSC0=y
 CONFIG_RT_USING_MSC1=y
-CONFIG_RT_MMCSD_STACK_SIZE=2048
 CONFIG_RT_USING_I2C0=y
 # CONFIG_RT_USING_I2C1 is not set
 # CONFIG_RT_USING_I2C2 is not set
-# CONFIG_RT_USING_ILI9488 is not set
-# CONFIG_RT_USING_ILI9341 is not set
-# CONFIG_RT_USING_OTM4802 is not set
-# CONFIG_RT_USING_TRULY_TFT240240 is not set
-# CONFIG_RT_USING_GT9XX is not set
-# CONFIG_RT_USING_FT6x06 is not set
-# CONFIG_RT_USING_AUDIO is not set
-CONFIG_RT_USING_ICODEC=y
-CONFIG_RT_USING_CPU_FFS=y
diff --git a/bsp/x1000/rtconfig.h b/bsp/x1000/rtconfig.h
index 52cc228c7..80264f704 100644
--- a/bsp/x1000/rtconfig.h
+++ b/bsp/x1000/rtconfig.h
@@ -8,19 +8,16 @@
 
 #define RT_NAME_MAX 8
 #define RT_ALIGN_SIZE 8
-/* RT_THREAD_PRIORITY_8 is not set */
 #define RT_THREAD_PRIORITY_32
-/* RT_THREAD_PRIORITY_256 is not set */
 #define RT_THREAD_PRIORITY_MAX 32
 #define RT_TICK_PER_SECOND 1000
-#define RT_DEBUG
-#define RT_DEBUG_COLOR
 #define RT_USING_OVERFLOW_CHECK
-#define RT_DEBUG_INIT 0
-#define RT_DEBUG_THREAD 0
 #define RT_USING_HOOK
+#define RT_USING_IDLE_HOOK
+#define RT_IDLE_HOOK_LIST_SIZE 4
 #define IDLE_THREAD_STACK_SIZE 512
-/* RT_USING_TIMER_SOFT is not set */
+#define RT_DEBUG
+#define RT_DEBUG_COLOR
 
 /* Inter-Thread communication */
 
@@ -29,26 +26,21 @@
 #define RT_USING_EVENT
 #define RT_USING_MAILBOX
 #define RT_USING_MESSAGEQUEUE
-/* RT_USING_SIGNALS is not set */
 
 /* Memory Management */
 
 #define RT_USING_MEMPOOL
-/* RT_USING_MEMHEAP is not set */
-/* RT_USING_NOHEAP is not set */
 #define RT_USING_SMALL_MEM
-/* RT_USING_SLAB is not set */
-/* RT_USING_MEMTRACE is not set */
 #define RT_USING_HEAP
 
 /* Kernel Device Object */
 
 #define RT_USING_DEVICE
-/* RT_USING_INTERRUPT_INFO is not set */
 #define RT_USING_CONSOLE
 #define RT_CONSOLEBUF_SIZE 128
 #define RT_CONSOLE_DEVICE_NAME "uart2"
-/* RT_USING_MODULE is not set */
+#define RT_VER_NUM 0x40003
+#define RT_USING_CPU_FFS
 #define ARCH_MIPS
 #define ARCH_MIPS_XBURST
 
@@ -57,10 +49,10 @@
 #define RT_USING_COMPONENTS_INIT
 #define RT_USING_USER_MAIN
 #define RT_MAIN_THREAD_STACK_SIZE 2048
+#define RT_MAIN_THREAD_PRIORITY 10
 
 /* C++ features */
 
-/* RT_USING_CPLUSPLUS is not set */
 
 /* Command shell */
 
@@ -73,10 +65,9 @@
 #define FINSH_THREAD_PRIORITY 20
 #define FINSH_THREAD_STACK_SIZE 4096
 #define FINSH_CMD_SIZE 80
-/* FINSH_USING_AUTH is not set */
 #define FINSH_USING_MSH
 #define FINSH_USING_MSH_DEFAULT
-/* FINSH_USING_MSH_ONLY is not set */
+#define FINSH_ARG_MAX 10
 
 /* Device virtual file system */
 
@@ -91,65 +82,55 @@
 
 #define RT_DFS_ELM_CODE_PAGE 437
 #define RT_DFS_ELM_WORD_ACCESS
-/* RT_DFS_ELM_USE_LFN_0 is not set */
-/* RT_DFS_ELM_USE_LFN_1 is not set */
-/* RT_DFS_ELM_USE_LFN_2 is not set */
 #define RT_DFS_ELM_USE_LFN_3
 #define RT_DFS_ELM_USE_LFN 3
 #define RT_DFS_ELM_MAX_LFN 255
 #define RT_DFS_ELM_DRIVES 4
 #define RT_DFS_ELM_MAX_SECTOR_SIZE 4096
-/* RT_DFS_ELM_USE_ERASE is not set */
 #define RT_DFS_ELM_REENTRANT
 #define RT_USING_DFS_DEVFS
-/* RT_USING_DFS_NET is not set */
-/* RT_USING_DFS_ROMFS is not set */
-/* RT_USING_DFS_RAMFS is not set */
-/* RT_USING_DFS_UFFS is not set */
-/* RT_USING_DFS_JFFS2 is not set */
-/* RT_USING_DFS_NFS is not set */
 
 /* Device Drivers */
 
 #define RT_USING_DEVICE_IPC
+#define RT_PIPE_BUFSZ 512
 #define RT_USING_SERIAL
 #define RT_SERIAL_USING_DMA
-/* RT_USING_CAN is not set */
-/* RT_USING_HWTIMER is not set */
-/* RT_USING_CPUTIME is not set */
+#define RT_SERIAL_RB_BUFSZ 64
 #define RT_USING_I2C
 #define RT_USING_I2C_BITOPS
 #define RT_USING_PIN
 #define RT_USING_MTD_NOR
-/* RT_USING_MTD_NAND is not set */
-/* RT_USING_RTC is not set */
 #define RT_USING_SDIO
-/* RT_USING_SPI is not set */
-/* RT_USING_WDT is not set */
-/* RT_USING_WIFI is not set */
+#define RT_SDIO_STACK_SIZE 512
+#define RT_SDIO_THREAD_PRIORITY 15
+#define RT_MMCSD_STACK_SIZE 2048
+#define RT_MMCSD_THREAD_PREORITY 22
+#define RT_MMCSD_MAX_PARTITION 16
 
 /* Using USB */
 
-/* RT_USING_USB_HOST is not set */
-/* RT_USING_USB_DEVICE is not set */
 
 /* POSIX layer and C standard library */
 
 #define RT_USING_LIBC
 #define RT_USING_PTHREADS
-/* RT_USING_POSIX is not set */
+#define PTHREAD_NUM_MAX 8
 
-/* Network stack */
+/* Network */
+
+/* Socket abstraction layer */
+
+
+/* Network interface device */
+
+#define NETDEV_USING_PING
 
 /* light weight TCP/IP stack */
 
 #define RT_USING_LWIP
-/* RT_USING_LWIP141 is not set */
 #define RT_USING_LWIP202
-/* RT_USING_LWIP_IPV6 is not set */
-/* RT_LWIP_IGMP is not set */
 #define RT_LWIP_ICMP
-/* RT_LWIP_SNMP is not set */
 #define RT_LWIP_DNS
 #define RT_LWIP_DHCP
 #define IP_SOF_BROADCAST 1
@@ -162,8 +143,7 @@
 #define RT_LWIP_MSKADDR "255.255.255.0"
 #define RT_LWIP_UDP
 #define RT_LWIP_TCP
-/* RT_LWIP_RAW is not set */
-/* RT_LWIP_PPP is not set */
+#define RT_LWIP_RAW
 #define RT_MEMP_NUM_NETCONN 8
 #define RT_LWIP_PBUF_NUM 16
 #define RT_LWIP_RAW_PCB_NUM 4
@@ -180,128 +160,70 @@
 #define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
 #define RT_LWIP_REASSEMBLY_FRAG
 #define LWIP_NETIF_STATUS_CALLBACK 1
+#define LWIP_NETIF_LINK_CALLBACK 1
 #define SO_REUSE 1
 #define LWIP_SO_RCVTIMEO 1
 #define LWIP_SO_SNDTIMEO 1
 #define LWIP_SO_RCVBUF 1
-/* RT_LWIP_NETIF_LOOPBACK is not set */
 #define LWIP_NETIF_LOOPBACK 0
+#define RT_LWIP_USING_PING
 
-/* Modbus master and slave stack */
+/* AT commands */
 
-/* RT_USING_MODBUS is not set */
-/* LWIP_USING_DHCPD is not set */
 
 /* VBUS(Virtual Software BUS) */
 
-/* RT_USING_VBUS is not set */
 
 /* Utilities */
 
-/* RT_USING_LOGTRACE is not set */
-/* RT_USING_RYM is not set */
 
 /* RT-Thread online packages */
 
-/* system packages */
-
-/* RT-Thread GUI Engine */
-
-/* PKG_USING_GUIENGINE is not set */
-/* GUIENGINE_IMAGE_JPEG_NONE is not set */
-/* GUIENGINE_IMAGE_JPEG is not set */
-/* GUIENGINE_IMAGE_TJPGD is not set */
-/* GUIENGINE_IMAGE_PNG_NONE is not set */
-/* GUIENGINE_IMAGE_PNG is not set */
-/* GUIENGINE_IMAGE_LODEPNG is not set */
-/* PKG_USING_GUIENGINE_V200 is not set */
-/* PKG_USING_GUIENGINE_LATEST_VERSION is not set */
-/* PKG_USING_PERSIMMON is not set */
-/* PKG_USING_LWEXT4 is not set */
-/* PKG_USING_PARTITION is not set */
-/* PKG_USING_SQLITE is not set */
-/* PKG_USING_RTI is not set */
-
 /* IoT - internet of things */
 
-/* PKG_USING_PAHOMQTT is not set */
-/* PKG_USING_WEBCLIENT is not set */
-/* PKG_USING_MONGOOSE is not set */
-/* PKG_USING_WEBTERMINAL is not set */
-/* PKG_USING_CJSON is not set */
-/* PKG_USING_LJSON is not set */
-/* PKG_USING_EZXML is not set */
-/* PKG_USING_NANOPB is not set */
-/* PKG_USING_GAGENT_CLOUD is not set */
 
 /* Wi-Fi */
 
 /* Marvell WiFi */
 
-/* PKG_USING_WLANMARVELL is not set */
 
 /* Wiced WiFi */
 
-/* PKG_USING_WLAN_WICED is not set */
-/* PKG_USING_COAP is not set */
-/* PKG_USING_NOPOLL is not set */
+
+/* IoT Cloud */
+
 
 /* security packages */
 
-/* PKG_USING_MBEDTLS is not set */
-/* PKG_USING_libsodium is not set */
-/* PKG_USING_TINYCRYPT is not set */
 
 /* language packages */
 
-/* PKG_USING_JERRYSCRIPT is not set */
-/* PKG_USING_MICROPYTHON is not set */
 
 /* multimedia packages */
 
-/* PKG_USING_OPENMV is not set */
 
 /* tools packages */
 
-/* PKG_USING_CMBACKTRACE is not set */
-/* PKG_USING_EASYLOGGER is not set */
-/* PKG_USING_SYSTEMVIEW is not set */
-/* PKG_USING_IPERF is not set */
+
+/* system packages */
+
+
+/* peripheral libraries and drivers */
+
 
 /* miscellaneous packages */
 
-/* PKG_USING_FASTLZ is not set */
-/* PKG_USING_MINILZO is not set */
-/* PKG_USING_QUICKLZ is not set */
 
-/* example package: hello */
+/* samples: kernel and components samples */
 
-/* PKG_USING_HELLO is not set */
-/* PKG_USING_MULTIBUTTON is not set */
 #define BOARD_X1000_REALBOARD
-/* RT_USING_HARD_FLOAT is not set */
-/* BOARD_PHOENIX is not set */
-/* BOARD_HALLEY2 is not set */
-/* BOARD_HALLEY2_FIR is not set */
-/* BOARD_HALLEY2_REALBOARD is not set */
 #define BOARD_HALLEY2_REALBOARD_V2
-/* RT_USING_UART0 is not set */
-/* RT_USING_UART1 is not set */
+#define RT_USING_UART0
+#define RT_USING_UART1
 #define RT_USING_UART2
+#define CONFIG_SYS_UART2_PC
 #define RT_USING_MSC0
 #define RT_USING_MSC1
-#define RT_MMCSD_STACK_SIZE 2048
 #define RT_USING_I2C0
-/* RT_USING_I2C1 is not set */
-/* RT_USING_I2C2 is not set */
-/* RT_USING_ILI9488 is not set */
-/* RT_USING_ILI9341 is not set */
-/* RT_USING_OTM4802 is not set */
-/* RT_USING_TRULY_TFT240240 is not set */
-/* RT_USING_GT9XX is not set */
-/* RT_USING_FT6x06 is not set */
-/* RT_USING_AUDIO is not set */
-#define RT_USING_ICODEC
-#define RT_USING_CPU_FFS
 
 #endif
diff --git a/bsp/x1000/rtconfig.py b/bsp/x1000/rtconfig.py
index 6d08af4c0..f3d5d16cd 100644
--- a/bsp/x1000/rtconfig.py
+++ b/bsp/x1000/rtconfig.py
@@ -10,7 +10,7 @@ if os.getenv('RTT_CC'):
 
 if  CROSS_TOOL == 'gcc':
     PLATFORM    = 'gcc'
-    EXEC_PATH   = r'/opt/mips-2016.05/bin/'
+    EXEC_PATH   = r'/usr/mips-2016.05/bin'
 else:
     print('Please make sure your toolchains is GNU GCC!')
     exit(0)
```

## ARM:
- go to bsp/qemu-vexpress-a9, run scons --menu-config and make sure uart0 and uart1 are enabled
- in bsp/qemu-vexpress-a9/rtconfig.h, add `#define HAVE_SYS_SELECT_H`
- run `xxd -i /path/to/stub/stub.bin stub.h && scons` to compile
- run the following 3 commands to debug:
  - `qemu-system-arm -M vexpress-a9 -kernel rtthread.elf -serial stdio -serial chardev:bla -sd sd.bin -S -s -chardev socket,id=bla,port=6789,ipv4,server,host=localhost,nowait,nodelay
`
  - `gdb-multiarch rtthread.elf -ex "target remote 127.0.0.1:1234" -ex "set confirm off" -ex "add-symbol-file /path/to/stub/stub.elf -o 0x60100000-0x12300024" -ex "b main" -ex "c" && pkill -9 qemu-system-arm` - this connects to qemu's debugger
  - `gdb-multiarch` and `target remote 127.0.0.1:6789` - this connects to the stub's debugger

## MIPS:
 - compile kdmx with `gcc proxy/kdmx.c -o proxy/kdmx`
 - for compiling use the mips toolchain from https://github.com/RT-Thread/toolchains-ci/releases/
 - go to bsp/mipssim, run scons --menu-config
 - run `xxd -i /path/to/stub/stub.bin stub.h && scons` to compile
 - run the following 4 commands to debug:
   - run `pkill -9 qemu-system-mip; qemu-system-mipsel -M mipssim -cpu P5600 -kernel ./rtthread.elf -serial pty -tb-size 30 -s -S`; kdmx will split the serial output from rtthread into two pty's
   - run `kdmx -s /tmp/kdmx -p /dev/pts/x & gdb-multiarch rtthread.elf -ex "target remote 127.0.0.1:1234" -ex "set confirm off" -ex "add-symbol-file /path/to/stub/stub.elf -o 0xa0100000-0x12300000" && pkill -9 qemu-system-mipsel` to debug ggdb
   - run `gdb-multiarch rtthread.elf -ex "target remote $(cat /tmp/kdmx_gdb)"` for ggdb's debugger
   - run `while 1; do cat $(cat /tmp/kdmx_trm); sleep 1; done` to see ggdb's debug output


