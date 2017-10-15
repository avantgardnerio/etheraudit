#ifndef XX
#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH)
#endif

XX(STOP, 0x00, 0, 0, 0)		///< halts execution
XX(ADD, 0x01, 2, 1, 0)		///< addition operation
XX(MUL, 0x02, 2, 1, 0)		///< mulitplication operation
XX(SUB, 0x03, 2, 1, 0)		///< subtraction operation
XX(DIV, 0x04, 2, 1, 0)		///< integer division operation
XX(SDIV, 0x05, 2, 1, 0)		///< signed integer division operation
XX(MOD, 0x06, 2, 1, 0)		///< modulo remainder operation
XX(SMOD, 0x07, 2, 1, 0)		///< signed modulo remainder operation
XX(ADDMOD, 0x08, 3, 1, 0)		///< unsigned modular addition
XX(MULMOD, 0x09, 3, 1, 0)		///< unsigned modular multiplication
XX(EXP, 0x0A, 2, 1, 0)		///< exponential operation
XX(SIGNEXTEND, 0x0B, 2, 1, 0)		///< extend length of signed integer

XX(LT, 0x10, 2, 1, 0)		///< less-than comparision
XX(GT, 0x11, 2, 1, 0)		///< greater-than comparision
XX(SLT, 0x12, 2, 1, 0)		///< signed less-than comparision
XX(SGT, 0x13, 2, 1, 0)		///< signed greater-than comparision
XX(EQ, 0x14, 2, 1, 0)		///< equality comparision
XX(ISZERO, 0x15, 1, 1, 0)		///< simple not operator
XX(AND, 0x16, 2, 1, 0)		///< bitwise AND operation
XX(OR, 0x17, 2, 1, 0)		///< bitwise OR operation
XX(XOR, 0x18, 2, 1, 0)		///< bitwise XOR operation
XX(NOT, 0x19, 1, 1, 0)		///< bitwise NOT opertation
XX(BYTE, 0x1A, 2, 1, 0)		///< retrieve single byte from word

XX(SHA3, 0x20, 2, 1, 0)		///< compute SHA3-256 hash

XX(ADDRESS, 0x30, 0, 1, 0)		///< get address of currently executing account
XX(BALANCE, 0x31, 1, 1, 0)		///< get balance of the given account
XX(ORIGIN, 0x32, 0, 1, 0)		///< get execution origination address
XX(CALLER, 0x33, 0, 1, 0)		///< get caller address
XX(CALLVALUE, 0x34, 0, 1, 0)		///< get deposited value by the instruction/transaction responsible for this execution
XX(CALLDATALOAD, 0x35, 1, 1, 0)		///< get input data of current environment
XX(CALLDATASIZE, 0x36, 0, 1, 0)		///< get size of input data in current environment
XX(CALLDATACOPY, 0x37, 3, 0, 0)		///< copy input data in current environment to memory
XX(CODESIZE, 0x38, 0, 1, 0)		///< get size of code running in current environment
XX(CODECOPY, 0x39, 3, 0, 0)		///< copy code running in current environment to memory
XX(GASPRICE, 0x3A, 0, 1, 0)		///< get price of gas in current environment
XX(EXTCODESIZE, 0x3B, 1, 1, 0)		///< get external code size (from another contract)
XX(EXTCODECOPY, 0x3C, 4, 0, 0)		///< copy external code (from another contract)
//XX(RETURNDATASIZE, 0x3d, 0, 0, 0)		///< size of data returned from previous call
//XX(RETURNDATACOPY, 0x3e, 0, 0, 0)		///< copy data returned from previous call to memory

XX(BLOCKHASH, 0x40, 1, 1, 0)		///< get hash of most recent complete block
XX(COINBASE, 0x41, 0, 1, 0)		///< get the block's coinbase address
XX(TIMESTAMP, 0x42, 0, 1, 0)		///< get the block's timestamp
XX(NUMBER, 0x43, 0, 1, 0)		///< get the block's number
XX(DIFFICULTY, 0x44, 0, 1, 0)		///< get the block's difficulty
XX(GASLIMIT, 0x45, 0, 1, 0)		///< get the block's gas limit

XX(POP, 0x50, 1, 0, 0)		///< remove item from stack
XX(MLOAD, 0x51, 1, 1, 0)		///< load word from memory
XX(MSTORE, 0x52, 2, 0, 0)		///< save word to memory
XX(MSTORE8, 0x53, 2, 0, 0)		///< save byte to memory
XX(SLOAD, 0x54, 1, 1, 0)		///< load word from storage
XX(SSTORE, 0x55, 2, 0, 0)		///< save word to storage
XX(JUMP, 0x56, 1, 0, 0)		///< alter the program counter to a jumpdest
XX(JUMPI, 0x57, 2, 0, 0)		///< conditionally alter the program counter
XX(PC, 0x58, 0, 1, 0)		///< get the program counter
XX(MSIZE, 0x59, 0, 1, 0)		///< get the size of active memory
XX(GAS, 0x5A, 0, 1, 0)		///< get the amount of available gas
XX(JUMPDEST, 0x5B, 0, 0, 0)		///< set a potential jump destination

XX(PUSH1, 0x60, 0, 1, 1)		///< place 1 byte item on stack
XX(PUSH2, 0x61, 0, 1, 2)		///< place 2 byte item on stack
XX(PUSH3, 0x62, 0, 1, 3)		///< place 3 byte item on stack
XX(PUSH4, 0x63, 0, 1, 4)		///< place 4 byte item on stack
XX(PUSH5, 0x64, 0, 1, 5)		///< place 5 byte item on stack
XX(PUSH6, 0x65, 0, 1, 6)		///< place 6 byte item on stack
XX(PUSH7, 0x66, 0, 1, 7)		///< place 7 byte item on stack
XX(PUSH8, 0x67, 0, 1, 8)		///< place 8 byte item on stack
XX(PUSH9, 0x68, 0, 1, 9)		///< place 9 byte item on stack
XX(PUSH10, 0x69, 0, 1, 10)		///< place 10 byte item on stack
XX(PUSH11, 0x6A, 0, 1, 11)		///< place 11 byte item on stack
XX(PUSH12, 0x6B, 0, 1, 12)		///< place 12 byte item on stack
XX(PUSH13, 0x6C, 0, 1, 13)		///< place 13 byte item on stack
XX(PUSH14, 0x6D, 0, 1, 14)		///< place 14 byte item on stack
XX(PUSH15, 0x6E, 0, 1, 15)		///< place 15 byte item on stack
XX(PUSH16, 0x6F, 0, 1, 16)		///< place 16 byte item on stack
XX(PUSH17, 0x70, 0, 1, 17)		///< place 17 byte item on stack
XX(PUSH18, 0x71, 0, 1, 18)		///< place 18 byte item on stack
XX(PUSH19, 0x72, 0, 1, 19)		///< place 19 byte item on stack
XX(PUSH20, 0x73, 0, 1, 20)		///< place 20 byte item on stack
XX(PUSH21, 0x74, 0, 1, 21)		///< place 21 byte item on stack
XX(PUSH22, 0x75, 0, 1, 22)		///< place 22 byte item on stack
XX(PUSH23, 0x76, 0, 1, 23)		///< place 23 byte item on stack
XX(PUSH24, 0x77, 0, 1, 24)		///< place 24 byte item on stack
XX(PUSH25, 0x78, 0, 1, 25)		///< place 25 byte item on stack
XX(PUSH26, 0x79, 0, 1, 26)		///< place 26 byte item on stack
XX(PUSH27, 0x7A, 0, 1, 27)		///< place 27 byte item on stack
XX(PUSH28, 0x7B, 0, 1, 28)		///< place 28 byte item on stack
XX(PUSH29, 0x7C, 0, 1, 29)		///< place 29 byte item on stack
XX(PUSH30, 0x7D, 0, 1, 30)		///< place 30 byte item on stack
XX(PUSH31, 0x7E, 0, 1, 31)		///< place 31 byte item on stack
XX(PUSH32, 0x7F, 0, 1, 32)		///< place 32 byte item on stack

XX(DUP1, 0x80, 1, 2, 0)		///< copies the highest item in the stack to the top of the stack
XX(DUP2, 0x81, 2, 3, 0)		///< copies the second highest item in the stack to the top of the stack
XX(DUP3, 0x82, 3, 4, 0)		///< copies the third highest item in the stack to the top of the stack
XX(DUP4, 0x83, 4, 5, 0)		///< copies the 4th highest item in the stack to the top of the stack
XX(DUP5, 0x84, 5, 6, 0)		///< copies the 5th highest item in the stack to the top of the stack
XX(DUP6, 0x85, 6, 7, 0)		///< copies the 6th highest item in the stack to the top of the stack
XX(DUP7, 0x86, 7, 8, 0)		///< copies the 7th highest item in the stack to the top of the stack
XX(DUP8, 0x87, 8, 9, 0)		///< copies the 8th highest item in the stack to the top of the stack
XX(DUP9, 0x88, 9, 10, 0)		///< copies the 9th highest item in the stack to the top of the stack
XX(DUP10, 0x89, 10, 11, 0)		///< copies the 10th highest item in the stack to the top of the stack
XX(DUP11, 0x8A, 11, 12, 0)		///< copies the 11th highest item in the stack to the top of the stack
XX(DUP12, 0x8B, 12, 13, 0)		///< copies the 12th highest item in the stack to the top of the stack
XX(DUP13, 0x8C, 13, 14, 0)		///< copies the 13th highest item in the stack to the top of the stack
XX(DUP14, 0x8D, 14, 15, 0)		///< copies the 14th highest item in the stack to the top of the stack
XX(DUP15, 0x8E, 15, 16, 0)		///< copies the 15th highest item in the stack to the top of the stack
XX(DUP16, 0x8F, 16, 17, 0)		///< copies the 16th highest item in the stack to the top of the stack

XX(SWAP1, 0x90, 2, 2, 0)		///< swaps the highest and second highest value on the stack
XX(SWAP2, 0x91, 3, 3, 0)		///< swaps the highest and third highest value on the stack
XX(SWAP3, 0x92, 4, 4, 0)		///< swaps the highest and 4th highest value on the stack
XX(SWAP4, 0x93, 5, 5, 0)		///< swaps the highest and 5th highest value on the stack
XX(SWAP5, 0x94, 6, 6, 0)		///< swaps the highest and 6th highest value on the stack
XX(SWAP6, 0x95, 7, 7, 0)		///< swaps the highest and 7th highest value on the stack
XX(SWAP7, 0x96, 8, 8, 0)		///< swaps the highest and 8th highest value on the stack
XX(SWAP8, 0x97, 9, 9, 0)		///< swaps the highest and 9th highest value on the stack
XX(SWAP9, 0x98, 10, 10, 0)		///< swaps the highest and 10th highest value on the stack
XX(SWAP10, 0x99, 11, 11, 0)		///< swaps the highest and 11th highest value on the stack
XX(SWAP11, 0x9A, 12, 12, 0)		///< swaps the highest and 12th highest value on the stack
XX(SWAP12, 0x9B, 13, 13, 0)		///< swaps the highest and 13th highest value on the stack
XX(SWAP13, 0x9C, 14, 14, 0)		///< swaps the highest and 14th highest value on the stack
XX(SWAP14, 0x9D, 15, 15, 0)		///< swaps the highest and 15th highest value on the stack
XX(SWAP15, 0x9E, 16, 16, 0)		///< swaps the highest and 16th highest value on the stack
XX(SWAP16, 0x9F, 17, 17, 0)		///< swaps the highest and 17th highest value on the stack

XX(LOG0, 0xa0, 2, 0, 0)		///< Makes a log entry; no topics.
XX(LOG1, 0xA1, 3, 0, 0)		///< Makes a log entry; 1 topic.
XX(LOG2, 0xA2, 4, 0, 0)		///< Makes a log entry; 2 topics.
XX(LOG3, 0xA3, 5, 0, 0)		///< Makes a log entry; 3 topics.
XX(LOG4, 0xA4, 6, 0, 0)		///< Makes a log entry; 4 topics.

 // these are generated by the interpreter - should never be in user code
/*
 */
XX(PUSHC, 0xac, 0, 0, 0)		///< push value from constant pool
XX(JUMPC, 0xAD, 0, 0, 0)		///< alter the program counter - pre-verified
XX(JUMPCI, 0xAE, 0, 0, 0)		///< conditionally alter the program counter - pre-verified

XX(JUMPTO, 0xb0, 0, 0, 4)		///< alter the program counter to a jumpdest
XX(JUMPIF, 0xB1, 0, 0, 4)		///< conditionally alter the program counter
XX(JUMPSUB, 0xB2, 0, 0, 4)		///< alter the program counter to a beginsub
XX(JUMPV, 0xB3, 0, 0, 8)		///< alter the program counter to a jumpdest
XX(JUMPSUBV, 0xB4, 0, 0, 0)		///< alter the program counter to a beginsub
XX(BEGINSUB, 0xB5, 0, 0, 0)		///< set a potential jumpsub destination
XX(BEGINDATA, 0xB6, 0, 0, 0)		///< begin the data section
XX(RETURNSUB, 0xB7, 0, 0, 0)		///< return to subroutine jumped from
XX(PUTLOCAL, 0xB8, 0, 1, 0)		///< pop top of stack to local variable
XX(GETLOCAL, 0xB9, 1, 0, 0)		///< push local variable to top of stack


XX(CREATE, 0xf0, 3, 1, 0)		///< create a new account with associated code
XX(CALL, 0xF1, 7, 1, 0)		///< message-call into an account
XX(CALLCODE, 0xF2, 7, 1, 0)		///< message-call with another account's code only
XX(RETURN, 0xF3, 2, 0, 0)		///< halt execution returning output data
XX(DELEGATECALL, 0xF4, 6, 1, 0)		///< like CALLCODE but keeps caller's value and sender
//XX(STATICCALL, 0xfa, 0, 0, 0)		///< like CALL except state changing operation are not permitted (will throw)
XX(REVERT, 0xfd, 0, 0, 0) ///< stop execution and revert state changes, without consuming all provided gas
XX(INVALID, 0xfe, 0, 0, 0)		///< dedicated invalid instruction
XX(SUICIDE, 0xff, 1, 0, 0) ///< halt execution and register account for later deletion

#undef XX