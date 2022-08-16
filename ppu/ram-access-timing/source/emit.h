
#ifndef _EMIT_H_
#define _EMIT_H_

typedef void (*emit_fn)();

void emit_init();
emit_fn emit_get_test(int delay, u32 address);
emit_fn emit_get_wait();

#endif // _EMIT_H_