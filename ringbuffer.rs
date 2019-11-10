#[derive(Debug)]
enum RBErr {
    Full,
    Empty,
}

// can't do the struct allocation with trailing data thing here, I think?
struct RingBuffer<T: Sized + Default> {
    entries: [T; 3],           // FIXME use const_generics for this, or heap allocate (ironically heap allocting solves the entire problem), size of 2 right now
    head:    Option<*mut T>,   // technically never need to modify the data but more types is a pain
    tail:    Option<*mut T>,   // will write here
}

impl<T: Sized + Default> RingBuffer<T> { // apparently these can't be const generic yet?
    fn new() -> Self {
        Self {
            entries: Default::default(),

            // can't set these here, the struct is about to move and
            // invalidate the pointers
            head:    None,
            tail:    None,
        }
    }

    // this is a hack! after putting in final resting place, init the
    // thing
    fn initptr(&mut self) {
        self.head = Some(self.entries.as_mut_ptr());
        self.tail = Some( unsafe {
            self.entries.as_mut_ptr().add(1)
        });
    }

    unsafe fn inc(&mut self, ptr: *mut T) -> *mut T {
        if ptr == self.entries.as_mut_ptr().add(3 /* FIXME hardcoded constant */) {
            return self.entries.as_mut_ptr();
        }
        else {
            return ptr.add(1);
        }
    }

    fn full(&mut self) -> bool {
        if self.head == self.tail { return true; }
        else                      { return false; }
    }

    fn empty(&mut self) -> bool {
        if unsafe { self.inc(self.head.unwrap()) } == self.tail.unwrap() { return true; }
        else                                                             { return false; }
    }

    fn push(&mut self, v: T) -> Result<(), RBErr> {
        if self.full() { return Err(RBErr::Full); }
        unsafe { *(self.tail.unwrap()) = v }; // move

        self.tail = unsafe { Some(self.inc(self.tail.unwrap())) };
        Ok( () )
    }

    fn pop(&mut self) -> Result<T, RBErr> {
        if self.empty() { return Err(RBErr::Empty); }
        let ret = unsafe { std::ptr::read(self.head.unwrap().add(1)) };
        self.head = unsafe { Some(self.inc(self.head.unwrap())) };
        return Ok( ret );
    }
}

fn make() -> RingBuffer<u32> {
    let mut r = RingBuffer::<u32>::new();
    r.initptr(); // assume it won't move after this
    r.push(10).unwrap();
    r.push(11).unwrap();
    return r; // uh oh
}

fn main() {
    let mut r = RingBuffer::<u32>::new();
    r.initptr(); // assume it won't move after this
    r.push(10).unwrap();
    r.push(11).unwrap();
    println!("{}", r.pop().unwrap());

    // now, here's the problem.
    // if we move this struct, we're screwed. without recalling
    // initptr, there's nothing we can do to fix our pointers.
    // since references are more-or-less the same thing as pointers,
    // we have the same problem with references.

    let mut r2 = r; // MOVE!
    println!("{}", r2.pop().unwrap()); // this one actually works because we're taking the same stack slot

    // this will not work, since r was created on stack frame of make
    let mut r = make();
    println!("{}", r.pop().unwrap()); // prints the right thing, but valgrind has a tantrum

    // mem::replace and mem::swap both have potential to copy internal
    // pointers around, screwing up the data stuctures. If we mark
    // RingBuffer as !Unpin, I think that solves this problem.
}
