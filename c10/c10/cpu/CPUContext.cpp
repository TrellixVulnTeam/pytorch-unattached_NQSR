#include "CPUContext.h"

namespace c10 { namespace cpu {

CPUContext & globalCPUContext() {
  static CPUContext ctx;
  return ctx;
}

}}
