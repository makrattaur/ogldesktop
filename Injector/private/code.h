
#define STATIC_SIZE(a) sizeof(a) / sizeof(a[0])

const unsigned char InjectedCodeX64[] = {
#include "hc-inject-x64.inc"
};
const int InjectedCodeX64Size = STATIC_SIZE(InjectedCodeX64);

const unsigned char InjectedCodeX64Debug[] = {
#include "hc-inject-x64-dbg.inc"
};
const int InjectedCodeX64DebugSize = STATIC_SIZE(InjectedCodeX64Debug);

const unsigned char InjectedCodeX86[] = {
#include "hc-inject-x86.inc"
};
const int InjectedCodeX86Size = STATIC_SIZE(InjectedCodeX86);

const unsigned char InjectedCodeX86Debug[] = {
#include "hc-inject-x86-dbg.inc"
};
const int InjectedCodeX86DebugSize = STATIC_SIZE(InjectedCodeX86Debug);

#undef STATIC_SIZE

