package Parser

import "math/big"

type ConstantExpr struct {
	value big.Int
}

func (expr ConstantExpr) Constant() (big.Int, error) {
	return expr.value, nil
}

func (ConstantExpr) Type() ExpressionType {
	return CONSTANT
}

func MakeConstantExpr(v big.Int) Expression {
	return ConstantExpr{value: v}
}
