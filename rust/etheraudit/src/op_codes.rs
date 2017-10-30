#![allow(dead_code)]

extern crate arrayvec;

use num;
use num::Zero;

use std;
use num::ToPrimitive;
use self::arrayvec::ArrayVec;
use num::bigint::ToBigInt;

pub type Instruction = u8;

/// Returns true if given instruction is `PUSHN` instruction.
pub fn is_push(i: Instruction) -> bool {
	i >= PUSH1 && i <= PUSH32
}

pub fn is_stop(op_code: u8) -> bool {
	op_code == STOP || op_code == RETURN
}

pub fn is_stack_manip_only(op_code: Instruction) -> bool {
	match op_code {
		DUP1 ... DUP16 => true,
		SWAP1 ... SWAP16 => true,
		PUSH1 ... PUSH32 => true,
		POP => true,
		_ => false
	}
}

pub fn is_jump(op_code: u8) -> bool {
	op_code == JUMP || op_code == JUMPI
}

#[test]
fn test_is_push() {
	assert!(is_push(PUSH1));
	assert!(is_push(PUSH32));
	assert!(!is_push(DUP1));
}

/// Returns number of bytes to read for `PUSHN` instruction
/// PUSH1 -> 1
pub fn get_push_bytes(i: Instruction) -> usize {
	assert!(is_push(i), "Only for PUSH instructions.");
	(i - PUSH1 + 1) as usize
}

/// Returns number of bytes to read for `PUSHN` instruction or 0.
pub fn push_bytes(i: Instruction) -> usize {
	if is_push(i) {
		get_push_bytes(i)
	} else {
		0
	}
}

#[test]
fn test_get_push_bytes() {
	assert_eq!(get_push_bytes(PUSH1), 1);
	assert_eq!(get_push_bytes(PUSH3), 3);
	assert_eq!(get_push_bytes(PUSH32), 32);
}

/// Returns stack position of item to duplicate
/// DUP1 -> 0
pub fn get_dup_position(i: Instruction) -> usize {
	assert!(i >= DUP1 && i <= DUP16);
	(i - DUP1) as usize
}

#[test]
fn test_get_dup_position() {
	assert_eq!(get_dup_position(DUP1), 0);
	assert_eq!(get_dup_position(DUP5), 4);
	assert_eq!(get_dup_position(DUP10), 9);
}

/// Returns stack position of item to SWAP top with
/// SWAP1 -> 1
pub fn get_swap_position(i: Instruction) -> usize {
	assert!(i >= SWAP1 && i <= SWAP16);
	(i - SWAP1 + 1) as usize
}

#[test]
fn test_get_swap_position() {
	assert_eq!(get_swap_position(SWAP1), 1);
	assert_eq!(get_swap_position(SWAP5), 5);
	assert_eq!(get_swap_position(SWAP10), 10);
}

/// Returns number of topcis to take from stack
/// LOG0 -> 0
pub fn get_log_topics (i: Instruction) -> usize {
	assert!(i >= LOG0 && i <= LOG4);
	(i - LOG0) as usize
}

#[test]
fn test_get_log_topics() {
	assert_eq!(get_log_topics(LOG0), 0);
	assert_eq!(get_log_topics(LOG2), 2);
	assert_eq!(get_log_topics(LOG4), 4);
}

#[derive(PartialEq, Clone, Copy)]
pub enum GasPriceTier {
	/// 0 Zero
	Zero,
	/// 2 Quick
	Base,
	/// 3 Fastest
	VeryLow,
	/// 5 Fast
	Low,
	/// 8 Mid
	Mid,
	/// 10 Slow
	High,
	/// 20 Ext
	Ext,
	/// Multiparam or otherwise special
	Special,
	/// Invalid
	Invalid
}

impl Default for GasPriceTier {
	fn default() -> Self {
		GasPriceTier::Invalid
	}
}

/// Returns the index in schedule for specific `GasPriceTier`
pub fn get_tier_idx (tier: GasPriceTier) -> usize {
	match tier {
		GasPriceTier::Zero => 0,
		GasPriceTier::Base => 1,
		GasPriceTier::VeryLow => 2,
		GasPriceTier::Low => 3,
		GasPriceTier::Mid => 4,
		GasPriceTier::High => 5,
		GasPriceTier::Ext => 6,
		GasPriceTier::Special => 7,
		GasPriceTier::Invalid => 8
	}
}

pub fn get_gas_price(instr: Instruction) -> usize {
	// https://ethereum.github.io/yellowpaper/paper.pdf -- Appendix G
	let opcode = OPCODES[instr as usize];
	match opcode.tier {
		GasPriceTier::Zero => 0,
		GasPriceTier::Base => 2,
		GasPriceTier::VeryLow => 3,
		GasPriceTier::Low => 5,
		GasPriceTier::Mid => 8,
		GasPriceTier::High => 10,
		GasPriceTier::Invalid => 0,
		_ => {
			match instr {
				EXTCODECOPY | EXTCODESIZE => 700,
				BALANCE => 400,
				SLOAD => 200,
				JUMPDEST => 1,
				SSTORE => 5000,
				EXP => 10,
				SHA3 => 30,
				LOG1 ... LOG4 => 375,
				CALL => 700,
				SUICIDE => 0,
				_ => panic!("Please add instruction to listing {} {:#x}", OPCODES[instr as usize].name, instr)
			}
		}
	}
}
/// EVM instruction information.
#[derive(Copy, Clone, Default)]
pub struct OpCodeInfo {
	/// Mnemonic name.
	pub name: &'static str,
	pub op_code: u8,
	/// Number of stack arguments.
	pub args: usize,
	/// Number of returned stack items.
	pub ret: usize,
	/// Gas price tier.
	pub tier: GasPriceTier
}

impl OpCodeInfo {
	/// Create new instruction info.
	pub fn new(name: &'static str, op_code: u8, args: usize, ret: usize, tier: GasPriceTier) -> Self {
		OpCodeInfo {
			name: name,
			op_code: op_code,
			args: args,
			ret: ret,
			tier: tier
		}
	}
}

lazy_static! {
	/// Static instruction table.
	pub static ref OPCODES: [OpCodeInfo; 0x100] = {
		let mut arr : [OpCodeInfo; 0x100] = (0..0x100).map(|i|{
		     OpCodeInfo::new("", i as u8, 0, 0, GasPriceTier::Invalid)
		}).collect::<ArrayVec<_>>().into_inner()
        .unwrap_or_else(|_| unreachable!());

		arr[STOP as usize] =			OpCodeInfo::new("STOP", STOP,			0, 0, GasPriceTier::Zero);
		arr[ADD as usize] = 			OpCodeInfo::new("ADD", ADD,				2, 1, GasPriceTier::VeryLow);
		arr[SUB as usize] = 			OpCodeInfo::new("SUB", SUB,				2, 1, GasPriceTier::VeryLow);
		arr[MUL as usize] = 			OpCodeInfo::new("MUL", MUL,				2, 1, GasPriceTier::Low);
		arr[DIV as usize] = 			OpCodeInfo::new("DIV", DIV,				2, 1, GasPriceTier::Low);
		arr[SDIV as usize] =			OpCodeInfo::new("SDIV", SDIV,			2, 1, GasPriceTier::Low);
		arr[MOD as usize] = 			OpCodeInfo::new("MOD", MOD,				2, 1, GasPriceTier::Low);
		arr[SMOD as usize] =			OpCodeInfo::new("SMOD", SMOD,			2, 1, GasPriceTier::Low);
		arr[EXP as usize] = 			OpCodeInfo::new("EXP", EXP,				2, 1, GasPriceTier::Special);
		arr[NOT as usize] = 			OpCodeInfo::new("NOT", NOT,				1, 1, GasPriceTier::VeryLow);
		arr[LT as usize] =				OpCodeInfo::new("LT", LT,				2, 1, GasPriceTier::VeryLow);
		arr[GT as usize] =				OpCodeInfo::new("GT", GT,				2, 1, GasPriceTier::VeryLow);
		arr[SLT as usize] = 			OpCodeInfo::new("SLT", SLT,				2, 1, GasPriceTier::VeryLow);
		arr[SGT as usize] = 			OpCodeInfo::new("SGT", SGT,				2, 1, GasPriceTier::VeryLow);
		arr[EQ as usize] =				OpCodeInfo::new("EQ", EQ,				2, 1, GasPriceTier::VeryLow);
		arr[ISZERO as usize] =			OpCodeInfo::new("ISZERO", ISZERO,			1, 1, GasPriceTier::VeryLow);
		arr[AND as usize] = 			OpCodeInfo::new("AND", AND,				2, 1, GasPriceTier::VeryLow);
		arr[OR as usize] =				OpCodeInfo::new("OR", OR,				2, 1, GasPriceTier::VeryLow);
		arr[XOR as usize] = 			OpCodeInfo::new("XOR", XOR,				2, 1, GasPriceTier::VeryLow);
		arr[BYTE as usize] =			OpCodeInfo::new("BYTE", BYTE,			2, 1, GasPriceTier::VeryLow);
		arr[ADDMOD as usize] =			OpCodeInfo::new("ADDMOD", ADDMOD,			3, 1, GasPriceTier::Mid);
		arr[MULMOD as usize] =			OpCodeInfo::new("MULMOD", MULMOD,			3, 1, GasPriceTier::Mid);
		arr[SIGNEXTEND as usize] =		OpCodeInfo::new("SIGNEXTEND", SIGNEXTEND,		2, 1, GasPriceTier::Low);
		arr[RETURNDATASIZE as usize] =	OpCodeInfo::new("RETURNDATASIZE", RETURNDATASIZE,	0, 1, GasPriceTier::Base);
		arr[RETURNDATACOPY as usize] =	OpCodeInfo::new("RETURNDATACOPY", RETURNDATACOPY,	3, 0, GasPriceTier::VeryLow);
		arr[SHA3 as usize] =			OpCodeInfo::new("SHA3", SHA3,			2, 1, GasPriceTier::Special);
		arr[ADDRESS as usize] = 		OpCodeInfo::new("ADDRESS", ADDRESS,			0, 1, GasPriceTier::Base);
		arr[BALANCE as usize] = 		OpCodeInfo::new("BALANCE", BALANCE,			1, 1, GasPriceTier::Special);
		arr[ORIGIN as usize] =			OpCodeInfo::new("ORIGIN", ORIGIN,			0, 1, GasPriceTier::Base);
		arr[CALLER as usize] =			OpCodeInfo::new("CALLER", CALLER,			0, 1, GasPriceTier::Base);
		arr[CALLVALUE as usize] =		OpCodeInfo::new("CALLVALUE", CALLVALUE,		0, 1, GasPriceTier::Base);
		arr[CALLDATALOAD as usize] =	OpCodeInfo::new("CALLDATALOAD", CALLDATALOAD,	1, 1, GasPriceTier::VeryLow);
		arr[CALLDATASIZE as usize] =	OpCodeInfo::new("CALLDATASIZE", CALLDATASIZE,	0, 1, GasPriceTier::Base);
		arr[CALLDATACOPY as usize] =	OpCodeInfo::new("CALLDATACOPY", CALLDATACOPY,	3, 0, GasPriceTier::VeryLow);
		arr[CODESIZE as usize] =		OpCodeInfo::new("CODESIZE", CODESIZE,		0, 1, GasPriceTier::Base);
		arr[CODECOPY as usize] =		OpCodeInfo::new("CODECOPY", CODECOPY,		3, 0, GasPriceTier::VeryLow);
		arr[GASPRICE as usize] =		OpCodeInfo::new("GASPRICE", GASPRICE,		0, 1, GasPriceTier::Base);
		arr[EXTCODESIZE as usize] = 	OpCodeInfo::new("EXTCODESIZE", EXTCODESIZE,		1, 1, GasPriceTier::Special);
		arr[EXTCODECOPY as usize] = 	OpCodeInfo::new("EXTCODECOPY", EXTCODECOPY,		4, 0, GasPriceTier::Special);
		arr[BLOCKHASH as usize] =		OpCodeInfo::new("BLOCKHASH", BLOCKHASH,		1, 1, GasPriceTier::Ext);
		arr[COINBASE as usize] =		OpCodeInfo::new("COINBASE", COINBASE,		0, 1, GasPriceTier::Base);
		arr[TIMESTAMP as usize] =		OpCodeInfo::new("TIMESTAMP", TIMESTAMP,		0, 1, GasPriceTier::Base);
		arr[NUMBER as usize] =			OpCodeInfo::new("NUMBER", NUMBER,			0, 1, GasPriceTier::Base);
		arr[DIFFICULTY as usize] =		OpCodeInfo::new("DIFFICULTY", DIFFICULTY,		0, 1, GasPriceTier::Base);
		arr[GASLIMIT as usize] =		OpCodeInfo::new("GASLIMIT", GASLIMIT,		0, 1, GasPriceTier::Base);
		arr[POP as usize] = 			OpCodeInfo::new("POP", POP,				1, 0, GasPriceTier::Base);
		arr[MLOAD as usize] =			OpCodeInfo::new("MLOAD", MLOAD,			1, 1, GasPriceTier::VeryLow);
		arr[MSTORE as usize] =			OpCodeInfo::new("MSTORE", MSTORE,			2, 0, GasPriceTier::VeryLow);
		arr[MSTORE8 as usize] = 		OpCodeInfo::new("MSTORE8", MSTORE8,			2, 0, GasPriceTier::VeryLow);
		arr[SLOAD as usize] =			OpCodeInfo::new("SLOAD", SLOAD,			1, 1, GasPriceTier::Special);
		arr[SSTORE as usize] =			OpCodeInfo::new("SSTORE", SSTORE,			2, 0, GasPriceTier::Special);
		arr[JUMP as usize] =			OpCodeInfo::new("JUMP", JUMP,			1, 0, GasPriceTier::Mid);
		arr[JUMPI as usize] =			OpCodeInfo::new("JUMPI", JUMPI,			2, 0, GasPriceTier::High);
		arr[PC as usize] =				OpCodeInfo::new("PC", PC,				0, 1, GasPriceTier::Base);
		arr[MSIZE as usize] =			OpCodeInfo::new("MSIZE", MSIZE,			0, 1, GasPriceTier::Base);
		arr[GAS as usize] = 			OpCodeInfo::new("GAS", GAS,				0, 1, GasPriceTier::Base);
		arr[JUMPDEST as usize] =		OpCodeInfo::new("JUMPDEST", JUMPDEST,		0, 0, GasPriceTier::Special);
		arr[PUSH1 as usize] =			OpCodeInfo::new("PUSH1", PUSH1,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH2 as usize] =			OpCodeInfo::new("PUSH2", PUSH2,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH3 as usize] =			OpCodeInfo::new("PUSH3", PUSH3,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH4 as usize] =			OpCodeInfo::new("PUSH4", PUSH4,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH5 as usize] =			OpCodeInfo::new("PUSH5", PUSH5,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH6 as usize] =			OpCodeInfo::new("PUSH6", PUSH6,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH7 as usize] =			OpCodeInfo::new("PUSH7", PUSH7,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH8 as usize] =			OpCodeInfo::new("PUSH8", PUSH8,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH9 as usize] =			OpCodeInfo::new("PUSH9", PUSH9,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH10 as usize] =			OpCodeInfo::new("PUSH10", PUSH10,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH11 as usize] =			OpCodeInfo::new("PUSH11", PUSH11,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH12 as usize] =			OpCodeInfo::new("PUSH12", PUSH12,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH13 as usize] =			OpCodeInfo::new("PUSH13", PUSH13,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH14 as usize] =			OpCodeInfo::new("PUSH14", PUSH14,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH15 as usize] =			OpCodeInfo::new("PUSH15", PUSH15,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH16 as usize] =			OpCodeInfo::new("PUSH16", PUSH16,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH17 as usize] =			OpCodeInfo::new("PUSH17", PUSH17,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH18 as usize] =			OpCodeInfo::new("PUSH18", PUSH18,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH19 as usize] =			OpCodeInfo::new("PUSH19", PUSH19,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH20 as usize] =			OpCodeInfo::new("PUSH20", PUSH20,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH21 as usize] =			OpCodeInfo::new("PUSH21", PUSH21,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH22 as usize] =			OpCodeInfo::new("PUSH22", PUSH22,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH23 as usize] =			OpCodeInfo::new("PUSH23", PUSH23,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH24 as usize] =			OpCodeInfo::new("PUSH24", PUSH24,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH25 as usize] =			OpCodeInfo::new("PUSH25", PUSH25,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH26 as usize] =			OpCodeInfo::new("PUSH26", PUSH26,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH27 as usize] =			OpCodeInfo::new("PUSH27", PUSH27,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH28 as usize] =			OpCodeInfo::new("PUSH28", PUSH28,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH29 as usize] =			OpCodeInfo::new("PUSH29", PUSH29,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH30 as usize] =			OpCodeInfo::new("PUSH30", PUSH30,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH31 as usize] =			OpCodeInfo::new("PUSH31", PUSH31,			0, 1, GasPriceTier::VeryLow);
		arr[PUSH32 as usize] =			OpCodeInfo::new("PUSH32", PUSH32,			0, 1, GasPriceTier::VeryLow);
		arr[DUP1 as usize] =			OpCodeInfo::new("DUP1", DUP1,			1, 2, GasPriceTier::VeryLow);
		arr[DUP2 as usize] =			OpCodeInfo::new("DUP2", DUP2,			2, 3, GasPriceTier::VeryLow);
		arr[DUP3 as usize] =			OpCodeInfo::new("DUP3", DUP3,			3, 4, GasPriceTier::VeryLow);
		arr[DUP4 as usize] =			OpCodeInfo::new("DUP4", DUP4,			4, 5, GasPriceTier::VeryLow);
		arr[DUP5 as usize] =			OpCodeInfo::new("DUP5", DUP5,			5, 6, GasPriceTier::VeryLow);
		arr[DUP6 as usize] =			OpCodeInfo::new("DUP6", DUP6,			6, 7, GasPriceTier::VeryLow);
		arr[DUP7 as usize] =			OpCodeInfo::new("DUP7", DUP7,			7, 8, GasPriceTier::VeryLow);
		arr[DUP8 as usize] =			OpCodeInfo::new("DUP8", DUP8,			8, 9, GasPriceTier::VeryLow);
		arr[DUP9 as usize] =			OpCodeInfo::new("DUP9", DUP9,			9, 10, GasPriceTier::VeryLow);
		arr[DUP10 as usize] =			OpCodeInfo::new("DUP10", DUP10,			10, 11, GasPriceTier::VeryLow);
		arr[DUP11 as usize] =			OpCodeInfo::new("DUP11", DUP11,			11, 12, GasPriceTier::VeryLow);
		arr[DUP12 as usize] =			OpCodeInfo::new("DUP12", DUP12,			12, 13, GasPriceTier::VeryLow);
		arr[DUP13 as usize] =			OpCodeInfo::new("DUP13", DUP13,			13, 14, GasPriceTier::VeryLow);
		arr[DUP14 as usize] =			OpCodeInfo::new("DUP14", DUP14,			14, 15, GasPriceTier::VeryLow);
		arr[DUP15 as usize] =			OpCodeInfo::new("DUP15", DUP15,			15, 16, GasPriceTier::VeryLow);
		arr[DUP16 as usize] =			OpCodeInfo::new("DUP16", DUP16,			16, 17, GasPriceTier::VeryLow);
		arr[SWAP1 as usize] =			OpCodeInfo::new("SWAP1", SWAP1,			2, 2, GasPriceTier::VeryLow);
		arr[SWAP2 as usize] =			OpCodeInfo::new("SWAP2", SWAP2,			3, 3, GasPriceTier::VeryLow);
		arr[SWAP3 as usize] =			OpCodeInfo::new("SWAP3", SWAP3,			4, 4, GasPriceTier::VeryLow);
		arr[SWAP4 as usize] =			OpCodeInfo::new("SWAP4", SWAP4,			5, 5, GasPriceTier::VeryLow);
		arr[SWAP5 as usize] =			OpCodeInfo::new("SWAP5", SWAP5,			6, 6, GasPriceTier::VeryLow);
		arr[SWAP6 as usize] =			OpCodeInfo::new("SWAP6", SWAP6,			7, 7, GasPriceTier::VeryLow);
		arr[SWAP7 as usize] =			OpCodeInfo::new("SWAP7", SWAP7,			8, 8, GasPriceTier::VeryLow);
		arr[SWAP8 as usize] =			OpCodeInfo::new("SWAP8", SWAP8,			9, 9, GasPriceTier::VeryLow);
		arr[SWAP9 as usize] =			OpCodeInfo::new("SWAP9", SWAP9,			10, 10, GasPriceTier::VeryLow);
		arr[SWAP10 as usize] =			OpCodeInfo::new("SWAP10", SWAP10,			11, 11, GasPriceTier::VeryLow);
		arr[SWAP11 as usize] =			OpCodeInfo::new("SWAP11", SWAP11,			12, 12, GasPriceTier::VeryLow);
		arr[SWAP12 as usize] =			OpCodeInfo::new("SWAP12", SWAP12,			13, 13, GasPriceTier::VeryLow);
		arr[SWAP13 as usize] =			OpCodeInfo::new("SWAP13", SWAP13,			14, 14, GasPriceTier::VeryLow);
		arr[SWAP14 as usize] =			OpCodeInfo::new("SWAP14", SWAP14,			15, 15, GasPriceTier::VeryLow);
		arr[SWAP15 as usize] =			OpCodeInfo::new("SWAP15", SWAP15,			16, 16, GasPriceTier::VeryLow);
		arr[SWAP16 as usize] =			OpCodeInfo::new("SWAP16", SWAP16,			17, 17, GasPriceTier::VeryLow);
		arr[LOG0 as usize] =			OpCodeInfo::new("LOG0", LOG0,			2, 0, GasPriceTier::Special);
		arr[LOG1 as usize] =			OpCodeInfo::new("LOG1", LOG1,			3, 0, GasPriceTier::Special);
		arr[LOG2 as usize] =			OpCodeInfo::new("LOG2", LOG2,			4, 0, GasPriceTier::Special);
		arr[LOG3 as usize] =			OpCodeInfo::new("LOG3", LOG3,			5, 0, GasPriceTier::Special);
		arr[LOG4 as usize] =			OpCodeInfo::new("LOG4", LOG4,			6, 0, GasPriceTier::Special);
		arr[CREATE as usize] =			OpCodeInfo::new("CREATE", CREATE,			3, 1, GasPriceTier::Special);
		arr[CALL as usize] =			OpCodeInfo::new("CALL", CALL,			7, 1, GasPriceTier::Special);
		arr[CALLCODE as usize] =		OpCodeInfo::new("CALLCODE", CALLCODE,		7, 1, GasPriceTier::Special);
		arr[RETURN as usize] =			OpCodeInfo::new("RETURN", RETURN,			2, 0, GasPriceTier::Zero);
		arr[DELEGATECALL as usize] =	OpCodeInfo::new("DELEGATECALL", DELEGATECALL,	6, 1, GasPriceTier::Special);
		arr[STATICCALL as usize] =		OpCodeInfo::new("STATICCALL", STATICCALL,		6, 1, GasPriceTier::Special);
		arr[SUICIDE as usize] = 		OpCodeInfo::new("SUICIDE", SUICIDE,			1, 0, GasPriceTier::Special);
		arr[CREATE2 as usize] = 		OpCodeInfo::new("CREATE2", CREATE2,			3, 1, GasPriceTier::Special);
		arr[REVERT as usize] =			OpCodeInfo::new("REVERT", REVERT,			2, 0, GasPriceTier::Zero);
		arr
	};
}

pub fn get_infix(op_code: Instruction) -> Option<&'static str> {
	match op_code {
		EXP =>  Some("**"),
		DIV =>  Some("/"),
		NOT =>  Some("!"),
		ISZERO =>  Some("0 == "),
		SUB =>  Some("-"),
		OR =>  Some("||"),
		AND =>  Some("&"),
		ADD =>  Some("+"),
		GT =>  Some(">"),
		LT =>  Some("<"),
		XOR =>  Some("^"),
		MUL =>  Some("*"),
		MOD =>  Some("%"),
		EQ =>  Some("=="),
		SDIV =>  Some("//"),
		_ => None
	}
}

pub fn is_arithmetic(op_code: Instruction) -> bool {
	match op_code {
		EXP => true,
		DIV => true,
		NOT => true,
		ISZERO => true,
		SUB => true,
		OR => true,
		AND => true,
		ADD => true,
		GT => true,
		LT => true,
		XOR => true,
		MUL => true,
		MOD => true,
		EQ => true,
		SDIV => true,
		_ => false
	}
}
fn negate(v: num::BigUint) -> num::BigUint {
	let bytes : std::vec::Vec<u8> = v.to_bytes_be().iter().map(std::ops::Not::not).collect();
	num::BigUint::from_bytes_be(&bytes)
}

pub fn solve(opcode: Instruction,input: &std::vec::Vec<num::BigInt>) -> num::BigInt {
	assert!(is_arithmetic(opcode));
	assert_eq!(OPCODES[opcode as usize].args, input.len());

	match opcode {
		EXP => num::pow(input[0].clone(), input[1].to_usize().unwrap()),
		DIV => {
			if input[1].clone() == num::BigInt::zero() {
				num::BigInt::zero()
			} else {
				input[0].clone() / input[1].clone()
			}
		},
		NOT => (negate(input[0].to_biguint().unwrap())).to_bigint().unwrap(),
		ISZERO => num::BigInt::from( (input[0].clone() == num::BigInt::zero()) as usize ),
		SUB =>input[0].clone() - input[1].clone(),
		AND =>(input[0].clone().to_biguint().unwrap() & input[1].clone().to_biguint().unwrap()).to_bigint().unwrap(),
		ADD =>input[0].clone() + input[1].clone(),
		GT => num::BigInt::from( (input[0].clone() > input[1].clone()) as usize ),
		LT => num::BigInt::from( (input[0].clone() > input[1].clone()) as usize ),
		XOR =>(input[0].clone().to_biguint().unwrap() ^ input[1].clone().to_biguint().unwrap()).to_bigint().unwrap(),
		MUL =>input[0].clone() * input[1].clone(),
		MOD =>input[0].clone() % input[1].clone(),
		EQ => num::BigInt::from( (input[0].clone() == input[1].clone()) as usize ),
		SDIV => {
			if input[1].clone() == num::BigInt::zero() {
				num::BigInt::zero()
			} else {
				input[0].clone() / input[1].clone()
			}
		}
		_ => panic!("Don't have the logic enabled for {}", OPCODES[opcode as usize].name)
	}
}
/// Virtual machine bytecode instruction.
/// halts execution
pub const STOP: Instruction = 0x00;
/// addition operation
pub const ADD: Instruction = 0x01;
/// mulitplication operation
pub const MUL: Instruction = 0x02;
/// subtraction operation
pub const SUB: Instruction = 0x03;
/// integer division operation
pub const DIV: Instruction = 0x04;
/// signed integer division operation
pub const SDIV: Instruction = 0x05;
/// modulo remainder operation
pub const MOD: Instruction = 0x06;
/// signed modulo remainder operation
pub const SMOD: Instruction = 0x07;
/// unsigned modular addition
pub const ADDMOD: Instruction = 0x08;
/// unsigned modular multiplication
pub const MULMOD: Instruction = 0x09;
/// exponential operation
pub const EXP: Instruction = 0x0a;
/// extend length of signed integer
pub const SIGNEXTEND: Instruction = 0x0b;

/// less-than comparision
pub const LT: Instruction = 0x10;
/// greater-than comparision
pub const GT: Instruction = 0x11;
/// signed less-than comparision
pub const SLT: Instruction = 0x12;
/// signed greater-than comparision
pub const SGT: Instruction = 0x13;
/// equality comparision
pub const EQ: Instruction = 0x14;
/// simple not operator
pub const ISZERO: Instruction = 0x15;
/// bitwise AND operation
pub const AND: Instruction = 0x16;
/// bitwise OR operation
pub const OR: Instruction = 0x17;
/// bitwise XOR operation
pub const XOR: Instruction = 0x18;
/// bitwise NOT opertation
pub const NOT: Instruction = 0x19;
/// retrieve single byte from word
pub const BYTE: Instruction = 0x1a;

/// compute SHA3-256 hash
pub const SHA3: Instruction = 0x20;

/// get address of currently executing account
pub const ADDRESS: Instruction = 0x30;
/// get balance of the given account
pub const BALANCE: Instruction = 0x31;
/// get execution origination address
pub const ORIGIN: Instruction = 0x32;
/// get caller address
pub const CALLER: Instruction = 0x33;
/// get deposited value by the instruction/transaction responsible for this execution
pub const CALLVALUE: Instruction = 0x34;
/// get input data of current environment
pub const CALLDATALOAD: Instruction = 0x35;
/// get size of input data in current environment
pub const CALLDATASIZE: Instruction = 0x36;
/// copy input data in current environment to memory
pub const CALLDATACOPY: Instruction = 0x37;
/// get size of code running in current environment
pub const CODESIZE: Instruction = 0x38;
/// copy code running in current environment to memory
pub const CODECOPY: Instruction = 0x39;
/// get price of gas in current environment
pub const GASPRICE: Instruction = 0x3a;
/// get external code size (from another contract)
pub const EXTCODESIZE: Instruction = 0x3b;
/// copy external code (from another contract)
pub const EXTCODECOPY: Instruction = 0x3c;
/// get the size of the return data buffer for the last call
pub const RETURNDATASIZE: Instruction = 0x3d;
/// copy return data buffer to memory
pub const RETURNDATACOPY: Instruction = 0x3e;

/// get hash of most recent complete block
pub const BLOCKHASH: Instruction = 0x40;
/// get the block's coinbase address
pub const COINBASE: Instruction = 0x41;
/// get the block's timestamp
pub const TIMESTAMP: Instruction = 0x42;
/// get the block's number
pub const NUMBER: Instruction = 0x43;
/// get the block's difficulty
pub const DIFFICULTY: Instruction = 0x44;
/// get the block's gas limit
pub const GASLIMIT: Instruction = 0x45;

/// remove item from stack
pub const POP: Instruction = 0x50;
/// load word from memory
pub const MLOAD: Instruction = 0x51;
/// save word to memory
pub const MSTORE: Instruction = 0x52;
/// save byte to memory
pub const MSTORE8: Instruction = 0x53;
/// load word from storage
pub const SLOAD: Instruction = 0x54;
/// save word to storage
pub const SSTORE: Instruction = 0x55;
/// alter the program counter
pub const JUMP: Instruction = 0x56;
/// conditionally alter the program counter
pub const JUMPI: Instruction = 0x57;
/// get the program counter
pub const PC: Instruction = 0x58;
/// get the size of active memory
pub const MSIZE: Instruction = 0x59;
/// get the amount of available gas
pub const GAS: Instruction = 0x5a;
/// set a potential jump destination
pub const JUMPDEST: Instruction = 0x5b;

/// place 1 byte item on stack
pub const PUSH1: Instruction = 0x60;
/// place 2 byte item on stack
pub const PUSH2: Instruction = 0x61;
/// place 3 byte item on stack
pub const PUSH3: Instruction = 0x62;
/// place 4 byte item on stack
pub const PUSH4: Instruction = 0x63;
/// place 5 byte item on stack
pub const PUSH5: Instruction = 0x64;
/// place 6 byte item on stack
pub const PUSH6: Instruction = 0x65;
/// place 7 byte item on stack
pub const PUSH7: Instruction = 0x66;
/// place 8 byte item on stack
pub const PUSH8: Instruction = 0x67;
/// place 9 byte item on stack
pub const PUSH9: Instruction = 0x68;
/// place 10 byte item on stack
pub const PUSH10: Instruction = 0x69;
/// place 11 byte item on stack
pub const PUSH11: Instruction = 0x6a;
/// place 12 byte item on stack
pub const PUSH12: Instruction = 0x6b;
/// place 13 byte item on stack
pub const PUSH13: Instruction = 0x6c;
/// place 14 byte item on stack
pub const PUSH14: Instruction = 0x6d;
/// place 15 byte item on stack
pub const PUSH15: Instruction = 0x6e;
/// place 16 byte item on stack
pub const PUSH16: Instruction = 0x6f;
/// place 17 byte item on stack
pub const PUSH17: Instruction = 0x70;
/// place 18 byte item on stack
pub const PUSH18: Instruction = 0x71;
/// place 19 byte item on stack
pub const PUSH19: Instruction = 0x72;
/// place 20 byte item on stack
pub const PUSH20: Instruction = 0x73;
/// place 21 byte item on stack
pub const PUSH21: Instruction = 0x74;
/// place 22 byte item on stack
pub const PUSH22: Instruction = 0x75;
/// place 23 byte item on stack
pub const PUSH23: Instruction = 0x76;
/// place 24 byte item on stack
pub const PUSH24: Instruction = 0x77;
/// place 25 byte item on stack
pub const PUSH25: Instruction = 0x78;
/// place 26 byte item on stack
pub const PUSH26: Instruction = 0x79;
/// place 27 byte item on stack
pub const PUSH27: Instruction = 0x7a;
/// place 28 byte item on stack
pub const PUSH28: Instruction = 0x7b;
/// place 29 byte item on stack
pub const PUSH29: Instruction = 0x7c;
/// place 30 byte item on stack
pub const PUSH30: Instruction = 0x7d;
/// place 31 byte item on stack
pub const PUSH31: Instruction = 0x7e;
/// place 32 byte item on stack
pub const PUSH32: Instruction = 0x7f;

/// copies the highest item in the stack to the top of the stack
pub const DUP1: Instruction = 0x80;
/// copies the second highest item in the stack to the top of the stack
pub const DUP2: Instruction = 0x81;
/// copies the third highest item in the stack to the top of the stack
pub const DUP3: Instruction = 0x82;
/// copies the 4th highest item in the stack to the top of the stack
pub const DUP4: Instruction = 0x83;
/// copies the 5th highest item in the stack to the top of the stack
pub const DUP5: Instruction = 0x84;
/// copies the 6th highest item in the stack to the top of the stack
pub const DUP6: Instruction = 0x85;
/// copies the 7th highest item in the stack to the top of the stack
pub const DUP7: Instruction = 0x86;
/// copies the 8th highest item in the stack to the top of the stack
pub const DUP8: Instruction = 0x87;
/// copies the 9th highest item in the stack to the top of the stack
pub const DUP9: Instruction = 0x88;
/// copies the 10th highest item in the stack to the top of the stack
pub const DUP10: Instruction = 0x89;
/// copies the 11th highest item in the stack to the top of the stack
pub const DUP11: Instruction = 0x8a;
/// copies the 12th highest item in the stack to the top of the stack
pub const DUP12: Instruction = 0x8b;
/// copies the 13th highest item in the stack to the top of the stack
pub const DUP13: Instruction = 0x8c;
/// copies the 14th highest item in the stack to the top of the stack
pub const DUP14: Instruction = 0x8d;
/// copies the 15th highest item in the stack to the top of the stack
pub const DUP15: Instruction = 0x8e;
/// copies the 16th highest item in the stack to the top of the stack
pub const DUP16: Instruction = 0x8f;

/// swaps the highest and second highest value on the stack
pub const SWAP1: Instruction = 0x90;
/// swaps the highest and third highest value on the stack
pub const SWAP2: Instruction = 0x91;
/// swaps the highest and 4th highest value on the stack
pub const SWAP3: Instruction = 0x92;
/// swaps the highest and 5th highest value on the stack
pub const SWAP4: Instruction = 0x93;
/// swaps the highest and 6th highest value on the stack
pub const SWAP5: Instruction = 0x94;
/// swaps the highest and 7th highest value on the stack
pub const SWAP6: Instruction = 0x95;
/// swaps the highest and 8th highest value on the stack
pub const SWAP7: Instruction = 0x96;
/// swaps the highest and 9th highest value on the stack
pub const SWAP8: Instruction = 0x97;
/// swaps the highest and 10th highest value on the stack
pub const SWAP9: Instruction = 0x98;
/// swaps the highest and 11th highest value on the stack
pub const SWAP10: Instruction = 0x99;
/// swaps the highest and 12th highest value on the stack
pub const SWAP11: Instruction = 0x9a;
/// swaps the highest and 13th highest value on the stack
pub const SWAP12: Instruction = 0x9b;
/// swaps the highest and 14th highest value on the stack
pub const SWAP13: Instruction = 0x9c;
/// swaps the highest and 15th highest value on the stack
pub const SWAP14: Instruction = 0x9d;
/// swaps the highest and 16th highest value on the stack
pub const SWAP15: Instruction = 0x9e;
/// swaps the highest and 17th highest value on the stack
pub const SWAP16: Instruction = 0x9f;

/// Makes a log entry; no topics.
pub const LOG0: Instruction = 0xa0;
/// Makes a log entry; 1 topic.
pub const LOG1: Instruction = 0xa1;
/// Makes a log entry; 2 topics.
pub const LOG2: Instruction = 0xa2;
/// Makes a log entry; 3 topics.
pub const LOG3: Instruction = 0xa3;
/// Makes a log entry; 4 topics.
pub const LOG4: Instruction = 0xa4;
/// Maximal number of topics for log instructions
pub const MAX_NO_OF_TOPICS : usize = 4;

/// create a new account with associated code
pub const CREATE: Instruction = 0xf0;
/// message-call into an account
pub const CALL: Instruction = 0xf1;
/// message-call with another account's code only
pub const CALLCODE: Instruction = 0xf2;
/// halt execution returning output data
pub const RETURN: Instruction = 0xf3;
/// like CALLCODE but keeps caller's value and sender
pub const DELEGATECALL: Instruction = 0xf4;
/// create a new account and set creation address to sha3(sender + sha3(init code)) % 2**160
pub const CREATE2: Instruction = 0xfb;
/// stop execution and revert state changes. Return output data.
pub const REVERT: Instruction = 0xfd;
/// like CALL but it does not take value, nor modify the state
pub const STATICCALL: Instruction = 0xfa;
/// halt execution and register account for later deletion
pub const SUICIDE: Instruction = 0xff;

