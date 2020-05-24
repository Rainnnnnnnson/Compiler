#include "ParseType.h"
#include "CompilerException.h"
using namespace Parse;
void ParseVisitor::VisitParseType(ParseType& type) {
	throw CompilerError();
}

void ParseVisitor::VisitLAType(LAType& type) {
	throw CompilerError();
}

void ParseVisitor::VisitSAType(SAType& type) {
	throw CompilerError();
}

#define VirtualVisitLAType(Type) void ParseVisitor::Visit(Type& type){ VisitLAType(type); } 
VirtualVisitLAType(Char);
VirtualVisitLAType(Int);
VirtualVisitLAType(Float);
VirtualVisitLAType(String);
VirtualVisitLAType(Id);
VirtualVisitLAType(Blank);
VirtualVisitLAType(TextEnd);
VirtualVisitLAType(Null);
VirtualVisitLAType(False);
VirtualVisitLAType(True);
VirtualVisitLAType(Var);
VirtualVisitLAType(Function);
VirtualVisitLAType(Array);
VirtualVisitLAType(Object);
VirtualVisitLAType(If);
VirtualVisitLAType(Else);
VirtualVisitLAType(While);
VirtualVisitLAType(Break);
VirtualVisitLAType(Continue);
VirtualVisitLAType(Return);
VirtualVisitLAType(Add);
VirtualVisitLAType(Subtract);
VirtualVisitLAType(Multiply);
VirtualVisitLAType(Divide);
VirtualVisitLAType(Modulus);
VirtualVisitLAType(Less);
VirtualVisitLAType(LessEquals);
VirtualVisitLAType(Greater);
VirtualVisitLAType(GreaterEquals);
VirtualVisitLAType(NotEquals);
VirtualVisitLAType(Equals);
VirtualVisitLAType(DoubleEquals);
VirtualVisitLAType(DoubleAnd);
VirtualVisitLAType(DoubleOr);
VirtualVisitLAType(Not);
VirtualVisitLAType(ParentheseSmallLeft);
VirtualVisitLAType(ParentheseSmallRight);
VirtualVisitLAType(ParentheseMediumLeft);
VirtualVisitLAType(ParentheseMediumRight);
VirtualVisitLAType(ParentheseBigLeft);
VirtualVisitLAType(ParentheseBigRight);
VirtualVisitLAType(Period);
VirtualVisitLAType(Comma);
VirtualVisitLAType(Semicolon);
#undef VirtualVisitLVType

#define VirtualVisitSAType(Type) void ParseVisitor::Visit(Type& type){ VisitSAType(type); } 
VirtualVisitSAType(Text);
VirtualVisitSAType(StatementDefineFunction);
VirtualVisitSAType(StatementDefineVariable);
VirtualVisitSAType(StatementOperate);
VirtualVisitSAType(StatementIf);
VirtualVisitSAType(StatementWhile);
VirtualVisitSAType(StatementBreak);
VirtualVisitSAType(StatementContinue);
VirtualVisitSAType(StatementReturn);
VirtualVisitSAType(StatementNext);
VirtualVisitSAType(StatementBlock);
VirtualVisitSAType(StatementNullable);
VirtualVisitSAType(Statement);
VirtualVisitSAType(Condition);
VirtualVisitSAType(IfNullable);
VirtualVisitSAType(IfNext);
VirtualVisitSAType(ElseNext);
VirtualVisitSAType(AssignmentNullable);
VirtualVisitSAType(Assignment);
VirtualVisitSAType(Expression);
VirtualVisitSAType(ExpressionEnd);
VirtualVisitSAType(ExpressionNot);
VirtualVisitSAType(ExpressionBrackets);
VirtualVisitSAType(Unknown);
VirtualVisitSAType(UnknownOperate);
VirtualVisitSAType(UnknownNullable);
VirtualVisitSAType(UnknownNext);
VirtualVisitSAType(UnknownOperateNode);
VirtualVisitSAType(Type);
VirtualVisitSAType(Bool);
VirtualVisitSAType(ArrayType);
VirtualVisitSAType(FunctionType);
VirtualVisitSAType(AccessObject);
VirtualVisitSAType(AccessArray);
VirtualVisitSAType(FunctionCall);
VirtualVisitSAType(FunctionParameter);
VirtualVisitSAType(ExpressionListNullable);
VirtualVisitSAType(ExpressionListNotNull);
VirtualVisitSAType(ExpressionListNextNullable);
VirtualVisitSAType(IdListNullable);
VirtualVisitSAType(IdListNotNull);
VirtualVisitSAType(IdListNextNullable);

VirtualVisitSAType(ExpressionLevel<0>);
#define TemplateVirtualVisitSAType(N)\
	VirtualVisitSAType(ExpressionLevel<N>);\
	VirtualVisitSAType(ExpressionNode<N>);\
	VirtualVisitSAType(ExpressionNullable<N>);\
	VirtualVisitSAType(ExpressionNext<N>);\
	VirtualVisitSAType(ExpressionSign<N>);
TemplateVirtualVisitSAType(5);
TemplateVirtualVisitSAType(4);
TemplateVirtualVisitSAType(3);
TemplateVirtualVisitSAType(2);
TemplateVirtualVisitSAType(1);
#undef TemplateVirtualVisit

#undef VirtualVisitSAType

#define TypeAccept(Type) void Type::Accept(ParseVisitor& visitor){ visitor.Visit(*this); }
TypeAccept(Blank);
TypeAccept(TextEnd);
TypeAccept(Null);
TypeAccept(False);
TypeAccept(True);
TypeAccept(Char);
TypeAccept(Int);
TypeAccept(Float);
TypeAccept(String);
TypeAccept(Var);
TypeAccept(Id);
TypeAccept(Function);
TypeAccept(Array);
TypeAccept(Object);
TypeAccept(If);
TypeAccept(Else);
TypeAccept(While);
TypeAccept(Break);
TypeAccept(Continue);
TypeAccept(Return);
TypeAccept(Add);
TypeAccept(Subtract);
TypeAccept(Multiply);
TypeAccept(Divide);
TypeAccept(Modulus);
TypeAccept(Less);
TypeAccept(LessEquals);
TypeAccept(Greater);
TypeAccept(GreaterEquals);
TypeAccept(NotEquals);
TypeAccept(Equals);
TypeAccept(DoubleEquals);
TypeAccept(DoubleAnd);
TypeAccept(DoubleOr);
TypeAccept(Not);
TypeAccept(ParentheseSmallLeft);
TypeAccept(ParentheseSmallRight);
TypeAccept(ParentheseMediumLeft);
TypeAccept(ParentheseMediumRight);
TypeAccept(ParentheseBigLeft);
TypeAccept(ParentheseBigRight);
TypeAccept(Period);
TypeAccept(Comma);
TypeAccept(Semicolon);
TypeAccept(Text);
TypeAccept(StatementDefineFunction);
TypeAccept(StatementDefineVariable);
TypeAccept(StatementOperate);
TypeAccept(StatementIf);
TypeAccept(StatementWhile);
TypeAccept(StatementBreak);
TypeAccept(StatementContinue);
TypeAccept(StatementReturn);
TypeAccept(StatementNext);
TypeAccept(StatementBlock);
TypeAccept(StatementNullable);
TypeAccept(Statement);
TypeAccept(Condition);
TypeAccept(IfNullable);
TypeAccept(IfNext);
TypeAccept(ElseNext);
TypeAccept(AssignmentNullable);
TypeAccept(Assignment);
TypeAccept(Expression);
TypeAccept(ExpressionEnd);
TypeAccept(ExpressionNot);
TypeAccept(ExpressionBrackets);
TypeAccept(Unknown);
TypeAccept(UnknownOperate);
TypeAccept(UnknownNullable);
TypeAccept(UnknownNext);
TypeAccept(UnknownOperateNode);
TypeAccept(Type);
TypeAccept(Bool);
TypeAccept(ArrayType);
TypeAccept(FunctionType);
TypeAccept(AccessObject);
TypeAccept(AccessArray);
TypeAccept(FunctionCall);
TypeAccept(FunctionParameter);
TypeAccept(ExpressionListNullable);
TypeAccept(ExpressionListNotNull);
TypeAccept(ExpressionListNextNullable);
TypeAccept(IdListNullable);
TypeAccept(IdListNotNull);
TypeAccept(IdListNextNullable);

TypeAccept(ExpressionLevel<0>);
#define TemplateTypeAccept(N)\
	TypeAccept(ExpressionLevel<N>);\
	TypeAccept(ExpressionNode<N>);\
	TypeAccept(ExpressionNullable<N>);\
	TypeAccept(ExpressionNext<N>);\
	TypeAccept(ExpressionSign<N>);
TemplateTypeAccept(5);
TemplateTypeAccept(4);
TemplateTypeAccept(3);
TemplateTypeAccept(2);
TemplateTypeAccept(1);
#undef TemplateTypeAccept
#undef TypeAccept