#pragma once

#include <stdlib.h>
#include <deque>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <wasp/Virtine.h>

namespace wasp {


  class Cache final {
   public:
    Cache(size_t memsz) : m_memsz(memsz) {
    }
    ~Cache(void);

    // allocate a new virtine. Typically not used by the end user
    wasp::Virtine *allocate();
    // get a clean virtine from the pool. Allocating if needed
    wasp::Virtine *get();
    void put(wasp::Virtine *);

		void ensure(int count);


		// the `data` arg must outlive the Cache
		void set_binary(const void *data, size_t size, off_t start);
   private:

		void lock(void);
		void unlock(void);
    // the size of a virtine's memory pool in this cache
    size_t m_memsz;
    std::mutex m_lock;
    std::deque<wasp::Virtine *> m_cache;

		wasp::ResetState *m_reset = NULL;

		const void *m_binary = NULL;
		size_t m_binary_size;
		off_t m_binary_start;
		
  };
}  // namespace wasp
