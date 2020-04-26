# Debugging using rt-thread

- Clone it from git@github.com:RT-Thread/rt-thread.git 
- Apply the following patch:

```
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
+    memcpy((void *)0x60100000, _path_to_stub_stub_bin, _path_to_stub_stub_bin_len);
+    ((void (*)(void *))0x60100000)(args);
+
+    test();
+
     return 0;
 }
 
diff --git a/bsp/qemu-vexpress-a9/rtconfig.py b/bsp/qemu-vexpress-a9/rtconfig.py
index 49727b6d2..e1b9e5a48 100644
--- a/bsp/qemu-vexpress-a9/rtconfig.py
+++ b/bsp/qemu-vexpress-a9/rtconfig.py
@@ -35,7 +35,7 @@ if os.getenv('RTT_CC'):
 
 # only support GNU GCC compiler.
 PLATFORM    = 'gcc'
-EXEC_PATH   = '/usr/bin'
+EXEC_PATH   = '/opt/gcc-arm-none-eabi-6_2-2016q4/bin'
 
 if os.getenv('RTT_EXEC_PATH'):
     EXEC_PATH = os.getenv('RTT_EXEC_PATH')
```

- go to bsp/qemu-vexpress-a9, run scons --menu-config and make sure uart0 and uart1 are enabled
- in bsp/qemu-vexpress-a9/rtconfig.h, add `#define HAVE_SYS_SELECT_H`
- run `xxd -i /path/to/stub/stub.bin stub.h && scons` to compile
- run the following 3 commands to debug:
  - `qemu-system-arm -M vexpress-a9 -kernel rtthread.elf -serial stdio -serial chardev:bla -sd sd.bin -S -s -chardev socket,id=bla,port=6789,ipv4,server,host=localhost,nowait,nodelay
`
  - `gdb-multiarch rtthread.elf -ex "target remote 127.0.0.1:1234" -ex "set confirm off" -ex "add-symbol-file /path/to/stub/stub.elf -o 0x60100000-0x12300024" -ex "b main" -ex "c" && pkill -9 qemu-system-arm` - this connects to qemu's debugger
  - `gdb-multiarch` and `target remote 127.0.0.1:6789` - this connects to the stub's debugger

