use std;

use program;
use op_codes;
use std::rc::*;

use expression;
use std::borrow::Borrow;
use std::ops::Deref;
use num::ToPrimitive;
use num;

use program::EvmStack;

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
    fn extract_inputs(&self, stack: &mut EvmStack) -> InstructionInputs {
        let mut inputs = InstructionInputs::new();
        let op_info = &op_codes::OPCODES[self.op_code as usize];

        for _ in 0..op_info.args {
            let entry = match stack.pop() {
                Some(e) => e,
                None => {
                    panic!("There should be no floating values here")
                }
            };

            inputs.push(entry);
        }

        inputs
    }

    pub fn iterate_stack(&self, stack: &mut EvmStack) -> Vec<(expression::Expression, usize)> {
        let inputs = self.extract_inputs(stack);
        println!("{} {:?}", op_codes::OPCODES[self.op_code as usize].name, inputs);
        let outputs = self.solve_outputs_for_inputs(&inputs);

        for output in outputs.iter().rev() {
            stack.push(Rc::clone(output));
        }

        if self.is_stop() {
            return vec![];
        } else if self.is_jump() {
            let condition = if self.op_code == op_codes::JUMPI {
                inputs[1].deref().clone()
            } else {
                expression::make_true()
            };

            let does_branch = match inputs.first().unwrap().deref() {
                &expression::Expression::Constant(ref v) => v.to_usize().unwrap(),
                val => panic!("Can't be dynamic here: {:?}", val)
            };

            let doesnt_branch = self.offset + 1;

            match expression::eval_as_bool(&condition) {
                Some(false) => {
                    vec![(expression::make_true(), doesnt_branch)]
                }
                Some(true) => {
                    vec![(expression::make_true(), does_branch)]
                }
                None => {
                    vec![(condition.clone(), does_branch),
                         (expression::negate(&condition), doesnt_branch)]
                }
            }
        } else {
            vec![(expression::make_true(), self.offset + 1)]
        }
    }
    pub fn is_jump(&self) -> bool{
        op_codes::is_jump(self.op_code)
    }
    pub fn is_stop(&self) -> bool {
        op_codes::is_stop(self.op_code)
    }
    pub fn solve_outputs_for_inputs(&self, inputs: &InstructionInputs) -> InstructionOutputs {
        match self.op_code {
            op_codes::PUSH1 ... op_codes::PUSH32 => {
                assert_eq!(self.inputs.len(), 0);
                return vec![ Rc::new(expression::make_constant(&self.data)) ];
            },
            op_codes::SWAP1 ... op_codes::SWAP16 => {
                let swap_pos = op_codes::get_swap_position(self.op_code);
                let mut outputs = inputs.to_vec();
                outputs.swap(0, swap_pos);
                return outputs;
            },
            op_codes::DUP1 ... op_codes::DUP16 => {
                let dup_pos = op_codes::get_dup_position(self.op_code);
                let mut outputs = vec![Rc::clone(&inputs[dup_pos])];
                outputs.extend (inputs.to_vec());
                return outputs;
            }
            _ => {
                let info = op_codes::OPCODES[self.op_code as usize];
                assert!(info.ret <= 1);

                if op_codes::is_arithmetic(self.op_code) {
                    let inputs: Option<std::vec::Vec<num::BigInt>> =
                        inputs.iter().map(
                            |x| if let expression::Expression::Constant(ref v) = *x.borrow() { Some(v.clone()) } else { None })
                            .collect();

                    if let Some(inputs) = inputs {
                        return vec![ Rc::new( expression::Expression::Constant(op_codes::solve(self.op_code, &inputs)))];
                    }
                }

                if info.ret == 0 {
                    return vec![]
                } else
                {
                    return vec![Rc::new(expression::Expression::ResultOf(self.offset) )]
                }
            }
        }
    }

    fn solve_outputs(&mut self) {
        self.outputs = self.solve_outputs_for_inputs(&self.inputs);
    }

    pub fn new(offset: usize, op_code: u8,
               data: &[u8], arg_count: &mut usize,
               stack: &mut program::EvmStack) -> Rc<Self> {
        let mut inputs = InstructionInputs::new();
        let op_info = &op_codes::OPCODES[op_code as usize];
        for _ in 0..op_info.args {
            let entry = match stack.pop() {
                Some(e) => e,
                None => {
                    *arg_count += 1;
                    Rc::new( expression::Expression::Argument(*arg_count-1))
                }
            };

            inputs.push(entry);
        }

        let mut instr = Instruction {offset, op_code, data: data.to_vec(), inputs, outputs: vec![]};
        instr.solve_outputs();

        for output in instr.outputs.iter().rev() {
            stack.push(Rc::clone(output));
        }
        //println!("{} {:?}", offset, stack);

        Rc::new(instr)
    }

    pub fn applied_stack(&self, stack: &mut program::EvmStack) -> Rc<Self> {
        let mut arg_count : usize = 0;
        Instruction::new(self.offset, self.op_code, &self.data, &mut arg_count, stack)
    }

    pub fn from_stack(in_offset: &mut usize,
                      byte_code: &[u8],
                      stack: &mut program::EvmStack, arg_count: &mut usize) -> Rc<Self> {
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

        Instruction::new(offset, op_code, &data, arg_count, stack)
    }

}
