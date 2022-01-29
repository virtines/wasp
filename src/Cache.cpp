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
#include <wasp/Cache.h>

#include <string.h>

wasp::Cache::~Cache(void) {
  while (m_cache.size() > 0) {
    auto *v = m_cache.front();
    m_cache.pop_front();
    delete v;
  }
}


void wasp::Cache::lock(void) { m_lock.lock(); }
void wasp::Cache::unlock(void) { m_lock.unlock(); }

wasp::Virtine *wasp::Cache::allocate() {
  auto *v = new wasp::Virtine();

  if (m_reset == NULL) {
    // printf("m_reset is null!\n");

    m_reset = new wasp::ResetState;
    // copy the initial reset state
    v->read_regs(m_reset->regs);
    v->read_sregs(m_reset->sregs);
    v->read_fpu(m_reset->fpu);
    // setup the initial region
    std::vector<wasp::ResetMemory> regions;
    wasp::ResetMemory m;
    m.size = m_binary_size;
    m.data = (void *)m_binary;
    m.address = m_binary_start;
    m_reset->regions.clear();
    m_reset->regions.push_back(std::move(m));
  }

  v->set_reset_mask(m_reset);
  v->allocate_memory(m_memsz);
  v->reset();

  return v;
}


void wasp::Cache::set_binary(const void *data, size_t size, off_t start) {
  m_binary = data;
  m_binary_size = size;
  m_binary_start = start;
}


wasp::Virtine *wasp::Cache::get() {
  wasp::Virtine *v = nullptr;
  lock();
  // if there are no virtines ready, create one
  if (m_cache.size() == 0) {
    v = allocate();
  } else {
    v = m_cache.front();
    m_cache.pop_front();
  }

  unlock();
  return v;
}

void wasp::Cache::ensure(int count) {
  lock();
  while (m_cache.size() < count) {
    m_cache.push_front(allocate());
  }
  unlock();
}


void wasp::Cache::put(wasp::Virtine *v) {
  // TODO: do we need to move the virtine cleaning to another thread?
  v->reset();
  lock();
  m_cache.push_back(v);
  unlock();
}
