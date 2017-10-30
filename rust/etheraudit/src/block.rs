use program;
use std;
use std::*;
use std::vec::*;

use memory::SharedPtr;
use memory::WeakPtr;

pub struct Block {
    pub start: usize,
    pub end: usize,
    pub exit_stack: program::EvmStack,
    pub entries: Vec< WeakPtr<Block>>,
    pub exits: Vec< WeakPtr<Block>>,
    pub dynamic_jump: bool,
    pub gas_price: usize,
    pub reachable: bool
}

impl Block {
    pub fn new(start: usize, end: usize, exit_stack: program::EvmStack, gas_price: usize) -> SharedPtr<Self> {
        SharedPtr::new(Block {
            start,
            end,
            exit_stack,
            entries: std::vec::Vec::new(),
            exits: std::vec::Vec::new(),
            dynamic_jump: false,
            gas_price,
            reachable: false
        })
    }

    pub fn exits(&self) -> Vec<SharedPtr<Block>>{
        self.exits.iter().filter_map(|x| x.upgrade() ).collect()
    }

    pub fn entries(&self) -> Vec<SharedPtr<Block>>{
        self.entries.iter().filter_map(|x| x.upgrade() ).collect()
    }
}
