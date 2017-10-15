#![feature(iterator_step_by)]

extern crate ethcore;
extern crate ethcore_devtools;
extern crate ethcore_io;
extern crate ethcore_util;
extern crate ethkey;
extern crate evm;

use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::collections::btree_map::BTreeMap;

type ByteCode = std::vec::Vec<u8>;

struct Instruction {
    offset: usize,
    op_code: u8,
    data: std::vec::Vec<u8>
}

impl Instruction {
    pub fn new(offset: usize, opCode: u8) -> Self {
        Instruction {offset, op_code: opCode, data: std::vec::Vec::new()}
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
    instructions: BTreeMap<usize, Instruction>
}

fn is_jump(op_code: u8) -> bool {
    return false; //op_code == evm::JUMP || op_code == evm::JUMPI;
}

impl Program {
    pub fn fill_blocks(self: &mut Program) {
        let mut lastStart = 0;
        for (pos, instr) in &self.instructions {

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
            for idx in 0..evm::push_bytes(bc) {
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

    let mut p = Program::new(byte_code);

    for (pos, instr) in p.instructions {
        println!("{} {:?}", evm::INSTRUCTIONS[instr.op_code as usize].name, instr.data);
    }
}

fn main() {
    let file_name = env::args().nth(1).expect("Please provide a file");
    read_file(&file_name);
}