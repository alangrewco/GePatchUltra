#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mock_kermit.h"
#include "../common/protocol.h"
#include "../common/framing.h"

int main(void) {
  int fd = mk_connect("127.0.0.1", 9000);
  if (fd < 0) { perror("connect"); return 1; }

  GeTinyPkt begin = { .type = PKT_FRAME_BEGIN };
  if (send_frame(fd, &begin, sizeof(begin)) < 0) { perror("send begin"); return 1; }

  GeCmdPkt c = {0};
  c.type = PKT_GE_CMD; c.list_id = 1;

  // Fake a tiny command stream: VIEWPORTXSCALE(0x42), VIEWPORTYSCALE(0x43), FINISH(0x0F)
  c.cmd = 0x42; c.data = 0x001000; c.base = 0; c.offset = 0x0; c.pc = 0x1000;
  if (send_frame(fd, &c, sizeof(c)) < 0) { perror("send cmd1"); return 1; }
  c.cmd = 0x43; c.data = 0x001000; c.pc  = 0x1004;
  if (send_frame(fd, &c, sizeof(c)) < 0) { perror("send cmd2"); return 1; }
  c.cmd = 0x0F; c.data = 0x000000; c.pc  = 0x1008;
  if (send_frame(fd, &c, sizeof(c)) < 0) { perror("send cmd3"); return 1; }

  GeTinyPkt end = { .type = PKT_FRAME_END };
  if (send_frame(fd, &end, sizeof(end)) < 0) { perror("send end"); return 1; }

  // Wait for ACK
  uint8_t* buf = NULL; uint32_t len = 0;
  if (recv_frame_alloc(fd, &buf, &len) != 0) { fprintf(stderr, "recv ACK failed\n"); return 1; }
  if (len != sizeof(GeTinyPkt) || buf[0] != PKT_ACK) { fprintf(stderr, "bad ACK\n"); free(buf); return 1; }
  free(buf);

  puts("[sender] success: got ACK");
  return 0;
}
