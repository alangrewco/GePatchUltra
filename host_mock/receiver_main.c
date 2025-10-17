#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mock_kermit.h"
#include "../common/protocol.h"
#include "../common/framing.h"

static void print_cmd(const GeCmdPkt* c) {
  printf("  GE_CMD: cmd=0x%02X data=0x%06X base=0x%08X off=0x%08X list=%u pc=0x%08X\n",
         c->cmd, c->data & 0xFFFFFFu, c->base, c->offset, c->list_id, c->pc);
}

int main(void) {
  int srv = mk_listen(9000);
  if (srv < 0) { perror("listen"); return 1; }
  printf("[receiver] listening on 127.0.0.1:9000\n");
  int fd = mk_accept(srv);
  if (fd < 0) { perror("accept"); return 1; }
  puts("[receiver] client connected");

  int running = 1, cmds = 0;
  while (running) {
    uint8_t* buf = NULL; uint32_t len = 0;
    int rc = recv_frame_alloc(fd, &buf, &len);
    if (rc) { fprintf(stderr, "recv error %d\n", rc); break; }
    if (len == 0) { free(buf); continue; }

    uint8_t type = buf[0];
    if (type == PKT_FRAME_BEGIN) {
      puts("[receiver] FRAME_BEGIN");
    } else if (type == PKT_GE_CMD) {
      if (len < sizeof(GeCmdPkt)) { fprintf(stderr, "short GE_CMD\n"); free(buf); break; }
      print_cmd((const GeCmdPkt*)buf);
      cmds++;
    } else if (type == PKT_FRAME_END) {
      puts("[receiver] FRAME_END -> sending ACK");
      GeTinyPkt ack = { .type = PKT_ACK };
      send_frame(fd, &ack, sizeof(ack));
      running = 0; // stop after one frame for the demo
    } else {
      printf("[receiver] unknown pkt %u len=%u\n", type, len);
    }
    free(buf);
  }

  printf("[receiver] received %d GE_CMDs\n", cmds);
  return 0;
}
