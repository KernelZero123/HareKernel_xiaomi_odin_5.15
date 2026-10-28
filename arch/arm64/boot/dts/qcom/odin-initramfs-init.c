/* odin bring-up init: minimal, static. Sets up USB gadget serial console
 * so we can watch boot progress over the USB cable, no extra hardware. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>

static int kmsg_fd = -1;
static int tty_fd  = -1;

static void klog(const char *s)
{
	char buf[1200];
	int n = snprintf(buf, sizeof(buf), "ODIN: %s", s);
	if (kmsg_fd >= 0) { write(kmsg_fd, buf, n); }
	if (tty_fd >= 0)  { write(tty_fd, buf, n); write(tty_fd, "\r\n", 2); }
	printf("%s\n", buf);
	fflush(stdout);
}

static void wr(const char *path, const char *val)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0) return;
	write(fd, val, strlen(val));
	close(fd);
}

static void mk(const char *p, int mode) { mkdir(p, mode); }

/* Write contents of a small file to our log */
static void dump(const char *path, const char *label)
{
	char buf[2048]; int fd, n;
	fd = open(path, O_RDONLY);
	if (fd < 0) { klog(label); return; }
	n = read(fd, buf, sizeof(buf)-1);
	close(fd);
	if (n > 0) { buf[n] = 0; klog(buf); }
	else klog(label);
}

static void setup_usb_gadget(void)
{
	mk("/sys/kernel/config", 0755);
	if (mount("configfs", "/sys/kernel/config", "configfs", 0, NULL) < 0 && errno != EBUSY)
		klog("configfs mount failed");

	const char *G = "/sys/kernel/config/usb_gadget/g1";
	mk(G, 0755);
	if (access(G, F_OK) != 0) { klog("no gadget dir (configfs?)"); return; }

	wr("/sys/kernel/config/usb_gadget/g1/idVendor",  "0x18d1");
	wr("/sys/kernel/config/usb_gadget/g1/idProduct", "0xd001");
	wr("/sys/kernel/config/usb_gadget/g1/bcdUSB",    "0x0200");
	wr("/sys/kernel/config/usb_gadget/g1/bcdDevice", "0x0100");

	mk("/sys/kernel/config/usb_gadget/g1/strings/0x409", 0755);
	wr("/sys/kernel/config/usb_gadget/g1/strings/0x409/serialnumber", "odin-bringup");
	wr("/sys/kernel/config/usb_gadget/g1/strings/0x409/manufacturer", "Xiaomi");
	wr("/sys/kernel/config/usb_gadget/g1/strings/0x409/product",      "Mi MIX 4 bringup");

	mk("/sys/kernel/config/usb_gadget/g1/configs/c.1", 0755);
	mk("/sys/kernel/config/usb_gadget/g1/configs/c.1/strings/0x409", 0755);
	wr("/sys/kernel/config/usb_gadget/g1/configs/c.1/strings/0x409/configuration", "ACM");
	wr("/sys/kernel/config/usb_gadget/g1/configs/c.1/MaxPower", "500");

	mk("/sys/kernel/config/usb_gadget/g1/functions/acm.gs0", 0755);
	if (symlink("/sys/kernel/config/usb_gadget/g1/functions/acm.gs0", "/sys/kernel/config/usb_gadget/g1/configs/c.1/acm.gs0") < 0 && errno != EEXIST)
		klog("acm symlink failed");

	/* bind to the first UDC */
	{
		char udc[128] = {0};
		int fd = open("/sys/class/udc", O_RDONLY | O_DIRECTORY);
		if (fd >= 0) {
			/* simplest: readdir not available without dirent; try known names */
			close(fd);
		}
		const char *names[] = {
			"a600000.dwc3", "a600000.usb", NULL
		};
		for (int i = 0; names[i]; i++) {
			char p[256];
			snprintf(p, sizeof(p), "/sys/class/udc/%s", names[i]);
			if (access(p, F_OK) == 0) { snprintf(udc, sizeof(udc), "%s", names[i]); break; }
		}
		if (udc[0]) {
			char msg[160];
			snprintf(msg, sizeof(msg), "UDC found: %s", udc);
			klog(msg);
			wr("/sys/kernel/config/usb_gadget/g1/UDC", udc);
		} else {
			klog("no UDC yet (dwc3 may not have probed)");
		}
	}

	/* give gadget a moment, then open the ACM tty */
	for (int i = 0; i < 30; i++) {
		tty_fd = open("/dev/ttyGS0", O_WRONLY | O_NONBLOCK);
		if (tty_fd >= 0) break;
		usleep(100000);
	}
	if (tty_fd >= 0) klog("USB serial console attached (/dev/ttyGS0)");
	else             klog("ttyGS0 not available, kmsg only");
}

int main(void)
{
	mk("/proc", 0555); mk("/sys", 0555); mk("/dev", 0755);
	mk("/tmp", 0755);  mk("/mnt", 0755);

	mount("proc",    "/proc", "proc",    0, NULL);
	mount("sysfs",   "/sys",  "sysfs",   0, NULL);
	mount("devtmpfs","/dev",  "devtmpfs",0, NULL);
	mount("devpts",  "/dev/pts","devpts",0, NULL);
	mount("tmpfs",   "/tmp",  "tmpfs",   0, NULL);

	kmsg_fd = open("/dev/kmsg", O_WRONLY);

	klog("================================");
	klog("Mi MIX 4 (odin) 5.15 bring-up");
	klog("================================");
	dump("/proc/version", "no /proc/version");

	setup_usb_gadget();

	klog("--- partitions ---");
	dump("/proc/partitions", "no /proc/partitions");
	klog("--- end ---");
	klog("init reached userspace. If you can read this, the kernel works.");

	for (;;) {
		sleep(60);
		klog("alive");
	}
	return 0;
}
