#include <pspsdk.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspge.h>
#include <string.h>
#include "ge_constants.h"
#include "transport_ring.h"
#include "../common/protocol.h"

typedef struct {
  u32 base, offset;
} GETraceCtx;

static void emit_ge(u8 cmd, u32 data, u32 base, u32 offset, u32 list_id) {
  pkt_ge_cmd_t p = {0};
  p.cmd = cmd; p.data = data; p.base = base; p.offset = offset; p.list_id = list_id;
  gep_send(&p, sizeof(p), PKT_GE_CMD);
}

void traceGeList(u32 *list, u32 *stall) {
  GETraceCtx ctx = {0};
  u32 *pc = list;
  const u32 list_id = ((u32)list) & 0x0fffffff;
  pkt_frame_t fb = { .frame_id = list_id };
  gep_send(&fb, sizeof(fb), PKT_FRAME_BEGIN);

  for (; pc && (!stall || pc != stall); pc++) {
    u32 op = *pc;
    u8  cmd = op >> 24;
    u32 data = op & 0xffffff;
    emit_ge(cmd, data, ctx.base, ctx.offset, list_id);

    switch (cmd) {
      case GE_CMD_BASE:        ctx.base = (data << 8) & 0x0f000000; break;
      case GE_CMD_OFFSETADDR:  ctx.offset = data << 8; break;
      case GE_CMD_ORIGIN:      ctx.offset = (u32)pc; break;

      case GE_CMD_CALL: {
        u32 addr = ((ctx.base | data) + ctx.offset) & 0x0ffffffc;
        u32 *target = (u32 *)(addr - 4);
        traceGeList(target, NULL);
        break;
      }
      case GE_CMD_JUMP:
      case GE_CMD_BJUMP: {
        u32 addr = ((ctx.base | data) + ctx.offset) & 0x0ffffffc;
        pc = (u32 *)(addr - 4);
        break;
      }
      case GE_CMD_RET:
        /* handled by caller stacks; we do flat walk here */
        break;

      case GE_CMD_END: {
        /* If previous was FINISH, we close the frame */
        u32 prev = *(pc - 1);
        if ((prev >> 24) == GE_CMD_FINISH) {
          pkt_frame_t fe = { .frame_id = list_id };
          gep_send(&fe, sizeof(fe), PKT_FRAME_END);
          return;
        }
        break;
      }
    }
  }
}
