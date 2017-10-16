extern crate num;

use std::collections::hash_map::HashMap;
use instruction::*;
use std;
use self::num::bigint::BigInt;
use expression::num::Zero;
use op_codes;
use program;

pub enum Expression {
    Constant(BigInt),
    Argument(usize),
    ResultOf(usize)
}

#[derive(Clone, Debug)]
pub enum OpTree {
    Operation(u8, std::vec::Vec<OpTree>),
    Constant(BigInt),
    Argument(usize),
    Query(String)
}

impl std::fmt::Display for OpTree {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match *self {
            OpTree::Constant(ref v) => write!(f, "{}", v),
            OpTree::Argument(idx) => write!(f, "<argument.{}>", idx),
            OpTree::Operation(op_code, ref inputs) => {
                if let Some(infix) = op_codes::get_infix(op_code) {
                    match inputs.len() {
                        1 => write!(f, "({}{})", infix, inputs[0]),
                        2 => write!(f, "({} {} {})", inputs[0], infix, inputs[1]),
                        _ => panic!("Invalid infix")
                    }
                } else {
                    let op_code = op_codes::OPCODES[op_code as usize];
                    let ins: std::vec::Vec<String> = inputs.iter().
                        map(|x| x.to_string()).collect();
                    write!(f, "{}({})", op_code.name, ins.join(", "))
                }
            },
            OpTree::Query(ref name) => write!(f, "<?{}>", name),
        }
    }
}

pub type QueryResult = HashMap<String, OpTree>;
type InternalQueryResult = std::vec::Vec<(String, OpTree)>;
impl OpTree {
    fn in_query(self: &OpTree, other: &OpTree) -> Option<InternalQueryResult>{
        match (self, other) {
            (_, &OpTree::Query(ref n)) => {
                Some( [(n.clone(), self.clone())].iter().cloned().collect() )
            },
            (&OpTree::Query(ref n), _) => {
                panic!("Query should be on the rhs")
            },
            (&OpTree::Operation(self_op, ref self_in),
             &OpTree::Operation(other_op, ref other_in)) => {
                if self_op == other_op && self_in.len() == other_in.len() {
                    let a : Option< std::vec::Vec<InternalQueryResult> > = self_in.iter().
                        zip(other_in.iter()).
                        map(OpTree::in_query_tuple).collect();

                        a.map(|v| v.concat())
                } else {
                    None
                }
            },
            (&OpTree::Constant(ref self_v), &OpTree::Constant(ref other_v)) => {
                if self_v == other_v { Some(vec![]) } else { None }
            },
            (&OpTree::Argument(self_v), &OpTree::Argument(other_v)) => {
                if self_v == other_v { Some(vec![]) } else { None }
            },
            _ => None
        }
    }

    fn in_query_tuple(t: (&OpTree, &OpTree)) -> Option<InternalQueryResult>{
        OpTree::in_query(t.0, t.1)
    }

    pub fn query(self: &OpTree, other: &OpTree) -> Option<QueryResult>{
        OpTree::in_query(self, other).and_then(|ans| Some(ans.iter().cloned().collect()))
    }

    pub fn create_from_instr(p: &program::Program, instr: &Instruction) -> OpTree {
        OpTree::Operation(
            instr.op_code,
            instr.inputs.iter()
                .map( |e| OpTree::create(p, e) ).collect()
        )
    }
    pub fn create(p: &program::Program, expr: &Expression) -> OpTree {
        match *expr {
            Expression::Constant(ref v) => OpTree::Constant(v.clone()),
            Expression::Argument(idx) => OpTree::Argument(idx),
            Expression::ResultOf(offset) => OpTree::create_from_instr(p, &p.instructions[&offset])
        }
    }
}

pub fn make_vec(val: &BigInt) -> std::vec::Vec<u8> {
    val.to_signed_bytes_be()
}
pub fn make_constant(data: &[u8]) -> Expression {
    let mut c = BigInt::default();

    for v in data {
        c = c << 8;
        c = c + v;
    }

    Expression::Constant(c)
}

impl std::fmt::Debug for Expression {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match *self {
            Expression::Constant(ref v) => write!(f, "{:?}", make_vec(&v)),
            Expression::Argument(idx) => write!(f, "<argument.{}>", idx),
            Expression::ResultOf(offset) => write!(f, "<#{}>", offset)
        }
    }
}