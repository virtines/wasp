#pragma once
/*
 * This file is part of the Wasp hypervisor developed at Illinois Institute of
 * Technology (HExSA Lab) and Northwestern University with funding from the
 * United States National Science Foundation.
 *
 * Copyright (c) 2022, Nicholas Wanninger
 *
 * Author:  Nicholas Wanninger <ncw@u.northwestern.edu>
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "LICENSE.txt".
 */

#include <stdlib.h>
#include <deque>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <wasp/Virtine.h>

namespace wasp {


  class Cache final {
   public:
    Cache(size_t memsz) : m_memsz(memsz) {}
    ~Cache(void);

    // allocate a new virtine. Typically not used by the end user
    wasp::Virtine *allocate();
    // get a clean virtine from the pool. Allocating if needed
    wasp::Virtine *get();
    void put(wasp::Virtine *);

    void ensure(int count);


		/* TODO: multithreading. Take the lock */
    size_t size(void) const { return m_cache.size(); }
    // the `data` arg must outlive the Cache
    void set_binary(const void *data, size_t size, off_t start);
    void load_binary(const char *bianry, off_t start);

		auto hits(void) const {
			return m_hits;
		}
		
		auto misses(void) const {
			return m_misses;
		}

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

    size_t m_hits = 0;
    size_t m_misses = 0;
  };
}  // namespace wasp
