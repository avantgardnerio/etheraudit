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
    pub blocks: BTreeMap<usize, Block>,
    pub op_trees: BTreeMap<usize, expression::OpTree>
}

impl Program {
    pub fn query(self: &Program,query: &expression::OpTree) -> std::vec::Vec< (usize, expression::QueryResult) > {
        self.op_trees.iter().filter_map(|(pos, tree)| {
            tree.query(query).map(|ans| (*pos, ans))
        }).collect()
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

            let new_tree = expression::OpTree::create_from_instr(&self, &instr);
            self.op_trees.insert(instr.offset, new_tree);
            self.instructions.insert(instr.offset, instr);
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

        rtn
    }
}
