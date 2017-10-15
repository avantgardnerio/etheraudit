use std::collections::btree_map::BTreeMap;
use std;

use expression;
use instruction::*;
use block::*;
use op_codes;
use std::rc::Rc;


pub type ByteCode = std::vec::Vec<u8>;
pub type EvmStack = std::vec::Vec< Rc<expression::Expression> >;

pub struct Program {
    pub byte_code: ByteCode,
    pub instructions: BTreeMap<usize, Instruction >,
    pub blocks: BTreeMap<usize, Block>
}

impl Program {
    fn fill_expressions(self: &mut Program) {
        for block in self.blocks.values() {
            let mut stack : std::vec::Vec< Rc<expression::Expression> > = std::vec::Vec::new();
            let mut arg = 0;
            for pos in block.start..block.end {
                if let Some(instr) = self.instructions.get_mut(&pos) {
                    let op_info: &op_codes::OpCodeInfo = &op_codes::OPCODES[instr.op_code as usize];
                    for i in 0..op_info.args {
                        let entry = match stack.pop() {
                            Some(e) => e,
                            None => {
                                arg += 1;
                                Rc::new( expression::Expression::Argument(arg-1))
                            }
                        };

                        instr.inputs.push(entry);
                    }

                    for i in 0..op_info.ret {

                    }

                    if !instr.data.is_empty() {
                        let expr = Rc::new(expression::make_constant(&instr.data));
                        instr.inputs.push(Rc::clone(&expr));
                        stack.push( expr);
                    }

                }
            }
        }
    }

    pub fn fill_instructions(self: &mut Program) {
        let mut stack = EvmStack::new();
        let mut start = 0;
        let mut this_end = Some(0);
        let mut arg_count = 0;

        let mut offset = 0;
        while offset < self.byte_code.len() {
            let instr = Instruction::from_stack(&mut offset,
                                                &self.byte_code, &mut stack,
                                                &mut arg_count);

            if instr.op_code == op_codes::JUMPDEST {
                if let Some(end) = this_end {
                    self.blocks.insert(start, Block::new(start, end, stack.to_vec()));
                }
                start = offset - 1;
                stack.clear();
                arg_count = 0;
            }

            this_end = Some(offset + instr.data.len());
            if op_codes::is_jump(instr.op_code) || op_codes::is_stop(instr.op_code) {
                if let Some(end) = this_end {
                    self.blocks.insert(start, Block::new(start, end, stack.to_vec()));
                    start = end;
                };

                this_end = None;
            }

            self.instructions.insert(instr.offset, instr);
        }
    }

    pub fn new(byte_code: ByteCode) -> Self {
        let mut rtn = Program {
            byte_code,
            instructions: BTreeMap::new(),
            blocks: BTreeMap::new()
        };

        rtn.fill_instructions();

        rtn
    }
}
