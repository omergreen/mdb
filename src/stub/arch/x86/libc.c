void cache_flush() {
    volatile asm("wbinvd");
}

