#include "CompilerException.h"
#include "AbstractSyntaxType.h"
using namespace AbstractSyntax;

void AbstractSyntaxVisitor::VisitAbstractSyntaxType(AbstractSyntaxType& type) {
	throw CompilerError();
}

void AbstractSyntaxVisitor::VisitStatement(Statement& type) {
	VisitAbstractSyntaxType(type);
}

void AbstractSyntaxVisitor::VisitExpression(Expression& type) {
	VisitAbstractSyntaxType(type);
}

void AbstractSyntaxVisitor::VisitSpecialOperation(SpecialOperation& type) {
	VisitAbstractSyntaxType(type);
}

void AbstractSyntaxVisitor::VisitUnaryOperation(UnaryOperation& type) {
	VisitExpression(type);
}

void AbstractSyntaxVisitor::VisitBinaryOperation(BinaryOperation& type) {
	VisitExpression(type);
}

void AbstractSyntaxVisitor::VisitStatementBlock(StatementBlock& type) {
	VisitAbstractSyntaxType(type);
}

#define VirtualVisitStatementBlock(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitStatementBlock(type); } 
VirtualVisitStatementBlock(MainBlock);
VirtualVisitStatementBlock(FunctionBlock);
VirtualVisitStatementBlock(DefaultBlock);
VirtualVisitStatementBlock(WhileBlock);
#undef VirtualVisitStatementBlock

#define VirtualVisitStatement(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitStatement(type); } 
VirtualVisitStatement(StatementDefineFunction);
VirtualVisitStatement(StatementDefineVariable);
VirtualVisitStatement(StatementAssignmentId);
VirtualVisitStatement(StatementAssignmentArray);
VirtualVisitStatement(StatementAssignmentField);
VirtualVisitStatement(StatementCall);
VirtualVisitStatement(StatementIf);
VirtualVisitStatement(StatementWhile);
VirtualVisitStatement(StatementBreak);
VirtualVisitStatement(StatementContinue);
VirtualVisitStatement(StatementReturn);
#undef VirtualVisitSpecialOperation

#define VirtualVisitType(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitExpression(type); } 
VirtualVisitType(Null);
VirtualVisitType(Bool);
VirtualVisitType(Char);
VirtualVisitType(Int);
VirtualVisitType(Float);
VirtualVisitType(String);
VirtualVisitType(Array);
VirtualVisitType(Function);
VirtualVisitType(Object);
VirtualVisitType(SpecialOperationList);
#undef VirtualVisitSpecialOperation

#define VirtualVisitSpecialOperation(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitSpecialOperation(type); } 
VirtualVisitSpecialOperation(FunctionCall);
VirtualVisitSpecialOperation(AccessArray);
VirtualVisitSpecialOperation(AccessField);
#undef VirtualVisitSpecialOperation

#define VirtualVisitUnaryOperation(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitUnaryOperation(type); } 
VirtualVisitUnaryOperation(Not);
#undef VirtualVisitUnaryOperation

#define VirtualVisitBinaryOperation(Type) void AbstractSyntaxVisitor::Visit(Type& type){ VisitBinaryOperation(type); } 
VirtualVisitBinaryOperation(Or);
VirtualVisitBinaryOperation(And);
VirtualVisitBinaryOperation(Equals);
VirtualVisitBinaryOperation(NotEquals);
VirtualVisitBinaryOperation(Less);
VirtualVisitBinaryOperation(LessEquals);
VirtualVisitBinaryOperation(Greater);
VirtualVisitBinaryOperation(GreaterEquals);
VirtualVisitBinaryOperation(Add);
VirtualVisitBinaryOperation(Subtract);
VirtualVisitBinaryOperation(Multiply);
VirtualVisitBinaryOperation(Divide);
VirtualVisitBinaryOperation(Modulus);
#undef VirtualVisitBinaryOperation

#define TypeAccept(Type) void Type::Accept(AbstractSyntaxVisitor& visitor){ visitor.Visit(*this); }
TypeAccept(MainBlock);
TypeAccept(FunctionBlock);
TypeAccept(DefaultBlock);
TypeAccept(WhileBlock);
TypeAccept(StatementDefineFunction);
TypeAccept(StatementDefineVariable);
TypeAccept(StatementAssignmentId);
TypeAccept(StatementAssignmentArray);
TypeAccept(StatementAssignmentField);
TypeAccept(StatementCall);
TypeAccept(StatementIf);
TypeAccept(StatementWhile);
TypeAccept(StatementBreak);
TypeAccept(StatementContinue);
TypeAccept(StatementReturn);
TypeAccept(Null);
TypeAccept(Bool);
TypeAccept(Char);
TypeAccept(Int);
TypeAccept(Float);
TypeAccept(String);
TypeAccept(Array);
TypeAccept(Function);
TypeAccept(Object);
TypeAccept(SpecialOperationList);
TypeAccept(FunctionCall);
TypeAccept(AccessArray);
TypeAccept(AccessField);
TypeAccept(Not);
TypeAccept(Or);
TypeAccept(And);
TypeAccept(Equals);
TypeAccept(NotEquals);
TypeAccept(Less);
TypeAccept(LessEquals);
TypeAccept(Greater);
TypeAccept(GreaterEquals);
TypeAccept(Add);
TypeAccept(Subtract);
TypeAccept(Multiply);
TypeAccept(Divide);
TypeAccept(Modulus);
#undef TypeAccept