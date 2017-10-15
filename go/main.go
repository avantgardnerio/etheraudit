package main

import (
	"./Parser"
	"io/ioutil"
	"strconv"
	"os"
	"fmt"
)

func main() {
	dat, _ := ioutil.ReadFile(os.Args[1])

	if dat[1] == 'x' {
		dat = dat[2:]
	}

	var bytecode = []byte{}
	for i := 0;i < len(dat);i+=2 {
		var sub = string(dat[i:i+2])
		var newByte, _ = strconv.ParseInt(sub, 16, 16)
		bytecode = append(bytecode, byte(newByte))
	}

	program := Parser.ParseByteCode(bytecode)

	for _, block := range program.Blocks {
		fmt.Println("\nNew Block:", block.Start, block.End)
		for i := block.Start;i < block.End;i++ {
			if instr, ok := program.Instructions[i]; ok {
				fmt.Printf("\t%#d (%#x): %s", instr.Offset, instr.Offset, instr.OpCode.String())
				for _, d := range instr.Data {
					fmt.Printf(" %#x", d)
				}
				fmt.Println()
			}
		}
	}
}
