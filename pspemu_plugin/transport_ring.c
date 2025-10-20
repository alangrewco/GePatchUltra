#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <psputilsforkernel.h>
#include <string.h>

#include "transport_ring.h"

static SceUID fd_ctrl = -1, fd_data = -1;
static gep_ctrl_t ctrl;
static u32 seq = 1;

static int mkdir_if_needed(void) {
  sceIoMkdir("ms0:/seplugins", 0777);
  sceIoMkdir("ms0:/seplugins/gepipe", 0777);
  return 0;
}
static int prealloc_file(SceUID fd, u32 size) {
  char zero[4096]; memset(zero, 0, sizeof(zero));
  sceIoLseek32(fd, size - sizeof(zero), PSP_SEEK_SET);
  sceIoWrite(fd, zero, sizeof(zero));
  sceIoSync("ms0:", 0);
  sceKernelDcacheWritebackInvalidateAll();
  return 0;
}

int gep_ring_init(void) {
  mkdir_if_needed();

  fd_ctrl = sceIoOpen(GEP_CTRL_PATH, PSP_O_RDWR | PSP_O_CREAT, 0666);
  if (fd_ctrl < 0) return -1;
  fd_data = sceIoOpen(GEP_DATA_PATH, PSP_O_RDWR | PSP_O_CREAT, 0666);
  if (fd_data < 0) return -1;

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.magic = GEP_MAGIC;
  ctrl.version = GEP_VER;
  ctrl.size = GEP_RING_SIZE;

  /* Write initial ctrl */
  sceIoLseek(fd_ctrl, 0, PSP_SEEK_SET);
  sceIoWrite(fd_ctrl, &ctrl, sizeof(ctrl));

  /* Preallocate ring file to exact size for sane wrap-around */
  prealloc_file(fd_data, GEP_RING_SIZE);
  return 0;
}

void gep_ring_shutdown(void) {
  if (fd_ctrl >= 0) sceIoClose(fd_ctrl);
  if (fd_data >= 0) sceIoClose(fd_data);
  fd_ctrl = fd_data = -1;
}

/* called periodically to ingest Vita ACK */
void gep_ack_poll(void) {
  gep_ctrl_t tmp;
  sceIoLseek(fd_ctrl, 0, PSP_SEEK_SET);
  sceIoRead(fd_ctrl, &tmp, sizeof(tmp));
  ctrl.ack_seq = tmp.ack_seq;
}

u32 gep_next_seq(void) {
  return seq++;
}

static int ring_free_bytes(void) {
  /* snapshot ctrl.r from disk */
  gep_ctrl_t tmp;
  sceIoLseek(fd_ctrl, 0, PSP_SEEK_SET);
  sceIoRead(fd_ctrl, &tmp, sizeof(tmp));
  ctrl.r = tmp.r;

  u32 w = ctrl.w, r = ctrl.r, size = ctrl.size;
  return (r + size - w - 1) & (size - 1);
}

static void ring_commit_write(const void *buf, u32 len) {
  u32 w = ctrl.w, size = ctrl.size;
  u32 end = (w + len) & (size - 1);

  if (w + len <= size) {
    sceIoLseek(fd_data, w, PSP_SEEK_SET);
    sceIoWrite(fd_data, buf, len);
  } else {
    u32 first = size - w;
    sceIoLseek(fd_data, w, PSP_SEEK_SET);
    sceIoWrite(fd_data, buf, first);
    sceIoLseek(fd_data, 0, PSP_SEEK_SET);
    sceIoWrite(fd_data, (const char*)buf + first, len - first);
  }

  ctrl.w = end;
  /* publish w to ctrl file */
  sceIoLseek(fd_ctrl, offsetof(gep_ctrl_t, w), PSP_SEEK_SET);
  sceIoWrite(fd_ctrl, &ctrl.w, sizeof(ctrl.w));
}

int gep_send(const void *payload, u16 size, u8 type) {
  gep_ack_poll();

  /* Window control: reject if too many in-flight seq not acked */
  if (seq - ctrl.ack_seq > GEP_WINDOW_PKTS) {
    ctrl.flags |= 1;
    ctrl.dropped++;
    sceIoLseek(fd_ctrl, offsetof(gep_ctrl_t, dropped), PSP_SEEK_SET);
    sceIoWrite(fd_ctrl, &ctrl.dropped, sizeof(ctrl.dropped));
    return -1;
  }

  if (size > GEP_MAX_PAYLOAD) return -1;

  gep_hdr_t hdr;
  hdr.type = type;
  hdr._pad = 0;
  hdr.size = size;
  hdr.seq  = gep_next_seq();

  const u32 total = sizeof(hdr) + size;
  const int freeb = ring_free_bytes();
  if (freeb < (int)total) {
    ctrl.flags |= 1;
    ctrl.dropped++;
    sceIoLseek(fd_ctrl, offsetof(gep_ctrl_t, dropped), PSP_SEEK_SET);
    sceIoWrite(fd_ctrl, &ctrl.dropped, sizeof(ctrl.dropped));
    return -1;
  }

  char buf[sizeof(hdr) + GEP_MAX_PAYLOAD];
  memcpy(buf, &hdr, sizeof(hdr));
  if (size) memcpy(buf + sizeof(hdr), payload, size);

  ring_commit_write(buf, total);
  sceIoSync("ms0:", 0);
  return 0;
}