#![feature(iterator_step_by)]
#![feature(slice_patterns)]
#[macro_use]
extern crate lazy_static;
extern crate num;
use std::env;
use std::fs::File;
use std::io::prelude::*;

mod op_codes;
mod instruction;
mod block;
mod program;
mod expression;
use std::rc::Rc;

use program::*;
use num::bigint::BigInt;
use num::Zero;

fn parse_byte_code(str: &str) -> ByteCode {
    let mut byte_code: ByteCode = std::vec::Vec::new();

    for idx in (0..str.len()).step_by(2) {
        let bc = &str[idx..idx+2];
        let ibc = u8::from_str_radix(bc, 16).expect("Couldn't parse");
        byte_code.push(ibc);
    }

    byte_code
}

fn read_file(file_name: &str) -> Program {
    println!("{}", file_name);

    let mut f = File::open(&file_name).expect("file not found");

    let mut contents = String::new();
    f.read_to_string(&mut contents)
        .expect("something went wrong reading the file");

    let byte_code = parse_byte_code(&contents);

    Program::new(byte_code)
}

fn process_program(p: &Program) {
    for block in p.blocks.values() {
        println!("Block {} {}", block.start, block.end);
        for pos in block.start..block.end {
            if let Some(instr) = p.instructions.get(&pos) {
                if !op_codes::is_stack_manip_only(instr.op_code) {
                    println!("{}\t{:?} := {}({:?})", pos,
                             instr.outputs, op_codes::OPCODES[instr.op_code as usize].name,
                             instr.inputs);
                }
            }
        }

        println!();
    }

    use expression::OpTree;
    let assert_query = OpTree::Operation(op_codes::JUMPI, vec![
        OpTree::Constant(BigInt::from(2)), OpTree::Query("condition".to_string())
    ]);

    for r in p.query(&assert_query) {
        println!("{:?}", r);
    }
}

fn main() {
    let def_file = "/home/justin/source/etheraudit/cpp/0x273930d21e01ee25e4c219b63259d214872220a2.bc".to_string();
    let file_name = env::args().nth(1).unwrap_or(def_file);
    let program = read_file(&file_name);
    process_program(&program);
}