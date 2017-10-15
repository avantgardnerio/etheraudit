use std;

pub struct Instruction {
    pub offset: usize,
    pub op_code: u8,
    pub data: std::vec::Vec<u8>
}

impl Instruction {
    pub fn new(offset: usize, op_code: u8) -> Self {
        Instruction {offset, op_code, data: std::vec::Vec::new()}
    }
}
