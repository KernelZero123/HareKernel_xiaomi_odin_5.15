/* odin bring-up init: minimal, static, no libc deps beyond syscalls */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/wait.h>

static void klog(const char *s)
{
	int fd = open("/dev/kmsg", O_WRONLY);
	if (fd >= 0) { write(fd, s, strlen(s)); close(fd); }
	printf("%s\n", s);
	fflush(stdout);
}

int main(void)
{
	mkdir("/proc", 0555);
	mkdir("/sys", 0555);
	mkdir("/dev", 0755);
	mkdir("/mnt", 0755);

	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
	mount("devpts", "/dev/pts", "devpts", 0, NULL);
	mount("tmpfs", "/tmp", "tmpfs", 0, NULL);

	klog("ODIN: ================================");
	klog("ODIN: userspace reached, init alive");
	klog("ODIN: ================================");

	/* dump kernel version + cmdline so pstore proves how far we got */
	{
		char buf[1024]; int fd, n;
		fd = open("/proc/version", O_RDONLY);
		if (fd >= 0) { n = read(fd, buf, sizeof(buf)-1); if (n > 0) { buf[n] = 0; klog(buf); } close(fd); }
		fd = open("/proc/cmdline", O_RDONLY);
		if (fd >= 0) { n = read(fd, buf, sizeof(buf)-1); if (n > 0) { buf[n] = 0; klog(buf); } close(fd); }
	}

	/* report block devices so we can see if UFS probed */
	{
		char buf[512]; int fd, n;
		fd = open("/proc/partitions", O_RDONLY);
		if (fd >= 0) { n = read(fd, buf, sizeof(buf)-1); if (n > 0) { buf[n] = 0; klog("ODIN: --- partitions ---"); klog(buf); } close(fd); }
	}

	klog("ODIN: init done, sleeping forever (pstore keeps this log)");

	for (;;) {
		sleep(3600);
		/* keep console alive so dmesg ring stays in ramoops */
		klog("ODIN: alive");
	}
	return 0;
}
