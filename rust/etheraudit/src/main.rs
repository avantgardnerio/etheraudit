#![feature(iterator_step_by)]
#[macro_use]
extern crate lazy_static;

extern crate ethcore;
extern crate ethcore_devtools;
extern crate ethcore_io;
extern crate ethcore_util;
extern crate ethkey;

use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::collections::btree_map::BTreeMap;

mod instructions;

type ByteCode = std::vec::Vec<u8>;

struct Instruction {
    offset: usize,
    op_code: u8,
    data: std::vec::Vec<u8>
}

impl Instruction {
    pub fn new(offset: usize, op_code: u8) -> Self {
        Instruction {offset, op_code: op_code, data: std::vec::Vec::new()}
    }
}

struct Block {
    start: usize,
    end: usize
}

impl Block {
    pub fn new(start: usize, end: usize) -> Self {
        Block {
            start,
            end
        }
    }
}

struct Program {
    byte_code: ByteCode,
    instructions: BTreeMap<usize, Instruction>,
    blocks: BTreeMap<usize, Block>
}


fn is_stop(op_code: u8) -> bool {
    return op_code == instructions::STOP ||
        op_code == instructions::RETURN;
}

fn is_jump(op_code: u8) -> bool {
    return op_code == instructions::JUMP ||
        op_code == instructions::JUMPI;
}

impl Program {
    pub fn fill_blocks(self: &mut Program) {
        let mut start = 0;
        let mut this_end = Some(0);

        for (pos, instr) in &self.instructions {
            if instr.op_code == instructions::JUMPDEST {
                match this_end {
                    Some(end) => { self.blocks.insert(start, Block::new(start, end)); },
                    None => {}
                }
                start = *pos;
            }

            this_end = Some(pos + instr.data.len() + 1);
            if is_jump(instr.op_code) || is_stop(instr.op_code) {
                match this_end {
                    Some(end) => {
                        self.blocks.insert(start, Block::new(start, end));
                        start = end;
                    },
                    None => {}
                }

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
            for _ in 0..instructions::push_bytes(bc) {
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

fn read_file(file_name: &String) {
    println!("{}", file_name);

    let mut f = File::open(&file_name).expect("file not found");

    let mut contents = String::new();
    f.read_to_string(&mut contents)
        .expect("something went wrong reading the file");

    let mut byte_code: ByteCode = std::vec::Vec::new();

    for idx in (0..contents.len()).step_by(2) {
        let bc = &contents[idx..idx+2];
        let ibc = u8::from_str_radix(bc, 16).expect("Couldn't parse");
        byte_code.push(ibc);
    }

    let p = Program::new(byte_code);

    for (_, block) in p.blocks {
        println!("Block {} {}", block.start, block.end);
        for pos in block.start..block.end {
            match p.instructions.get(&pos) {
                Some(instr) => {
                    println!("{}\t{} {:?}", pos, instructions::INSTRUCTIONS[instr.op_code as usize].name, instr.data);
                }, None => {}
            }
        }
        println!("");
    }
}

fn main() {
    let def_file = "/home/justin/source/etheraudit/cpp/0x273930d21e01ee25e4c219b63259d214872220a2.bc".to_string();
    let file_name = env::args().nth(1).unwrap_or(def_file);
    read_file(&file_name);
}