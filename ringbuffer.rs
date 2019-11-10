struct RingBuffer<T: Sized + Default> {
    entries: [T; 4],           // FIXME use const_generics for this, or heap allocate (ironically heap allocting solves the entire problem)
    head:    Option<*const T>, // technically never need to modify the data
    tail:    Option<*mut T>,   // will write here
}

impl<T: Sized + Default> RingBuffer<T, 3> { // apparently these can't be const generic yet?
    fn new() -> Self {
        Self {
            entries: Default::default(),

            // can't set these here, the struct is about to move and
            // invalidate the pointers
            head:    None,
            tail:    None,
        }
    }
}

fn main() {
    let r = RingBuffer::<u32, 2>::new();
}
