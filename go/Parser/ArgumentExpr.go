package Parser

import "math/big"
import "errors"

type ArgumentExpr struct {
	index int
}

func (expr ArgumentExpr) Constant() (big.Int, error) {
	return big.Int{}, errors.New("Not a constant expression")
}

func (ArgumentExpr) Type() ExpressionType {
	return CONSTANT
}

func MakeArgumentExpr(idx int) Expression {
	return ArgumentExpr{index: idx}
}

