#pragma once
/*
	抽象语法树 抽象节点分类
	ASType           -> StatementBlock
	ASType           -> Statement
	ASType           -> Expression
	ASType           -> SpecialOperation
	ASType           -> UnaryOperation
	ASType           -> BinaryOperation

	抽象语法树 派生类分类
	Expression       -> Null
	Expression       -> Bool
	Expression       -> Char
	Expression       -> Int
	Expression       -> Float
	Expression       -> String
	Expression       -> Array
	Expression       -> Function
	Expression       -> Object
	Expression       -> Id

	Statement        -> StatementDefineFunction
	Statement        -> StatementDefineVariable
	Statement        -> StatementAssignmentId
	Statement        -> StatementAssignmentArray
	Statement        -> StatementAssignmentField
	Statament        -> StatementCall
	Statement        -> StatementIf
	Statement        -> StatementWhile
	Statement        -> StatementBreak
	Statement        -> StatementContinue
	Statement        -> StatementReturn

	SpecialOperation -> FunctionCall
	SpecialOperation -> AccessArray
	SpecialOperation -> AccessField

	UnaryOperation   -> Not
	UnaryOperation   -> Negative

	BinaryOperation  -> Or
	BinaryOperation  -> And
	BinaryOperation  -> Equals
	BinaryOperation  -> NotEquals
	BinaryOperation  -> Less
	BinaryOperation  -> LessEquals
	BinaryOperation  -> Greater
	BinaryOperation  -> GreaterEquals
	BinaryOperation  -> Add
	BinaryOperation  -> Subtract
	BinaryOperation  -> Multiply
	BinaryOperation  -> Divide
	BinaryOperation  -> Modulus

*/
#include <memory>
#include <vector>
#include <string>
#include <set>
using std::set;
using std::unique_ptr;
using std::wstring;
using std::vector;

namespace AbstractSyntax {
	struct AbstractSyntaxVisitor;

	//基类
	struct AbstractSyntaxType {
		virtual ~AbstractSyntaxType() = default;
		virtual void Accept(AbstractSyntaxVisitor& visitor) = 0;
		int line = 0;
	};

	//抽象类
	struct Expression : public AbstractSyntaxType {};
	struct Statement : public AbstractSyntaxType {};
	struct SpecialOperation : public AbstractSyntaxType {};
	struct UnaryOperation : public Expression {
		unique_ptr<Expression> expression;
	};
	struct BinaryOperation : public Expression {
		unique_ptr<Expression> left;
		unique_ptr<Expression> right;
	};
	//语句块 实现
	struct StatementBlock : public AbstractSyntaxType {
		vector<unique_ptr<Statement>> statements;
	};

	struct MainBlock : public StatementBlock {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		set<wstring> closure;
	};
	struct FunctionBlock : public StatementBlock {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		set<wstring> closure;
	};
	struct DefaultBlock : public StatementBlock {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};
	struct WhileBlock : public StatementBlock {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};

	//表达式实现
	struct Null : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};
	struct Bool : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		bool value = false;
	};
	struct Char : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wchar_t value = L'\0';
	};
	struct Int : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		int value = 0;
	};
	struct Float : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		float value = 0;
	};
	struct String : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring value;
	};
	struct Array : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<Expression> length;
	};
	struct Function : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		vector<wstring> idList;
		FunctionBlock functionBlock;
	};
	struct Object : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};
	struct SpecialOperationList : public Expression {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring id;
		vector<unique_ptr<SpecialOperation>> specialOperations;
	};


	//语句实现
	struct StatementDefineFunction : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring id;
		vector<wstring> idList;
		FunctionBlock functionBlock;
	};
	struct StatementDefineVariable : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring id;
		unique_ptr<Expression> expression;
	};
	struct StatementAssignmentId : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring id;
		unique_ptr<Expression> expression;
	};
	struct StatementAssignmentArray : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<SpecialOperationList> specialOperationList;
		unique_ptr<Expression> index;
		unique_ptr<Expression> expression;
	};
	struct StatementAssignmentField : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<SpecialOperationList> specialOperationList;
		wstring field;
		unique_ptr<Expression> expression;
	};
	struct StatementCall : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<SpecialOperationList> specialOperationList;
	};
	struct StatementIf : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<Expression> condition;
		DefaultBlock ifBlock;
		DefaultBlock elseBlock;
	};
	struct StatementWhile : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<Expression> condition;
		WhileBlock whileBlock;
	};
	struct StatementBreak : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};
	struct StatementContinue : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};
	struct StatementReturn : public Statement {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<Expression> expression;
	};

	//特殊操作
	struct FunctionCall : public SpecialOperation {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		vector<unique_ptr<Expression>> expressionList;
	};
	struct AccessArray : public SpecialOperation {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		unique_ptr<Expression> index;
	};
	struct AccessField : public SpecialOperation {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
		wstring id;
	};

	//一元操作
	struct Not : public UnaryOperation {
		virtual void Accept(AbstractSyntaxVisitor& visitor);
	};

	//二元操作
#define ASTypeBinaryOperation(Type) struct Type : public BinaryOperation{ virtual void Accept(AbstractSyntaxVisitor& visitor); };
	ASTypeBinaryOperation(Or);
	ASTypeBinaryOperation(And);
	ASTypeBinaryOperation(NotEquals);
	ASTypeBinaryOperation(Equals);
	ASTypeBinaryOperation(Less);
	ASTypeBinaryOperation(LessEquals);
	ASTypeBinaryOperation(Greater);
	ASTypeBinaryOperation(GreaterEquals);
	ASTypeBinaryOperation(Add);
	ASTypeBinaryOperation(Subtract);
	ASTypeBinaryOperation(Multiply);
	ASTypeBinaryOperation(Divide);
	ASTypeBinaryOperation(Modulus);
#undef ASTypeBinaryOperation

	struct AbstractSyntaxVisitor {
		~AbstractSyntaxVisitor() = default;
		virtual void VisitAbstractSyntaxType(AbstractSyntaxType& type);
		virtual void VisitStatement(Statement& type);
		virtual void VisitExpression(Expression& type);
		virtual void VisitSpecialOperation(SpecialOperation& type);
		virtual void VisitUnaryOperation(UnaryOperation& type);
		virtual void VisitBinaryOperation(BinaryOperation& type);
		virtual void VisitStatementBlock(StatementBlock& type);
#define VirtualVisit(Type) virtual void Visit(Type& type);
		VirtualVisit(MainBlock);
		VirtualVisit(FunctionBlock);
		VirtualVisit(DefaultBlock);
		VirtualVisit(WhileBlock);
		VirtualVisit(StatementDefineFunction);
		VirtualVisit(StatementDefineVariable);
		VirtualVisit(StatementAssignmentId);
		VirtualVisit(StatementAssignmentArray);
		VirtualVisit(StatementAssignmentField);
		VirtualVisit(StatementCall);
		VirtualVisit(StatementIf);
		VirtualVisit(StatementWhile);
		VirtualVisit(StatementBreak);
		VirtualVisit(StatementContinue);
		VirtualVisit(StatementReturn);
		VirtualVisit(Null);
		VirtualVisit(Bool);
		VirtualVisit(Char);
		VirtualVisit(Int);
		VirtualVisit(Float);
		VirtualVisit(String);
		VirtualVisit(Array);
		VirtualVisit(Function);
		VirtualVisit(Object);
		VirtualVisit(SpecialOperationList);
		VirtualVisit(FunctionCall);
		VirtualVisit(AccessArray);
		VirtualVisit(AccessField);
		VirtualVisit(Not);
		VirtualVisit(Or);
		VirtualVisit(And);
		VirtualVisit(Equals);
		VirtualVisit(NotEquals);
		VirtualVisit(Less);
		VirtualVisit(LessEquals);
		VirtualVisit(Greater);
		VirtualVisit(GreaterEquals);
		VirtualVisit(Add);
		VirtualVisit(Subtract);
		VirtualVisit(Multiply);
		VirtualVisit(Divide);
		VirtualVisit(Modulus);
#undef VirtualVisit
	};
}