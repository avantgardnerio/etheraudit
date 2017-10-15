package Parser

import (
	"go-ethereum/core/vm"
	"math/big"
	"errors"
)

type instruction struct {
	Offset int
	OpCode vm.OpCode
	Data   []byte

	Inputs []Expression
	Outputs []Expression
}

func (i instruction) Constant() (big.Int, error) {
	return big.Int{}, errors.New("Not a constant")
}

func (i instruction) Type() ExpressionType {
	return INSTR_OUTPUT
}

type block struct {
	Start int
	End   int
}

type program struct {
	Instructions map[int]instruction
	Blocks []block
	bytecode []byte
}

func getDataLength(code vm.OpCode) int {
	if code.IsPush() {
		return (int)(code - vm.PUSH1 + 1)
	}
	return 0
}

func ParseByteCode(bytecode []byte) program {
	rtn := program{Instructions: map[int]instruction{}, Blocks: []block{}, bytecode: bytecode}
	rtn.fillInstructions(bytecode)
	rtn.fillBlocks()
	rtn.fillOps()
	return rtn
}
func (p program) fillInstructions(bytecode []byte) {
	for pos := 0; pos < len(bytecode); pos++ {
		opCodeByte := bytecode[pos]
		opCode := vm.OpCode(opCodeByte)
		newInstr := instruction{Offset: pos, OpCode: opCode, Data: []byte{}}

		dataLength := getDataLength(opCode)
		ipos := pos

		for i := 0; i < dataLength; i++ {
			pos++
			if pos < len(bytecode) {
				newInstr.Data = append(newInstr.Data, bytecode[pos])
			}
		}

		p.Instructions[ipos] = newInstr
	}
}

func (p* program) addBlock(b block) {
	if b.End != -1 {
		p.Blocks = append(p.Blocks, b)
	}
}

func (p* program) fillBlocks() {
	currentBlock := block{Start: 0, End: 0}
	for offset := 0; offset < len(p.bytecode);offset++ {
		if instr, ok := p.Instructions[offset]; ok {
			if offset != instr.Offset {
				panic("!!!!")
			}
			if instr.OpCode == vm.JUMPDEST {
				p.addBlock(currentBlock)
				currentBlock = block{Start: instr.Offset, End: -1}
			}

			currentBlock.End = instr.Offset + len(instr.Data) + 1
			if isJump(instr.OpCode) || isStop(instr.OpCode) {
				p.addBlock(currentBlock)
				currentBlock = block{Start: currentBlock.End, End: -1}
			}
		}
	}

	if currentBlock.End != -1 {
		p.Blocks = append(p.Blocks, currentBlock)
	}
}

func isStop(code vm.OpCode) bool {
	return code == vm.STOP || code == vm.REVERT ||
		code == 0xff || code == 0xfe
}
func isJump(code vm.OpCode) bool {
	return code == vm.JUMP || code == vm.JUMPI
}
func (b* block) fillOps() {

}
func (p* program) fillOps() {
	for _, b := range p.Blocks {
		b.fillOps()
	}
}