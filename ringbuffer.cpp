#include <array>
#include <iostream>
#include <stdexcept>

template <typename T, size_t N>
struct ring_buffer {
  std::array<T, N+1> entries;

  T* head = entries.data(); // pop from here
  T* tail = head+1;         // push to here, one past the end

  ring_buffer() = default;

#ifdef CTORS
  ring_buffer(ring_buffer const& other)
    : entries(other.entries)
    , head(entries.data() + (other.head - other.entries.data()))
    , tail(entries.data() + (other.tail - other.entries.data()))
  { }

  ring_buffer(ring_buffer&& other)
    : entries(std::move(other.entries)) // is this valid?
    , head(entries.data() + (other.head - other.entries.data()))
    , tail(entries.data() + (other.tail - other.entries.data()))
  {
    // some sort of valid state, just empty again
    other.head = other.entries.data();
    other.tail = other.head+1;
  }
#endif

  T* inc(T* ptr) { // need both, cause const...
    if (ptr == entries.data()+N) return entries.data();
    else                         return ptr+1;
  }

  T const* inc(T* ptr) const {
    if (ptr == entries.data()+N) return entries.data();
    else                         return ptr+1;
  }

  bool full() const {
    if (tail == head) return true;
    else              return false;
  }

  bool empty() const {
    if (inc(head) == tail) return true;
    else                   return false;
  }

  void push(T value) {
    if (full()) throw std::runtime_error("full");
    *tail = std::move(value);
    tail = inc(tail);
  }

  T pop() {
    if (empty()) throw std::runtime_error("empty");
    T ret = std::move(*inc(head));
    head = inc(head);
    return ret;
  }

  // throw if the head and tail pointers are junk
  void validate() const {
    if (entries.data() <= head && entries.data() + N+1 >= head) return;
    if (entries.data() <= tail && entries.data() + N+1 >= tail) return;
    throw std::runtime_error("bad");
  }

  void print() const {
    printf("head %zu tail %zu\n", head - entries.data(), tail - entries.data());
    printf("entries %p head %p tail %p\n", entries.data(), head, tail);
    for (size_t i = 0; i < N+1; ++i) {
      printf("  entries[%zu] = %d\n", i, (uint32_t)entries.at(i));
    }
  }
};

struct NonCopy {
  NonCopy() : id(0) { } // need some default stub

  NonCopy(uint32_t id)
    : id(id)
  { }

  NonCopy(NonCopy&& other)
    : id(other.id)
  {
    other.id = 0;
  }

  NonCopy& operator=(NonCopy&& other)
  {
    id = other.id;
    other.id = 0;
    return *this;
  }

  NonCopy(NonCopy const&)            = delete;
  NonCopy& operator=(NonCopy const&) = delete;

  operator uint32_t() const { return id; }

  uint32_t id;
};

int main() {
  ring_buffer<NonCopy, 2> rb;
  rb.push(10);
  rb.push(11);
  rb.print();
  rb.validate(); // all good!
  std::cout << std::endl;

  ring_buffer<NonCopy, 2> rb2 = std::move(rb); // this moves the location of our array
  rb2.print();                                 // without move ctor, head and tail still point to the original location
  rb2.validate();                              // will fail without the move ctor
  std::cout << rb2.pop() << std::endl; // prints '0' without a corrected move ctor
  std::cout << rb2.pop() << std::endl; // prints '0' without a corrected move ctor
  std::cout << std::endl;

  rb.print(); // for curiosity
}

// so, if you managed to write a structure like this in rust, it would
// need to be unpin, because moving the ring buffer would invalidate
// the internal pointers.
// rust has no concept of move-ctor (so you can't fix them after a
// move, they would just be garbage), and it has no concept of the empty
// shell left behind (rb).
