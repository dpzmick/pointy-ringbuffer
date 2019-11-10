#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char*  head;
  char*  tail;
  size_t n_elt;
  size_t elt_sz;                // could make this "compile time" by passing it to every function
  char   buffer[];              // trailing array of entries... aligned to end of struct
} ring_buffer_t;

static void*
_rb_inc( ring_buffer_t* rb,
         char*          ptr )
{
  if (ptr == rb->buffer + rb->elt_sz*(rb->n_elt-1)) {
    return rb->buffer;
  }
  else {
    return ptr + rb->elt_sz;
  }
}

static bool
_rb_full( ring_buffer_t* rb )
{
  if (rb->tail == rb->head) return true;
  else                      return false;
}

static bool
_rb_empty( ring_buffer_t* rb )
{
  if (_rb_inc(rb, rb->head) == rb->tail) return true;
  else                                   return false;
}

size_t
ring_buffer_size( size_t n_elements,
                  size_t element_size )
{
  return sizeof(ring_buffer_t) + (n_elements+1)*element_size;
}

ring_buffer_t*
new_ring_buffer( void*  mem,
                 size_t n_elt,
                 size_t element_size )
{
  ring_buffer_t* ret = mem; // better be the right size GOOD LUCK
  ret->head   = ret->buffer;
  ret->tail   = ret->buffer + element_size;
  ret->n_elt  = n_elt+1;
  ret->elt_sz = element_size;
  return ret;
}

bool
ring_buffer_push( ring_buffer_t* rb,
                  void*          element )
{
  if (_rb_full(rb)) return false;
  memcpy(rb->tail, element, rb->elt_sz);
  rb->tail = _rb_inc(rb, rb->tail);
  return true;
}

bool
ring_buffer_pop( ring_buffer_t* rb,
                 void*          into /* output copied here */ )
{
  if (_rb_empty(rb)) return false;
  memcpy(into, _rb_inc(rb, rb->head), rb->elt_sz);
  rb->head = _rb_inc(rb, rb->head);
  return true;
}

// Move all of the elements from one ring buffer to another,
// invalidates `a`, populates `b`
// both must be the right size GOOD LUCK (could validate this at
// runtime)

void
ring_buffer_move( ring_buffer_t* a,
                  ring_buffer_t* b )
{
  memcpy(b->buffer, a->buffer, a->n_elt * a->elt_sz);
  b->head = b->buffer + ((char*)a->head - a->buffer);
  b->tail = b->buffer + ((char*)a->tail - a->buffer);

  // theoretically the other two elements are already the same
}

int main()
{
  size_t sz = ring_buffer_size(2, sizeof(uint32_t));
  void* mem = calloc(sz, 1);
  if (!mem) abort();

  ring_buffer_t* rb = new_ring_buffer(mem, 2, sizeof(uint32_t));
  if (!rb) abort();

  bool ret = ring_buffer_push(rb, &(uint32_t){10});
  if (!ret) abort();

  ret = ring_buffer_push(rb, &(uint32_t){11});
  if (!ret) abort();

  uint32_t out[1];
  ring_buffer_pop(rb, out);
  printf("pop: %d\n", *out);

  ring_buffer_pop(rb, out);
  printf("pop: %d\n", *out);

  free(mem);
}
