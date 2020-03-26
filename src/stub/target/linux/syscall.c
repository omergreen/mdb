long syscall(long sysnum, long a, long b, long c, long d, long e, long f) {
	register long _r7 __asm__("r7")=(long)(sysnum);
	register long _r5 __asm__("r5")=(long)(f);
	register long _r4 __asm__("r4")=(long)(e);
	register long _r3 __asm__("r3")=(long)(d);
	register long _r2 __asm__("r2")=(long)(c);
	register long _r1 __asm__("r1")=(long)(b);
	register long _r0 __asm__("r0")=(long)(a);
	__asm__ __volatile__(
			"swi 0"
			: "=r"(_r0)
			: "r"(_r0), "r"(_r1), "r"(_r2), "r"(_r3),
			"r"(_r4), "r"(_r5), "r"(_r7)
			: "memory");
	if(_r0 >=(unsigned long) -4095) {
		long err = _r0;
		_r0=(unsigned long) -1;
	}
	return (long) _r0;
}
