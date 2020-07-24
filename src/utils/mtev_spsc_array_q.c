/*
 * Copyright (c) 2005-2009, OmniTI Computer Consulting, Inc.
 * All rights reserved.
 * Copyright (c) 2015-2020, Circonus, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name OmniTI Computer Consulting, Inc. nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ck_md.h>
#include <stdatomic.h>

#include "mtev_defines.h"
#include "mtev_spsc_array_q.h"

typedef struct mtev_spsc_array_q_t {
    int8_t padding[CK_MD_CACHELINE];
    struct {
        uint64_t tail;
        uint64_t head_cache;
        int8_t padding[CK_MD_CACHELINE - (2 * sizeof(uint64_t))];
    } producer;

    struct {
        uint64_t head;
        int8_t padding[CK_MD_CACHELINE - (1 * sizeof(uint64_t))];
    } consumer;

    uint64_t capacity;
    uint64_t mask;
    volatile void **buffer;
} mtev_spsc_array_q_t;

mtev_spsc_array_q_t *mtev_spsc_array_q_init(uint64_t length) {
  mtev_spsc_array_q_t *q = malloc(sizeof(mtev_spsc_array_q_t));

  // TODO check length is a power of two

  // Make sure the buffer is never located right next to the fields
  q->buffer = malloc(sizeof(int*) * length);
  memset(&q->buffer, 0, sizeof(int*) * length);

  q->producer.head_cache = 0;
  q->producer.tail = 0;
  q->consumer.head = 0;
  q->capacity = length;
  q->mask = length - 1;
  return q;
}

void mtev_spsc_array_q_destroy(mtev_spsc_array_q_t *q) {
  free(q->buffer);
  free(q);
}

offer_result_t mtev_spsc_array_q_offer(volatile mtev_spsc_array_q_t *q, void *element) {
  if (NULL == element) return failure;
  uint64_t current_head = q->producer.head_cache;
  uint64_t buffer_limit = current_head + q->capacity;
  uint64_t current_tail = q->producer.tail;

  if (current_tail >= buffer_limit) {
    current_head = atomic_load(&q->consumer.head);
    buffer_limit = current_head + q->capacity;
    if (current_tail >= buffer_limit) return full;
    q->producer.head_cache = current_head;
  }

  const uint64_t index = current_tail & q->mask;

  atomic_store_explicit(&q->buffer[index], element, memory_order_relaxed);
  atomic_store_explicit(&q->producer.tail, current_tail + 1, memory_order_relaxed);
  return success;
}

volatile void* mtev_spsc_array_q_poll(volatile mtev_spsc_array_q_t *q) {
  const uint64_t current_head = q->consumer.head;
  const uint64_t index = current_head & q->mask;
  volatile void *item = atomic_load(&q->buffer[index]);

  if (item) {
    atomic_store_explicit(&q->buffer[index], NULL, memory_order_relaxed);
    atomic_store_explicit(&q->consumer.head, current_head + 1, memory_order_relaxed);
  }

  return item;
}

uint64_t mtev_spsc_array_q_size(volatile mtev_spsc_array_q_t *q) {
  uint64_t current_head_after = atomic_load(&q->consumer.head);
  uint64_t current_head_before;
  uint64_t current_tail;
  uint64_t size;
  
  do {
      current_head_before = current_head_after;
      current_tail = atomic_load(&q->producer.tail);
      current_head_after = atomic_load(&q->consumer.head);
  } while (current_head_after != current_head_before);

  size = current_tail - current_head_after;

  if ((int64_t)size < 0) return 0;
  if (size > q->capacity) return q->capacity;
  return size;
}
