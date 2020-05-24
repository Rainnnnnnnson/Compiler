/*
	使用LL(1)分析 下面是产生式 已消除左递归
	ExpressionLevel<n> 使用递归 来确定优先级符号
	使用Visitor模式

	LAType 表示 词法分析得出的 <终结符>
	SAType 表示 语法分析得出的 <非终结符>

	-------------------------------------------------------------------------
	类型
	Text                       -> StatementNullable TextEnd

	StatementDefineFunction    -> Function Id FunctionParameter StatementBlock
	StatementDefineVariable    -> Var Id = Expression ;
	StatementOperate           -> UnknownOperate AssignmentNullable ;
	StatementIf                -> If Condition StatementBlock IfNullable
	StatementWhile             -> While Condition StatementBlock
	StatementBreak             -> Break ;
	StatementContinue          -> Continue ;
	StatementReturn            -> Return Expression ;
	StatementNext              -> Statement StatementNullable
	StatementBlock             -> { StatementNullable }
	StatementNullable          -> StatementNext
	StatementNullable          -> ε
	Statement                  -> StatementDefineFunction
	Statement                  -> StatementDefineVariable
	Statement                  -> StatementOperate
	Statement                  -> StatementIf
	Statement                  -> StatementWhile
	Statement                  -> StatementBreak
	Statement                  -> StatementContinue
	Statement                  -> StatementReturn

	Condition                  -> ( Expression )
	IfNullable                 -> IfNext
	IfNullable                 -> ε
	IfNext                     -> Else ElseNext
	ElseNext                   -> StatementIf
	ElseNext                   -> StatementBlock

	AssignmentNullable         -> Assignment
	AssignmentNullable         -> ε
	Assignment                 -> = Expression

	Expression                 -> ExpressionLevel<N>
	ExpressionLevel<N>         -> ExpressionNode<N> ExpressionNullable<N>
	ExpressionNode<N>          -> ExpressionLevel<N - 1>
	ExpressionNullable<N>      -> ExpressionNext<N>
	ExpressionNullable<N>      -> ε
	ExpressionNext<N>          -> ExpressionSign<N> ExpressionLevel<N>
	ExpressionLevel<0>         -> ExpressionEnd

	ExpressionEnd              -> ExpressionNot
	ExpressionEnd              -> ExpressionBrackets
	ExpressionEnd              -> Unknown
	ExpressionNot              -> ! Unknown
	ExpressionBrackets         -> ( Expression )

	Unknown                    -> Type
	Unknown                    -> UnknownOperate
	UnknownOperate             -> Id UnknownNullable
	UnknownNullable            -> UnknownNext
	UnknownNullable            -> ε
	UnknownNext                -> UnknownOperateNode UnknownNullable
	UnknownOperateNode         -> AccessArray
	UnknownOperateNode         -> AccessObject
	UnknownOperateNode         -> FunctionCall

	Type                       -> Null
	Type                       -> Bool
	Type                       -> Char
	Type                       -> Int
	Type                       -> Float
	Type                       -> String
	Type                       -> Object
	Type                       -> ArrayType
	Type                       -> FunctionType

	Bool                       -> False
	Bool                       -> True

	ArrayType                  -> Array AccessArray
	FunctionType               -> Function FunctionParameter StatementBlock
	FunctionParameter          -> ( IdListNullable )

	AccessObject               -> . Id
	AccessArray                -> [ Expression ]
	FunctionCall               -> ( ExpressionListNullable )

	ExpressionListNullable     -> ε
	ExpressionListNullable     -> ExpressionListNotNull
	ExpressionListNotNull      -> Expression ExpressionListNextNullable
	ExpressionListNextNullable -> , Expression ExpressionListNextNullable
	ExpressionListNextNullable -> ε

	IdListNullable             -> ε
	IdListNullable             -> IdListNotNull
	IdListNotNull              -> Id IdListNextNullable
	IdListNextNullable         -> , Id IdListNextNullable
	IdListNextNullable         -> ε

	--------------------------------------------------------------------------
	运算优先级
	ExpressionSign<N>          -> <Sign>

	ExpressionSign<5>  ||  &&
	ExpressionSign<4>  ==  !=
	ExpressionSign<3>  <=  >=  <   >
	ExpressionSign<2>  +   -
	ExpressionSign<1>  *   /   %

	--------------------------------------------------------------------------
	特殊
	Blank 注释空格都是它 语义分析中忽视它
*/
#pragma once
#include<vector>
#include<string>
#include<memory>
using std::unique_ptr;
using std::wstring;
using std::vector;

namespace Parse {
	struct ParseVisitor;

	struct ParseType {
		virtual ~ParseType() = default;
		virtual void Accept(ParseVisitor& visitor) = 0;
		int line = 0;
	};

	struct LAType : public ParseType {
	};

	struct SAType : public ParseType {
		vector<unique_ptr<ParseType>> parseTypes;
	};

	template<size_t N>
	struct ExpressionLevel : public SAType {
		virtual void Accept(ParseVisitor& visitor);
	};

	template<size_t N>
	struct ExpressionNullable : public SAType {
		virtual void Accept(ParseVisitor& visitor);
	};

	template<size_t N>
	struct ExpressionNext : public SAType {
		virtual void Accept(ParseVisitor& visitor);
	};

	template<size_t N>
	struct ExpressionNode : public SAType {
		virtual void Accept(ParseVisitor& visitor);
	};

	template<size_t N>
	struct ExpressionSign : public SAType {
		virtual void Accept(ParseVisitor& visitor);
	};

	struct Char : public LAType {
		virtual void Accept(ParseVisitor& visitor);
		wchar_t value = L'\0';
	};

	struct Int : public LAType {
		virtual void Accept(ParseVisitor& visitor);
		int value = 0;
	};

	struct Float : public LAType {
		virtual void Accept(ParseVisitor& visitor);
		float value = 0;
	};

	struct String : public LAType {
		virtual void Accept(ParseVisitor& visitor);
		wstring value;
	};

	struct Id : public LAType {
		virtual void Accept(ParseVisitor& visitor);
		wstring value;
	};

#define DerivedLAType(Type) struct Type : public LAType { virtual void Accept(ParseVisitor& visitor); };
//DerivedLAType(Char);
//DerivedLAType(Int);
//DerivedLAType(Float);
//DerivedLAType(String);
//DerivedLAType(Id);
	DerivedLAType(Blank);
	DerivedLAType(TextEnd);
	DerivedLAType(Null);
	DerivedLAType(False);
	DerivedLAType(True);
	DerivedLAType(Var);
	DerivedLAType(Function);
	DerivedLAType(Array);
	DerivedLAType(Object);
	DerivedLAType(If);
	DerivedLAType(Else);
	DerivedLAType(While);
	DerivedLAType(Break);
	DerivedLAType(Continue);
	DerivedLAType(Return);
	DerivedLAType(Add);
	DerivedLAType(Subtract);
	DerivedLAType(Multiply);
	DerivedLAType(Divide);
	DerivedLAType(Modulus);
	DerivedLAType(Less);
	DerivedLAType(LessEquals);
	DerivedLAType(Greater);
	DerivedLAType(GreaterEquals);
	DerivedLAType(NotEquals);
	DerivedLAType(Equals);
	DerivedLAType(DoubleEquals);
	DerivedLAType(DoubleAnd);
	DerivedLAType(DoubleOr);
	DerivedLAType(Not);
	DerivedLAType(ParentheseSmallLeft);
	DerivedLAType(ParentheseSmallRight);
	DerivedLAType(ParentheseMediumLeft);
	DerivedLAType(ParentheseMediumRight);
	DerivedLAType(ParentheseBigLeft);
	DerivedLAType(ParentheseBigRight);
	DerivedLAType(Period);
	DerivedLAType(Comma);
	DerivedLAType(Semicolon);
#undef DerivedLAType

#define DerivedSAType(Type) struct Type : public SAType { virtual void Accept(ParseVisitor& visitor); };
	DerivedSAType(Text);
	DerivedSAType(StatementDefineFunction);
	DerivedSAType(StatementDefineVariable);
	DerivedSAType(StatementOperate);
	DerivedSAType(StatementIf);
	DerivedSAType(StatementWhile);
	DerivedSAType(StatementBreak);
	DerivedSAType(StatementContinue);
	DerivedSAType(StatementReturn);
	DerivedSAType(StatementNext);
	DerivedSAType(StatementBlock);
	DerivedSAType(StatementNullable);
	DerivedSAType(Statement);
	DerivedSAType(Condition);
	DerivedSAType(IfNullable);
	DerivedSAType(IfNext);
	DerivedSAType(ElseNext);
	DerivedSAType(AssignmentNullable);
	DerivedSAType(Assignment);
	DerivedSAType(Expression);
	DerivedSAType(ExpressionEnd);
	DerivedSAType(ExpressionNot);
	DerivedSAType(ExpressionBrackets);
	DerivedSAType(Unknown);
	DerivedSAType(UnknownOperate);
	DerivedSAType(UnknownOperateNode);
	DerivedSAType(UnknownNullable);
	DerivedSAType(UnknownNext);
	DerivedSAType(Type);
	DerivedSAType(Bool);
	DerivedSAType(ArrayType);
	DerivedSAType(FunctionType);
	DerivedSAType(AccessObject);
	DerivedSAType(AccessArray);
	DerivedSAType(FunctionCall);
	DerivedSAType(FunctionParameter);
	DerivedSAType(ExpressionListNullable);
	DerivedSAType(ExpressionListNotNull);
	DerivedSAType(ExpressionListNextNullable);
	DerivedSAType(IdListNullable);
	DerivedSAType(IdListNotNull);
	DerivedSAType(IdListNextNullable);
#undef DerivedSAType

	struct ParseVisitor {
		~ParseVisitor() = default;
		virtual void VisitParseType(ParseType& type);
		virtual void VisitLAType(LAType& type);
		virtual void VisitSAType(SAType& type);
#define VirtualVisit(Type) virtual void Visit(Type& type);
		VirtualVisit(Blank);
		VirtualVisit(TextEnd);
		VirtualVisit(Null);
		VirtualVisit(False);
		VirtualVisit(True);
		VirtualVisit(Char);
		VirtualVisit(Int);
		VirtualVisit(Float);
		VirtualVisit(String);
		VirtualVisit(Var);
		VirtualVisit(Id);
		VirtualVisit(Function);
		VirtualVisit(Array);
		VirtualVisit(Object);
		VirtualVisit(If);
		VirtualVisit(Else);
		VirtualVisit(While);
		VirtualVisit(Break);
		VirtualVisit(Continue);
		VirtualVisit(Return);
		VirtualVisit(Add);
		VirtualVisit(Subtract);
		VirtualVisit(Multiply);
		VirtualVisit(Divide);
		VirtualVisit(Modulus);
		VirtualVisit(Less);
		VirtualVisit(LessEquals);
		VirtualVisit(Greater);
		VirtualVisit(GreaterEquals);
		VirtualVisit(NotEquals);
		VirtualVisit(Equals);
		VirtualVisit(DoubleEquals);
		VirtualVisit(DoubleAnd);
		VirtualVisit(DoubleOr);
		VirtualVisit(Not);
		VirtualVisit(ParentheseSmallLeft);
		VirtualVisit(ParentheseSmallRight);
		VirtualVisit(ParentheseMediumLeft);
		VirtualVisit(ParentheseMediumRight);
		VirtualVisit(ParentheseBigLeft);
		VirtualVisit(ParentheseBigRight);
		VirtualVisit(Period);
		VirtualVisit(Comma);
		VirtualVisit(Semicolon);

		VirtualVisit(Text);
		VirtualVisit(StatementDefineFunction);
		VirtualVisit(StatementDefineVariable);
		VirtualVisit(StatementOperate);
		VirtualVisit(StatementIf);
		VirtualVisit(StatementWhile);
		VirtualVisit(StatementBreak);
		VirtualVisit(StatementContinue);
		VirtualVisit(StatementReturn);
		VirtualVisit(StatementNext);
		VirtualVisit(StatementBlock);
		VirtualVisit(StatementNullable);
		VirtualVisit(Statement);

		VirtualVisit(Condition);
		VirtualVisit(IfNullable);
		VirtualVisit(IfNext);
		VirtualVisit(ElseNext);
		VirtualVisit(AssignmentNullable);
		VirtualVisit(Assignment);

		VirtualVisit(Expression);
		VirtualVisit(ExpressionEnd);
		VirtualVisit(ExpressionNot);
		VirtualVisit(ExpressionBrackets);

		VirtualVisit(Unknown);
		VirtualVisit(UnknownOperate);
		VirtualVisit(UnknownOperateNode);
		VirtualVisit(UnknownNullable);
		VirtualVisit(UnknownNext);

		VirtualVisit(Type);
		VirtualVisit(Bool);
		VirtualVisit(ArrayType);
		VirtualVisit(FunctionType);
		VirtualVisit(AccessObject);
		VirtualVisit(AccessArray);
		VirtualVisit(FunctionCall);
		VirtualVisit(FunctionParameter);

		VirtualVisit(ExpressionListNullable);
		VirtualVisit(ExpressionListNotNull);
		VirtualVisit(ExpressionListNextNullable);
		VirtualVisit(IdListNullable);
		VirtualVisit(IdListNotNull);
		VirtualVisit(IdListNextNullable);

		VirtualVisit(ExpressionLevel<0>);
#define TemplateVirtualVisit(N)\
	VirtualVisit(ExpressionLevel<N>);\
	VirtualVisit(ExpressionNode<N>);\
	VirtualVisit(ExpressionNullable<N>);\
	VirtualVisit(ExpressionNext<N>);\
	VirtualVisit(ExpressionSign<N>);
		TemplateVirtualVisit(5);
		TemplateVirtualVisit(4);
		TemplateVirtualVisit(3);
		TemplateVirtualVisit(2);
		TemplateVirtualVisit(1);
#undef TemplateVirtualVisit

#undef VirtualVisit
	};

}