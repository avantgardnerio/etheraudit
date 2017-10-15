package Parser

import "math/big"

type ExpressionType int
const (
	CONSTANT ExpressionType = 1
	INSTR_OUTPUT
	ARGUMENT
)

type Expression interface {
	Constant() (big.Int, error)
	Type() ExpressionType
}
