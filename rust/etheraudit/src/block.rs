use program;

pub struct Block {
    pub start: usize,
    pub end: usize,
    pub exit_stack: program::EvmStack
}

impl Block {
    pub fn new(start: usize, end: usize, exit_stack: program::EvmStack) -> Self {
        Block {
            start,
            end,
            exit_stack
        }
    }
}
