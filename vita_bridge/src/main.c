#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>   // for sceIoMkdir
#include <string.h>

static void log_host(const char *msg) {
  sceIoMkdir("ux0:data/renderer_bridge", 0777);
  int fd = sceIoOpen("ux0:data/renderer_bridge/log.txt",
                     SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
  if (fd >= 0) {
    sceIoWrite(fd, msg, strlen(msg));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
  }
}

int main(void) {
  log_host("Vita bridge app started");
  sceKernelDelayThread(2 * 1000 * 1000); // 2 seconds
  sceKernelExitProcess(0);
  return 0;
}
