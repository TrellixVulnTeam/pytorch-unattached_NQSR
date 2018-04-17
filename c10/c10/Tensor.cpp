#include "Tensor.h"

// TODO: Strictly temporary: these polymorphic functions should still go through the dispatcher
#include "c10/op/All.h"

// TODO: Strictly temporary, hardcoded CPU
#include "c10/cpu/op/CPUAll.h"

namespace c10 {

void Tensor::resize_(ArrayRef<int64_t> size, ArrayRef<int64_t> stride, bool keep_data) {
  // TODO: Use the dynamic dispatcher instead
  cpu::op::resize_(*this, size, stride, keep_data);
}

void Tensor::copy_(DataType dtype, const void* p, int64_t size_bytes) {
  // TODO: Use the dynamic dispatcher instead
  cpu::op::copy_(*this, dtype, p, size_bytes);
}

void Tensor::extend_(int64_t num, double growthPct) {
  // TODO: Use the dynamic dispatcher instead
  cpu::op::extend_(*this, num, growthPct);
}

void Tensor::reserve_(ArrayRef<int64_t> new_size) {
  // TODO: Use the dynamic dispatcher instead
  cpu::op::reserve_(*this, new_size);
}

}
