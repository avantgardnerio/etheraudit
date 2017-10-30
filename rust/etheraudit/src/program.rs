use std::collections::btree_map::BTreeMap;
use std;

use expression;
use instruction::*;
use block::*;
use op_codes;
use std::rc::*;
use std::vec::*;
//use std::sync::*;
use num::ToPrimitive;
use memory::*;
use std::ops::Deref;

pub type ByteCode = std::vec::Vec<u8>;
pub type EvmStack = std::vec::Vec< Rc<expression::Expression> >;

pub struct Program {
    pub byte_code: ByteCode,
    pub instructions: BTreeMap<usize, Rc<Instruction> >,
    pub blocks: BTreeMap<usize, SharedPtr<Block>>,
    pub op_trees: BTreeMap<usize, expression::OpTree>
}

impl Program {
    pub fn blocks(&self) -> Vec<SharedPtr<Block>> {
        self.blocks.values().map(SharedPtr::clone).collect()
    }
    pub fn query(&self,query: &expression::OpTree) -> std::vec::Vec< (usize, expression::QueryResult) > {
        self.op_trees.iter().filter_map(|(pos, tree)| {
            tree.query(query).map(|ans| (*pos, ans))
        }).collect()
    }
    pub fn calc_gas_price(&self, start: usize, end: usize) -> usize {
        let mut rtn = 0;
        for idx in start..end {
            if let Some(instr) = self.instructions.get(&idx) {
                rtn += op_codes::get_gas_price(instr.op_code);
            }
        }
        rtn
    }
    pub fn insert_block(&mut self, start: usize, end: usize, stack: &EvmStack) {
        let gas_price = self.calc_gas_price(start, end);
        self.blocks.insert(start, Block::new(start, end, stack.to_vec(),
                                             gas_price));
    }

    pub fn solve_stack(&self, block: &Block, entry_stack: &EvmStack) -> (Vec<(expression::Expression, usize)>, EvmStack) {
        let mut rtn = entry_stack.clone();
        let mut exits : Vec<(expression::Expression, usize)> = Vec::new();

        for instr in self.instructions(block) {
            exits = instr.iterate_stack(&mut rtn);
        }
        (exits, rtn)
    }

    pub fn fill_instructions(&mut self) {
        let mut stack = EvmStack::new();
        let mut start: usize = 0;
        let mut this_end : Option<usize> = Some(0);
        let mut arg_count = 0;

        let mut offset = 0;
        while offset < self.byte_code.len() {
            let instr = Instruction::from_stack(&mut offset,
                                                &self.byte_code, &mut stack,
                                                &mut arg_count);

            if instr.op_code == op_codes::JUMPDEST {
                if let Some(end) = this_end {
                    self.insert_block(start, end, &stack);
                }
                start = offset - 1;
                stack.clear();
                arg_count = 0;
            }

            this_end = Some(offset);
            if op_codes::is_jump(instr.op_code) || op_codes::is_stop(instr.op_code) {
                if let Some(end) = this_end {
                    self.insert_block(start, end, &stack);
                    start = end;
                };

                this_end = None;
            }

            let new_tree = expression::OpTree::create_from_instr(&self, &instr);
            self.op_trees.insert(instr.offset, new_tree);
            self.instructions.insert(instr.offset, instr);
        }
    }

    pub fn instructions(&self, block: &Block) -> std::vec::Vec<Rc<Instruction>> {
        (block.start..block.end).filter_map(|x| self.instructions.get(&x) ).
            map(|x| x.clone()).
            collect()
    }

    fn is_valid_jump_dest(&self, dest: usize) -> bool {
        if let Some(instr) = self.instructions.get(&dest) {
                return instr.op_code == op_codes::JUMPDEST;
        }

        false
    }
    fn add_edge(&self, as_jump: bool, from_ptr: usize, to_ptr: usize) -> bool {
        if !as_jump || self.is_valid_jump_dest(to_ptr) {
            let from = self.blocks.get(&from_ptr);
            let to = self.blocks.get(&to_ptr);
            match (from, to) {
                (Some(from), Some(to)) => {
                    from.get_mut().exits.push(to.downgrade());
                    to.get_mut().entries.push(from.downgrade());
                    return true;
                },
                _ => println!("Edge added between invalid blocks {:?} {:?}", from_ptr, to_ptr)
            }
        }
        false
    }

    fn fill_block_references(&mut self) {
        for block_ptr in self.blocks() {
            let instructions = self.instructions(&block_ptr.get());
            assert!(instructions.len() > 0);
            let start = block_ptr.get().start;
            let end = block_ptr.get().end;
            if let Some(instr) = instructions.last() {
                if instr.op_code == op_codes::JUMP || instr.op_code == op_codes::JUMPI {
                    assert!(!instr.inputs.is_empty());
                    if let expression::Expression::Constant(ref dest) = *instr.inputs[0] {
                        if let Some(v) = dest.to_usize() {
                            self.add_edge(true,start, v);
                        }
                    } else {
                        block_ptr.get_mut().dynamic_jump = true;
                    }
                }

                if !op_codes::is_stop(instr.op_code) &&
                    instr.op_code != op_codes::JUMP &&
                    self.instructions.contains_key(&end) {
                    self.add_edge(false,start, end);
                }
            }

        };
    }

    fn solve_dynamics(&mut self) {
        let dynamic_set : std::collections::HashSet<usize> =
        self.blocks().iter().map(SharedPtr::get).
            filter(|b|b.dynamic_jump).
            map(|b|b.start).collect();

        //Vec<(usize, Vec<SharedPtr<Block>>, EvmStack)>
        let mut states = Vec::new();
        states.push((500000, vec![self.blocks[&0].clone()], EvmStack::new()));

        while states.len() > 0 {
            let mut new_states = Vec::new();

            for (gas_limit, blocks, stack) in states {
                let mut block = blocks.last().unwrap().get_mut();
                block.reachable = true;

                let (exits, new_stack ) =
                    self.solve_stack(block.deref(), &stack);

                for (cond, next) in exits {
                    if self.is_valid_jump_dest(next) || block.end == next {
                        let b= self.blocks.get(&next).unwrap();
                        let mut new_blocks = blocks.clone();
                        new_blocks.push(b.clone());
                        new_states.push( ( gas_limit - block.gas_price,new_blocks, new_stack.clone() ) );
                    } else {
                        println!("Invalid jump to {}", next);
                    }
                }
            }

            states = new_states;
        }

    }

    pub fn new(byte_code: ByteCode) -> Self {
        let mut rtn = Program {
            byte_code,
            instructions: BTreeMap::new(),
            blocks: BTreeMap::new(),
            op_trees: BTreeMap::new()
        };

        rtn.fill_instructions();
        rtn.fill_block_references();
        rtn.solve_dynamics();

        rtn
    }
}
