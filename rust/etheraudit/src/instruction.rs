use std;

use expression::*;
use program;
use op_codes;
use std::rc::Rc;

use expression;
use std::borrow::Borrow;
use num;

pub type InstructionInputs = program::EvmStack;
pub type InstructionOutputs = program::EvmStack;

pub struct Instruction {
    pub offset: usize,
    pub op_code: u8,
    pub data: std::vec::Vec<u8>,

    pub inputs: InstructionInputs,
    pub outputs: InstructionOutputs,
}

impl Instruction {
    fn solve_outputs(self: &mut Instruction) {
        match self.op_code {
            op_codes::PUSH1 ... op_codes::PUSH32 => {
                assert_eq!(self.inputs.len(), 0);
                self.outputs = vec![ Rc::new(expression::make_constant(&self.data)) ];
            },
            op_codes::SWAP1 ... op_codes::SWAP16 => {
                let swap_pos = op_codes::get_swap_position(self.op_code);
                self.outputs = self.inputs.to_vec();
                self.outputs.swap(0, swap_pos);
            },
            op_codes::DUP1 ... op_codes::DUP16 => {
                let dup_pos = op_codes::get_dup_position(self.op_code);
                self.outputs = vec![Rc::clone(&self.inputs[dup_pos])];
                self.outputs.extend (self.inputs.to_vec());
            }
            _ => {
                let info = op_codes::OPCODES[self.op_code as usize];
                assert!(info.ret <= 1);

                if op_codes::is_arithmetic(self.op_code) {
                    let inputs: Option<std::vec::Vec<num::BigInt>> =
                        self.inputs.iter().map(
                            |x| if let expression::Expression::Constant(ref v) = *x.borrow() { Some(v.clone()) } else { None })
                            .collect();

                    if let Some(inputs) = inputs {
                        self.outputs = vec![ Rc::new( expression::Expression::Constant(op_codes::solve(self.op_code, &inputs)))];
                        return;
                    }
                }

                if info.ret == 0 {
                    self.outputs = vec![]
                } else
                {
                    self.outputs = vec![Rc::new(expression::Expression::ResultOf(self.offset) )]
                }
            }
        }
    }

    pub fn from_stack(in_offset: &mut usize,
                      byte_code: &[u8],
                      stack: &mut program::EvmStack, arg_count: &mut usize) -> Self {
        let offset : usize = *in_offset;
        let op_code = byte_code[*in_offset];
        let mut data = std::vec::Vec::<u8>::new();
        *in_offset += 1;
        for _ in 0..op_codes::push_bytes(op_code) {
            if offset < byte_code.len() {
                data.push(byte_code[*in_offset]);
            }
            *in_offset += 1;
        }

        let mut inputs = InstructionInputs::new();
        let op_info = &op_codes::OPCODES[op_code as usize];
        for i in 0..op_info.args {
            let entry = match stack.pop() {
                Some(e) => e,
                None => {
                    *arg_count += 1;
                    Rc::new( expression::Expression::Argument(*arg_count-1))
                }
            };

            inputs.push(entry);
        }

        let mut instr = Instruction {offset, op_code, data, inputs, outputs: vec![]};
        instr.solve_outputs();

        for output in instr.outputs.iter().rev() {
            stack.push(Rc::clone(output));
        }
        //println!("{} {:?}", offset, stack);

        instr
    }

}
