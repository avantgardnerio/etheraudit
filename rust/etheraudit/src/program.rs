use std::collections::btree_map::BTreeMap;
use std;

use instruction::*;
use block::*;
use op_codes;

pub type ByteCode = std::vec::Vec<u8>;

pub struct Program {
    pub byte_code: ByteCode,
    pub instructions: BTreeMap<usize, Instruction>,
    pub blocks: BTreeMap<usize, Block>
}

impl Program {
    pub fn fill_blocks(self: &mut Program) {
        let mut start = 0;
        let mut this_end = Some(0);

        for (pos, instr) in &self.instructions {
            if instr.op_code == op_codes::JUMPDEST {
                if let Some(end) = this_end {
                    self.blocks.insert(start, Block::new(start, end));
                }
                start = *pos;
            }

            this_end = Some(pos + instr.data.len() + 1);
            if op_codes::is_jump(instr.op_code) || op_codes::is_stop(instr.op_code) {
                if let Some(end) = this_end {
                    self.blocks.insert(start, Block::new(start, end));
                    start = end;
                };

                this_end = None;
            }
        }
    }

    pub fn fill_instructions(self: &mut Program) {
        let mut offset = 0;
        while offset < self.byte_code.len() {
            let bc = self.byte_code[offset];
            //let opCode = evm::INSTRUCTIONS[bc as usize];
            self.instructions.insert(offset, Instruction::new(offset,bc));
            let instr = &mut self.instructions.get_mut(&offset).expect("but I just added it!");

            offset += 1;
            for _ in 0..op_codes::push_bytes(bc) {
                if offset < self.byte_code.len() {
                    instr.data.push(self.byte_code[offset]);
                }
                offset += 1;
            }
        }
    }

    pub fn new(byte_code: ByteCode) -> Self {
        let mut rtn = Program {
            byte_code,
            instructions: BTreeMap::new(),
            blocks: BTreeMap::new()
        };

        rtn.fill_instructions();
        rtn.fill_blocks();
        rtn
    }
}
