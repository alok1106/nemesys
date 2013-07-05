#include <tr1/memory>
#include <stdio.h>

#include <string>
#include <vector>

using namespace std;
using namespace std::tr1;

#include "ast.hh"
#include "ast_visitor.hh"



// helpers for str()/print() methods

static string comma_list(const vector<string>& l) {
  string ret;
  for (int x = 0; x < l.size(); x++) {
    if (x > 0)
      ret += ", ";
    ret += l[x];
  }
  return ret;
}

template<typename T> static string comma_str_list(const vector<shared_ptr<T> >& l) {
  string ret;
  for (int x = 0; x < l.size(); x++) {
    if (x > 0)
      ret += ", ";
    if (!l[x])
      ret += "NULL";
    else
      ret += l[x]->str();
  }
  return ret;
}

template<typename T> static string str_or_null(const shared_ptr<T> item) {
  return item.get() ? item->str() : "NULL";
}

static void print_indent(int level) {
  while (level--)
    printf(" "); // TODO: this should be putc or soemthing
}

const char* augment_operator_names[] = {"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "**=", "//=", NULL};



// UnpackingFormat

// UnpackingTuple

string UnpackingTuple::str() const {
  return "(" + comma_str_list(objects) + ")";
}

void UnpackingTuple::accept(ASTVisitor* v) {
  v->visit(this);
}

// UnpackingVariable

string UnpackingVariable::str() const {
  return name;
}

UnpackingVariable::UnpackingVariable(string _name) : name(_name) {}

void UnpackingVariable::accept(ASTVisitor* v) {
  v->visit(this);
}

// Expression

bool Expression::valid_lvalue() const {
  return false;
}

// ArgumentDefinition

string ArgumentDefinition::str() const {
  if (mode == DefaultArgMode) {
    if (name.length() > 0)
      return name + (!default_value ? "" : "=" + default_value->str());
    else
      return str_or_null(default_value);
  }
  if (mode == ArgListMode) {
    if (!default_value)
      return "*" + name;
    else
      return "*" + str_or_null(default_value);
  }
  if (mode == KeywordArgListMode) {
    if (!default_value)
      return "**" + name;
    else
      return "**" + str_or_null(default_value);
  }
  return "(BAD ARGUMENT DEFINITION)";
}

ArgumentDefinition::ArgumentDefinition(string _name, shared_ptr<Expression> _default_value, ArgumentMode _mode) :
    name(_name), default_value(_default_value), mode(_mode) {}

void ArgumentDefinition::accept(ASTVisitor* v) {
  v->visit(this);
}

// UnaryOperation

const char* unary_operator_names[] = {
  "not ",
  "~",
  "+",
  "-",
  "$REPR$",
  "yield ",
  "$INVALID$",
};

string UnaryOperation::str() const {
  // dammit c++; I shouldn't have to string() this
  return string("(") + unary_operator_names[oper] + expr->str() + ")";
}

UnaryOperation::UnaryOperation(UnaryOperator _oper, shared_ptr<Expression> _expr) :
    oper(_oper), expr(_expr) { }
UnaryOperation::UnaryOperation() { }

void UnaryOperation::accept(ASTVisitor* v) {
  v->visit(this);
}

// BinaryOperation

const char* binary_operator_names[] = {
  "or",
  "and",
  "<",
  ">",
  "==",
  ">=",
  "<=",
  "!=",
  "in",
  "not in",
  "is",
  "is not",
  "|",
  "&",
  "^",
  "<<",
  ">>",
  "+",
  "-",
  "*",
  "/",
  "%",
  "//",
  "**",
  "$INVALID$",
};

string BinaryOperation::str() const {
  return "(" + left->str() + " " + binary_operator_names[oper] + " " + right->str() + ")";
}

BinaryOperation::BinaryOperation(BinaryOperator _oper, shared_ptr<Expression> _left, shared_ptr<Expression> _right) :
    oper(_oper), left(_left), right(_right) { }
BinaryOperation::BinaryOperation() { }

void BinaryOperation::accept(ASTVisitor* v) {
  v->visit(this);
}

// TernaryOperation

const char* ternary_operator_first_names[] = {
  "if",
  "$INVALID$",
};

const char* ternary_operator_second_names[] = {
  "else",
  "$INVALID$",
};

string TernaryOperation::str() const {
  return "(" + left->str() + " " + ternary_operator_first_names[oper] + " " + center->str() + " " + ternary_operator_second_names[oper] + " " + right->str() + ")";
}

TernaryOperation::TernaryOperation(TernaryOperator _oper, shared_ptr<Expression> _left, shared_ptr<Expression> _center, shared_ptr<Expression> _right) :
    oper(_oper), left(_left), center(_center), right(_right) { }
TernaryOperation::TernaryOperation() { }

void TernaryOperation::accept(ASTVisitor* v) {
  v->visit(this);
}

// ListConstructor

string ListConstructor::str() const {
  return "[" + comma_str_list(items) + "]";
}

void ListConstructor::accept(ASTVisitor* v) {
  v->visit(this);
}

// DictConstructor

string DictConstructor::str() const {
  string ret;
  for (int x = 0; x < items.size(); x++) {
    if (x > 0)
      ret += ", ";
    ret += str_or_null(items[x].first);
    ret += ": ";
    ret += str_or_null(items[x].second);
  }
  return "{" + ret + "}";
}

void DictConstructor::accept(ASTVisitor* v) {
  v->visit(this);
}

// SetConstructor

string SetConstructor::str() const {
  return "set(" + comma_str_list(items) + ")";
}

void SetConstructor::accept(ASTVisitor* v) {
  v->visit(this);
}

// TupleConstructor

string TupleConstructor::str() const {
  return "(" + comma_str_list(items) + ")";
}

bool TupleConstructor::valid_lvalue() const {
  // a TupleConstructor is a valid lvalue if all of its items are valid lvalues and it has at least one item
  for (int x = 0; x < items.size(); x++)
    if (!items[x]->valid_lvalue())
      return false;
  return (items.size() > 0);
}

void TupleConstructor::accept(ASTVisitor* v) {
  v->visit(this);
}

// ListComprehension

string ListComprehension::str() const {
  return "[" + str_or_null(item_pattern) + " for " + str_or_null(variables) + " in " + str_or_null(source_data) + (!if_expr ? "" : (" if " + if_expr->str())) + "]";
}

void ListComprehension::accept(ASTVisitor* v) {
  v->visit(this);
}

// DictComprehension

string DictComprehension::str() const {
  return "{" + str_or_null(key_pattern) + ": " + str_or_null(value_pattern) + " for " + str_or_null(variables) + " in " + str_or_null(source_data) + (!if_expr ? "" : (" if " + if_expr->str())) + "}";
}

DictComprehension::DictComprehension(shared_ptr<Expression> _key_pattern, shared_ptr<Expression> _value_pattern, shared_ptr<UnpackingFormat> _variables, shared_ptr<Expression> _source_data, shared_ptr<Expression> _if_expr)
  : key_pattern(_key_pattern), value_pattern(_value_pattern), variables(_variables), source_data(_source_data), if_expr(_if_expr) {}

void DictComprehension::accept(ASTVisitor* v) {
  v->visit(this);
}

// SetComprehension

string SetComprehension::str() const {
  return "{" + str_or_null(item_pattern) + " for " + str_or_null(variables) + " in " + str_or_null(source_data) + (!if_expr ? "" : (" if " + if_expr->str())) + "}";
}

SetComprehension::SetComprehension(shared_ptr<Expression> _item_pattern, shared_ptr<UnpackingFormat> _variables, shared_ptr<Expression> _source_data, shared_ptr<Expression> _if_expr)
  : item_pattern(_item_pattern), variables(_variables), source_data(_source_data), if_expr(_if_expr) {}

void SetComprehension::accept(ASTVisitor* v) {
  v->visit(this);
}

// LambdaDefinition

string LambdaDefinition::str() const {
  return "lambda " + comma_str_list(args) + ": " + str_or_null(result);
}

LambdaDefinition::LambdaDefinition(const vector<shared_ptr<ArgumentDefinition> >& _args, shared_ptr<Expression> _result) : args(_args), result(_result) {}
LambdaDefinition::LambdaDefinition() {}

void LambdaDefinition::accept(ASTVisitor* v) {
  v->visit(this);
}

// FunctionCall

string FunctionCall::str() const {
  return function->str() + "(" + comma_str_list(args) + ")";
}

void FunctionCall::accept(ASTVisitor* v) {
  v->visit(this);
}

// ArrayIndex

string ArrayIndex::str() const {
  return array->str() + "[" + index->str() + "]";
}

bool ArrayIndex::valid_lvalue() const {
  return true;
}

void ArrayIndex::accept(ASTVisitor* v) {
  v->visit(this);
}

// ArraySlice

string ArraySlice::str() const {
  return array->str() + "[" + str_or_null(slice_left) + ":" + str_or_null(slice_right) + "]";
}

void ArraySlice::accept(ASTVisitor* v) {
  v->visit(this);
}

// IntegerConstant

string IntegerConstant::str() const {
  // this is why standard c++ strings blow
  char buffer[0x40];
  sprintf(buffer, "%ld", value);
  return string(buffer);
}

IntegerConstant::IntegerConstant(long _value) : value(_value) {}

void IntegerConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// FloatingConstant

string FloatingConstant::str() const {
  char buffer[0x40];
  sprintf(buffer, "%g", value);
  return string(buffer);
}

FloatingConstant::FloatingConstant(float _value) : value(_value) {}

void FloatingConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// StringConstant

string StringConstant::str() const {
  return "\'" + value + "\'";
}

StringConstant::StringConstant(string _value) : value(_value) {}

void StringConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// TrueConstant

string TrueConstant::str() const {
  return "True";
}

void TrueConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// FalseConstant

string FalseConstant::str() const {
  return "False";
}

void FalseConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// NoneConstant

string NoneConstant::str() const {
  return "None";
}

void NoneConstant::accept(ASTVisitor* v) {
  v->visit(this);
}

// VariableLookup

string VariableLookup::str() const {
  return name;
}

bool VariableLookup::valid_lvalue() const {
  return true;
}

VariableLookup::VariableLookup(string _name) : name(_name) {}

void VariableLookup::accept(ASTVisitor* v) {
  v->visit(this);
}

// AttributeLookup

string AttributeLookup::str() const {
  return left->str() + "." + right->str();
}

bool AttributeLookup::valid_lvalue() const {
  return true;
}

AttributeLookup::AttributeLookup(shared_ptr<Expression> _left, shared_ptr<Expression> _right) : left(_left), right(_right) {}
AttributeLookup::AttributeLookup() {}

void AttributeLookup::accept(ASTVisitor* v) {
  v->visit(this);
}



// statement functions



// Statement

// SimpleStatement

void SimpleStatement::print(int indent_level) const {
  print_indent(indent_level);
  printf("%s\n", str().c_str());
}

// CompoundStatement

void CompoundStatement::print(int indent_level) const {
  print_indent(indent_level);
  printf("%s\n", str().c_str());
  for (int x = 0; x < suite.size(); x++) {
    if (!suite[x])
      printf("# NULL STATEMENT\n");
    else
      suite[x]->print(indent_level + AST_PRINT_INDENT_STEP);
  }
}

// ModuleStatement

string ModuleStatement::str() const {
  return "# ModuleStatement";
}

void ModuleStatement::print(int indent_level) const {
  // basically the same as the base class method except it doesn't indent
  print_indent(indent_level);
  printf("%s\n", str().c_str());
  for (int x = 0; x < suite.size(); x++) {
    if (!suite[x])
      printf("# NULL STATEMENT\n");
    else
      suite[x]->print(indent_level);
  }
}

void ModuleStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ExpressionStatement

string ExpressionStatement::str() const {
  return str_or_null(expr);
}

ExpressionStatement::ExpressionStatement(shared_ptr<Expression> _expr) : expr(_expr) {}

void ExpressionStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// AssignmentStatement

string AssignmentStatement::str() const {
  return comma_str_list(left) + " = " + comma_str_list(right);
}

void AssignmentStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// AugmentStatement

string AugmentStatement::str() const {
  return comma_str_list(left) + " " + augment_operator_names[oper] + " " + comma_str_list(right);
}

void AugmentStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// PrintStatement

string PrintStatement::str() const {
  if (!stream)
    return "print " + comma_str_list(items) + (suppress_newline ? "," : "");
  return "print >> " + str_or_null(stream) + ", " + comma_str_list(items) + (suppress_newline ? "," : "");
}

void PrintStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// DeleteStatement

string DeleteStatement::str() const {
  return "del " + comma_str_list(items);
}

void DeleteStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// PassStatement

string PassStatement::str() const {
  return "pass";
}

void PassStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// FlowStatement

// ImportStatement

string ImportStatement::str() const {
  if (import_star)
    return "from " + comma_list(module_names) + " import *";
  if (module_renames.size())
    return "import " + comma_list(module_names) + " as " + comma_list(module_renames);
  if (symbol_renames.size())
    return "from " + comma_list(module_names) + " import " + comma_list(symbol_list) + " as " + comma_list(symbol_renames);
  if (symbol_list.size())
    return "from " + comma_list(module_names) + " import " + comma_list(symbol_list);
  return "import " + comma_list(module_names);
}

void ImportStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// GlobalStatement

string GlobalStatement::str() const {
  return "global " + comma_list(names);
}

void GlobalStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ExecStatement

string ExecStatement::str() const {
  return "exec " + str_or_null(code) + ", " + str_or_null(globals) + ", " + str_or_null(locals);
}

void ExecStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// AssertStatement

string AssertStatement::str() const {
  return "assert " + str_or_null(check) + ", " + str_or_null(failure_message);
}

void AssertStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// BreakStatement

string BreakStatement::str() const {
  return "break";
}

void BreakStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ContinueStatement

string ContinueStatement::str() const {
  return "continue";
}

void ContinueStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ReturnStatement

string ReturnStatement::str() const {
  return "return " + comma_str_list(items);
}

void ReturnStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// RaiseStatement

string RaiseStatement::str() const {
  return "raise " + str_or_null(type) + ", " + str_or_null(value) + ", " + str_or_null(traceback);
}

void RaiseStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// YieldStatement

string YieldStatement::str() const {
  return "yield " + str_or_null(expr);
}

void YieldStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// SingleIfStatement

string SingleIfStatement::str() const {
  return "if " + str_or_null(check) + ":";
}

void SingleIfStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ElseStatement

string ElseStatement::str() const {
  return "else:";
};

void ElseStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// IfStatement

string IfStatement::str() const {
  return "if " + str_or_null(check) + ":";
}

void IfStatement::print(int indent_level) const {
  CompoundStatement::print(indent_level);
  for (int x = 0; x < elifs.size(); x++) {
    if (!elifs[x])
      printf("# NULL STATEMENT\n");
    else
      elifs[x]->print(indent_level);
  }
  if (else_suite)
    else_suite->print(indent_level);
}

void IfStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ElifStatement

string ElifStatement::str() const {
  return "elif " + str_or_null(check) + ":";
}

void ElifStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ForStatement

string ForStatement::str() const {
  return "for " + str_or_null(variables) + " in " + comma_str_list(in_exprs) + ":";
}

void ForStatement::print(int indent_level) const {
  CompoundStatement::print(indent_level);
  if (else_suite)
    else_suite->print(indent_level);
}

void ForStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// WhileStatement

string WhileStatement::str() const {
  return "while " + str_or_null(condition) + ":";
}

void WhileStatement::print(int indent_level) const {
  CompoundStatement::print(indent_level);
  if (else_suite)
    else_suite->print(indent_level);
}

void WhileStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// ExceptStatement

string ExceptStatement::str() const {
  if (!name.length())
    return "except " + str_or_null(types) + ":";
  else
    return "except " + str_or_null(types) + " as " + name + ":";
}

void ExceptStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// FinallyStatement

string FinallyStatement::str() const {
  return "finally:";
};

void FinallyStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// TryStatement

string TryStatement::str() const {
  return "try:";
}

void TryStatement::print(int indent_level) const {
  CompoundStatement::print(indent_level);
  for (int x = 0; x < excepts.size(); x++) {
    if (!excepts[x])
      printf("# NULL STATEMENT\n");
    else
      excepts[x]->print(indent_level);
  }
  if (else_suite)
    else_suite->print(indent_level);
  if (finally_suite)
    finally_suite->print(indent_level);
}

void TryStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// WithStatement

string WithStatement::str() const {

  string ret = "with ";
  for (int x = 0; x < items.size(); x++) {
    if (x > 0)
      ret += ", ";
    ret += str_or_null(items[x]);
    if (names[x].length() > 0) {
      ret += " as ";
      ret += names[x];
    }
  }
  return ret + ":";
}

void WithStatement::accept(ASTVisitor* v) {
  v->visit(this);
}

// FunctionDefinition

string FunctionDefinition::str() const {
  return "def " + name + "(" + comma_str_list(args) + "):";
}

void FunctionDefinition::print(int indent_level) const {
  for (int x = 0; x < decorators.size(); x++) {
    print_indent(indent_level);
    if (!decorators[x])
      printf("# NULL DECORATOR\n");
    else
      printf("@%s\n", decorators[x]->str().c_str());
  }
  CompoundStatement::print(indent_level);
}

void FunctionDefinition::accept(ASTVisitor* v) {
  v->visit(this);
}

// ClassDefinition

string ClassDefinition::str() const {
  if (parent_types.size() == 0)
    return "class " + name + ":";
  else
    return "class " + name + "(" + comma_str_list(parent_types) + "):";
}

void ClassDefinition::print(int indent_level) const {
  for (int x = 0; x < decorators.size(); x++) {
    print_indent(indent_level);
    if (!decorators[x])
      printf("# NULL DECORATOR\n");
    else
      printf("@%s\n", decorators[x]->str().c_str());
  }
  CompoundStatement::print(indent_level);
}

void ClassDefinition::accept(ASTVisitor* v) {
  v->visit(this);
}
