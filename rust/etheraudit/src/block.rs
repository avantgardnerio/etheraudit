pub struct Block {
    pub start: usize,
    pub end: usize
}

impl Block {
    pub fn new(start: usize, end: usize) -> Self {
        Block {
            start,
            end
        }
    }
}
