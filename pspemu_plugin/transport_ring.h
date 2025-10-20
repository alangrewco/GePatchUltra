#pragma once
#include <psptypes.h>
#include "../common/protocol.h"

int  gep_ring_init(void);
void gep_ring_shutdown(void);

/* returns 0 on success, -1 on drop (no space / window full) */
int  gep_send(const void *payload, u16 size, u8 type);

/* helpers */
u32  gep_next_seq(void);
void gep_ack_poll(void); /* read vita ACK from ctrl.bin */