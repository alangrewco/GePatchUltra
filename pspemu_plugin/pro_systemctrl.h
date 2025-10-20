#ifndef PRO_SYSTEMCTRL_H
#define PRO_SYSTEMCTRL_H
#include <psptypes.h>

// Exported by ARK-4 SystemCtrlForKernel static lib:
unsigned int sctrlHENFindFunction(const char* modname, const char* libname, unsigned int nid);

// Keep this here for later syscall patching (also provided by ARK lib):
int sctrlHENPatchSyscall(unsigned int oldaddr, void* newaddr);

#endif
