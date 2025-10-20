#include "pro_systemctrl.h"
#include <psptypes.h>

u32 FindProc(const char* modname, const char* libname, u32 nid) {
    return sctrlHENFindFunction(modname, libname, nid);
}
