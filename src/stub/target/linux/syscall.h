// long __syscall0(long n);
// long __syscall1(long n, long a1);
// long __syscall2(long n, long a1, long a2);
// long __syscall3(long n, long a1, long a2, long a3);
// long __syscall4(long n, long a1, long a2, long a3, long a4);
// long __syscall5(long n, long a1, long a2, long a3, long a4, long a5);
// long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6);

extern long syscall(long n, ...);
