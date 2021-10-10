#pragma once

#include <stdint.h>
#include <chrono>


using namespace std::chrono;

namespace wasp {


  inline uint64_t time_us(void) {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  }

}  // namespace wasp
