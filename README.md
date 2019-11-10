An attempt to understand rust's Pin/Unpin behavior by writing a bad (and
probably incorrect ringbuffer)

There's two things of note here:
1) If we have internal pointers, we cannot fix them up with a move-ctor, since
   rust doesn't tell a struct when it has been moved.
2) If I want to make a ring buffer that stores the data members next to some
   book-keeping info, in rust, that's apparently hard (in C/C++ this is easy,
   either use dynamic memory with some trailing stuff on the struct, or use a
   template).
