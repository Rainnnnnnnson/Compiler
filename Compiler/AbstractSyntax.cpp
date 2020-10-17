#include "AbstractSyntax.h"
#include "CompilerException.h"
#include <typeindex>
#include <memory>
#include <set>
#include <functional>
#include <algorithm>
#include <optional>
#include <iterator>
using std::optional;
using std::function;
using std::make_unique;
using std::set;
using std::pair;
using std::type_index;
using std::unique_ptr;
using namespace AbstractSyntax;

struct EnvironmentBlockState {
    EnvironmentBlockState(bool inWhile, bool functionButtom) : inWhile(inWhile), functionButtom(functionButtom) {}
    bool inWhile;
    bool functionButtom;
    set<wstring> idSet;
    set<wstring> closureSet;
};


class SemanticAnalysisEnvironment {
public:
    SemanticAnalysisEnvironment(const vector<wstring>& registeredNames) {
        environmentBlockStates.push_back(EnvironmentBlockState(false, false));
        for (auto& name : registeredNames) {
            DefineVariable(name);
        }
    }
    void EnterMainBlock() {
        environmentBlockStates.push_back(EnvironmentBlockState(false, true));
    }
    void EnterFunctionBlock() {
        environmentBlockStates.push_back(EnvironmentBlockState(false, true));
    }
    void EnterDefaultBlock() {
        bool last = environmentBlockStates.back().inWhile;
        environmentBlockStates.push_back(EnvironmentBlockState(last, false));
    }
    void EnterWhileBlock() {
        environmentBlockStates.push_back(EnvironmentBlockState(true, false));
    }
    set<wstring> ExitBlock() {
        auto back = std::move(environmentBlockStates.back());
        environmentBlockStates.pop_back();
        auto closureSet = std::move(back.closureSet);
        auto& top = environmentBlockStates.back();
        //根据上一层闭包来判断是否需要加入上一层的闭包
        for (auto& item : closureSet) {
            auto find = top.idSet.find(item);
            if (find == top.idSet.end()) {
                if (FindIdInCurrentBlock(item)) {
                    //当前层有不需要加入闭包
                } else if (FindIdInCurrentFunction(item)) {
                    DefineVariableAndClosure(item);
                } else if (FindIdInPrevousEnvironment(item)) {
                    DefineVariableAndClosure(item);
                } else {
                    throw CompilerError();
                }
            }
        }
        return closureSet;
    }
    set<wstring> ExitMainBlock() {
        return ExitBlock();
    }

    set<wstring> ExitFunctionBlock() {
        return ExitBlock();
    }
    void ExitDefaultBlock() {
        ExitBlock();
    }
    void ExitWhileBlock() {
        ExitBlock();
    }
    bool InWhile() {
        return environmentBlockStates.back().inWhile;
    }
    //当前块
    bool FindIdInCurrentBlock(const wstring& id) {
        auto& current = environmentBlockStates.back();
        if (current.idSet.find(id) != current.idSet.end()) {
            return true;
        }
        return false;
    }
    //当前块到函数块为止 (含当前块)
    bool FindIdInCurrentFunction(const wstring& id) {
        auto iter = environmentBlockStates.rbegin();
        auto end = iter;
        while (end->functionButtom == false) {
            end += 1;
        }
        end += 1;
        while (iter < end) {
            if (iter->idSet.find(id) != iter->idSet.end()) {
                return true;
            }
            iter += 1;
        }
        return false;
    }
    //函数块之前所有
    bool FindIdInPrevousEnvironment(const wstring& id) {
        auto iter = environmentBlockStates.rbegin();
        while (iter->functionButtom == false) {
            iter += 1;
        }
        iter += 1;
        auto end = environmentBlockStates.rend();
        while (iter < end) {
            if (iter->idSet.find(id) != iter->idSet.end()) {
                return true;
            }
            iter += 1;
        }
        return false;
    }

    void DefineVariable(const wstring& id) {
        auto& current = environmentBlockStates.back();
        if (!current.idSet.insert(id).second) {
            throw CompilerError();
        }
    }
    void DefineVariableAndClosure(const wstring& id) {
        auto& current = environmentBlockStates.back();
        if (!current.idSet.insert(id).second) {
            throw CompilerError();
        }
        if (!current.closureSet.insert(id).second) {
            throw CompilerError();
        }
    }
private:
    vector<EnvironmentBlockState> environmentBlockStates;
};

class SpecialOperationProcess : AbstractSyntaxVisitor {
public:
    SpecialOperationProcess(SemanticAnalysisEnvironment& environment) : environment(environment) {}
    void Handle(SpecialOperation& type) {
        type.Accept(*this);
    }
    void Visit(FunctionCall& type) override;
    void Visit(AccessArray& type) override;
    void Visit(AccessField& type) override {}
private:
    SemanticAnalysisEnvironment& environment;
};

class ExpressionProcess : public AbstractSyntaxVisitor {
public:
    ExpressionProcess(SemanticAnalysisEnvironment& environment) : environment(environment) {}
    void Handle(Expression& type) {
        type.Accept(*this);
    }
    void Visit(SpecialOperationList& type) override {
        if (environment.FindIdInCurrentFunction(type.id)) {

        } else if (environment.FindIdInPrevousEnvironment(type.id)) {
            environment.DefineVariableAndClosure(type.id);
        } else {
            throw CompileException(MessageHead(type.line) + WstringToString(type.id) + "在环境中无法找到");
        }
        for (auto& item : type.specialOperations) {
            SpecialOperationProcess(environment).Handle(*item);
        }
    }
    void VisitBinaryOperation(BinaryOperation& type) override {
        ExpressionProcess(environment).Handle(*type.left);
        ExpressionProcess(environment).Handle(*type.right);
    }
    void VisitUnaryOperation(UnaryOperation& type) override {
        ExpressionProcess(environment).Handle(*type.expression);
    }
    void Visit(Null& type) override {}
    void Visit(Bool& type) override {}
    void Visit(Char& type) override {}
    void Visit(Int& type) override {}
    void Visit(Float& type) override {}
    void Visit(String& type) override {}
    void Visit(Array& type) override {
        ExpressionProcess(environment).Handle(*type.length);
    }
    void Visit(Function& type) override;
    void Visit(Object& type) override {}
private:
    SemanticAnalysisEnvironment& environment;
};

void SpecialOperationProcess::Visit(FunctionCall& type) {
    for (auto& item : type.expressionList) {
        ExpressionProcess(environment).Handle(*item);
    }
}

void SpecialOperationProcess::Visit(AccessArray& type) {
    ExpressionProcess(environment).Handle(*type.index);
}

class FunctionBlockProcess : public AbstractSyntaxVisitor {
public:
    FunctionBlockProcess(SemanticAnalysisEnvironment& environment) : environment(environment) {}
    unique_ptr<StatementReturn> CreateReturnNull(int line) {
        auto p = make_unique<StatementReturn>();
        p->expression = make_unique<Null>();
        p->line = line;
        return std::move(p);
    }
    void Handle(FunctionBlock& type, const vector<wstring>& idList);
private:
    SemanticAnalysisEnvironment& environment;
};

void ExpressionProcess::Visit(Function& type) {
    FunctionBlockProcess(environment).Handle(type.functionBlock, type.idList);
}

class IsBreakContinueProcess : public AbstractSyntaxVisitor {
public:
    IsBreakContinueProcess(SemanticAnalysisEnvironment& environment) : environment(environment), b(false) {}
    bool Handle(Statement& type) {
        type.Accept(*this);
        return b;
    }
    void Visit(StatementBreak& type) override {
        b = true;
    }
    void Visit(StatementContinue& type) override {
        b = true;
    }
    void VisitStatement(Statement& type) override {
        b = false;
    }
private:
    bool b;
    SemanticAnalysisEnvironment& environment;
};

class IsReturnProcess : public AbstractSyntaxVisitor {
public:
    IsReturnProcess(SemanticAnalysisEnvironment& environment) : environment(environment), b(false) {}
    bool Handle(Statement& type) {
        type.Accept(*this);
        return b;
    }
    void Visit(StatementReturn& type) override {
        b = true;
    }
    void VisitStatement(Statement& type) override {
        b = false;
    }
private:
    bool b;
    SemanticAnalysisEnvironment& environment;
};

class WhileBlockProcess : public AbstractSyntaxVisitor {
public:
    WhileBlockProcess(SemanticAnalysisEnvironment& environment) : environment(environment) {}
    void Handle(WhileBlock& type);
private:
    SemanticAnalysisEnvironment& environment;
};

class DefaultBlockProcess : public AbstractSyntaxVisitor {
public:
    DefaultBlockProcess(SemanticAnalysisEnvironment& environment) : environment(environment) {}
    void Handle(DefaultBlock& type);
private:
    SemanticAnalysisEnvironment& environment;
};

class StatementProcess : public AbstractSyntaxVisitor {
public:
    StatementProcess(SemanticAnalysisEnvironment& environment) :environment(environment) {}
    void Handle(AbstractSyntaxType& type) {
        type.Accept(*this);
    }
    void Visit(StatementDefineFunction& type) override {
        if (environment.FindIdInCurrentBlock(type.id)) {
            throw CompileException(MessageHead(type.line) + WstringToString(type.id) + "在当前语句块中存在");
        }
        environment.DefineVariable(type.id);
        FunctionBlockProcess(environment).Handle(type.functionBlock, type.idList);
    }
    void Visit(StatementDefineVariable& type) override {
        ExpressionProcess(environment).Handle(*type.expression);
        if (environment.FindIdInCurrentBlock(type.id)) {
            throw CompileException(MessageHead(type.line) + WstringToString(type.id) + "在当前语句块中存在");
        }
        environment.DefineVariable(type.id);
    }
    void Visit(StatementAssignmentId& type) override {
        ExpressionProcess(environment).Handle(*type.expression);
        if (environment.FindIdInCurrentFunction(type.id)) {
            return;
        }
        if (environment.FindIdInPrevousEnvironment(type.id)) {
            environment.DefineVariableAndClosure(type.id);
            return;
        }
        throw CompileException(MessageHead(type.line) + WstringToString(type.id) + "在环境中无法找到");
    }

    void Visit(StatementAssignmentField& type) override {
        ExpressionProcess(environment).Handle(*type.specialOperationList);
        ExpressionProcess(environment).Handle(*type.expression);
    }
    void Visit(StatementAssignmentArray& type) override {
        ExpressionProcess(environment).Handle(*type.specialOperationList);
        ExpressionProcess(environment).Handle(*type.index);
        ExpressionProcess(environment).Handle(*type.expression);
    }
    void Visit(StatementCall& type) override {
        ExpressionProcess(environment).Handle(*type.specialOperationList);

    }
    void Visit(StatementIf& type) override {
        ExpressionProcess(environment).Handle(*type.condition);
        DefaultBlockProcess(environment).Handle(type.ifBlock);
        DefaultBlockProcess(environment).Handle(type.elseBlock);
    }
    void Visit(StatementWhile& type) override {
        ExpressionProcess(environment).Handle(*type.condition);
        WhileBlockProcess(environment).Handle(type.whileBlock);
    }
    void Visit(StatementBreak& type) override {}
    void Visit(StatementContinue& type) override {}
    void Visit(StatementReturn& type) override {
        ExpressionProcess(environment).Handle(*type.expression);
    }
private:
    SemanticAnalysisEnvironment& environment;
};

void DefaultBlockProcess::Handle(DefaultBlock& type) {
    if (type.statements.empty()) {
        return;
    }
    environment.EnterDefaultBlock();
    if (!environment.InWhile()) {
        for (auto& item : type.statements) {
            if (IsBreakContinueProcess(environment).Handle(*item)) {
                throw CompileException(MessageHead(type.line) + "不该出现break continue");
            }
        }
    }
    auto iter = type.statements.begin();
    auto end = type.statements.end() - 1;
    for (iter; iter < end; iter += 1) {
        if (IsBreakContinueProcess(environment).Handle(**iter)) {
            throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现break continue");
        } else if (IsReturnProcess(environment).Handle(**iter)) {
            throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现return");
        }
    }
    for (auto& item : type.statements) {
        StatementProcess(environment).Handle(*item);
    }
    environment.ExitDefaultBlock();
}

void WhileBlockProcess::Handle(WhileBlock& type) {
    if (type.statements.empty()) {
        return;
    }
    environment.EnterWhileBlock();
    auto iter = type.statements.begin();
    auto end = type.statements.end() - 1;
    for (iter; iter < end; iter += 1) {
        if (IsBreakContinueProcess(environment).Handle(**iter)) {
            throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现break continue");
        } else if (IsReturnProcess(environment).Handle(**iter)) {
            throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现return");
        }
    }
    for (auto& item : type.statements) {
        StatementProcess(environment).Handle(*item);
    }
    environment.ExitWhileBlock();
}

void FunctionBlockProcess::Handle(FunctionBlock& type, const vector<wstring>& idList) {
    if (type.statements.empty()) {
        type.statements.push_back(CreateReturnNull(type.line));
    } else if (!IsReturnProcess(environment).Handle(*type.statements.back())) {
        type.statements.push_back(CreateReturnNull(type.line));
    }
    environment.EnterFunctionBlock();
    for (auto& id : idList) {
        if (environment.FindIdInCurrentBlock(id)) {
            throw CompileException(MessageHead(type.line) + WstringToString(id) + "函数参数重复出现");
        }
        environment.DefineVariable(id);
    }
    for (auto& item : type.statements) {
        if (IsBreakContinueProcess(environment).Handle(*item)) {
            throw CompileException(MessageHead(type.line) + "不应该出现break continue");
        }
    }
    auto iter = type.statements.begin();
    auto end = type.statements.end() - 1;
    for (iter; iter < end; iter += 1) {
        if (IsReturnProcess(environment).Handle(**iter)) {
            throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现return");
        }
    }
    for (auto& item : type.statements) {
        StatementProcess(environment).Handle(*item);
    }
    type.closure = environment.ExitFunctionBlock();
}

class MainBlockProcess : public AbstractSyntaxVisitor {
public:
    MainBlockProcess(const RegisteredNameList& nameList) : environment(nameList.registeredNames) {}
    void Handle(MainBlock& type) {
        type.Accept(*this);
    }
    unique_ptr<StatementReturn> CreateReturnNull(int line) {
        auto p = make_unique<StatementReturn>();
        p->expression = make_unique<Null>();
        p->line = line;
        return std::move(p);
    }
    void Visit(MainBlock& type) override {
        if (type.statements.empty()) {
            type.statements.push_back(CreateReturnNull(type.line));
        } else if (!IsReturnProcess(environment).Handle(*type.statements.back())) {
            type.statements.push_back(CreateReturnNull(type.line));
        }
        environment.EnterMainBlock();
        for (auto& item : type.statements) {
            if (IsBreakContinueProcess(environment).Handle(*item)) {
                throw CompileException(MessageHead(type.line) + "不应该出现break continue");
            }
        }
        auto iter = type.statements.begin();
        auto end = type.statements.end() - 1;
        for (iter; iter < end; iter += 1) {
            if (IsReturnProcess(environment).Handle(**iter)) {
                throw CompileException(MessageHead(type.line) + "非语句块结尾不该出现return");
            }
        }
        for (auto& item : type.statements) {
            StatementProcess(environment).Handle(*item);
        }
        type.closure = environment.ExitMainBlock();
    }
private:
    SemanticAnalysisEnvironment environment;
};

AbstractSyntaxTreeTransform SemanticAnalysis(const RegisteredNameList& nameList, AbstractSyntaxTree&& abstractSyntaxTree) {

    auto root = std::move(abstractSyntaxTree.root);
    MainBlockProcess(nameList).Handle(root);
    return AbstractSyntaxTreeTransform(std::move(root));
}

/*----------------------------------------------------------------------------------------
                                    上面语义分析 下面生成代码
----------------------------------------------------------------------------------------*/

struct VariableData {
    inline VariableData(bool closure, int16_t offest) : closure(closure), offest(offest) {}
    bool closure;
    int16_t offest;
};

struct CodeGenerateBlock {
    CodeGenerateBlock() : whileBegin() {}
    CodeGenerateBlock(int32_t whileBegin) : whileBegin(whileBegin) {}
    map<wstring, int16_t> variableOffest;
    optional<int32_t> whileBegin;
    vector<function<void(int32_t)>> breakActions;
    vector<function<void(int32_t)>> continueActions;
};

class StringMap {
public:
    void Insert(const wstring& idName) {
        int32_t index = static_cast<int32_t>(idNames.size());
        idNames.push_back(idName);
        idToIndexMap.insert(pair(idName, index));
    }
    int32_t GetIndex(const wstring& idName) const {
        auto find = idToIndexMap.find(idName);
        if (find != idToIndexMap.end()) {
            return find->second;
        }
        throw CompilerError();
    }
public:
    vector<wstring> idNames;
    map<wstring, int32_t> idToIndexMap;
};

class CodeGenerateStack {
public:
    CodeGenerateStack() : currentStackOffest(0) {}
    CodeGenerateStack(const vector<wstring>& closure, const vector<wstring>& parameters) : closure(closure) {
        blocks.push_back(CodeGenerateBlock());
        auto& top = blocks[0];
        //压入 SP PC 闭包 起始偏移为2
        int16_t offest = 2;
        for (auto& parameter : parameters) {
            offest += 1;
            top.variableOffest.insert(pair(parameter, offest));
        }
        currentStackOffest = offest;
    }
    void EnterBlock() {
        blocks.push_back(CodeGenerateBlock());
    }
    void ExitBlock() {
        auto block = std::move(blocks.back());
        blocks.pop_back();
        auto& top = blocks.back();
        std::move(block.breakActions.begin(), block.breakActions.end(), std::back_inserter(top.breakActions));
        std::move(block.continueActions.begin(), block.continueActions.end(), std::back_inserter(top.continueActions));
    }
    void EnterWhileBlock(int32_t whileBegin) {
        blocks.push_back(CodeGenerateBlock(whileBegin));
    }
    void ExitWhileBlock(int32_t nextPosition) {
        auto& top = blocks.back();
        for (auto& action : top.breakActions) {
            action(nextPosition);
        }
        for (auto& action : top.continueActions) {
            action(top.whileBegin.value());
        }
        blocks.pop_back();
    }
    void RegistBreak(function<void(int32_t)> action) {
        for (auto iter = blocks.rbegin(); iter < blocks.rend(); iter += 1) {
            if (iter->whileBegin.has_value()) {
                iter->breakActions.push_back(std::move(action));
                return;
            }
        }
        throw CompilerError();
    }
    void RegistContinue(function<void(int32_t)> action) {
        for (auto iter = blocks.rbegin(); iter < blocks.rend(); iter += 1) {
            if (iter->whileBegin.has_value()) {
                iter->continueActions.push_back(std::move(action));
                return;
            }
        }
        throw CompilerError();
    }
    void DefineVariable(wstring varIdName) {
        auto& top = blocks.back();
        auto find = top.variableOffest.find(varIdName);
        if (find != top.variableOffest.end()) {
            throw CompilerError();
        }
        top.variableOffest.insert(pair(varIdName, currentStackOffest));
    }
    VariableData GetVariableData(wstring varIdName) {
        for (auto iter = blocks.rbegin(); iter < blocks.rend(); iter += 1) {
            auto find = iter->variableOffest.find(varIdName);
            if (find != iter->variableOffest.end()) {
                return VariableData(false, find->second);
            }
        }
        if (closure.empty()) {
            throw CompilerError();
        }
        auto find = std::find(closure.begin(), closure.end(), varIdName);
        if (find != closure.end()) {
            int16_t offest = static_cast<int16_t>(find - closure.begin());
            return VariableData(true, offest);
        }
        throw CompilerError();
    }
    int16_t MoveOffest(int16_t value) {
        currentStackOffest += value;
        return currentStackOffest;
    };
    int16_t GetOffest() {
        return currentStackOffest;
    }
private:
    int16_t currentStackOffest;
    vector<wstring> closure;
    vector<CodeGenerateBlock> blocks;
};

/*
    根据声明函数来进行递归
*/
class CodeGenerateEnvironment {
public:
    //返回插入位置
    int32_t AddInstruction(Instruction instruction, int line) {
        int32_t index = static_cast<int32_t>(instructions.size());
        instructions.push_back(instruction);
        instructionLines.push_back(line);
        return index;
    }
    Instruction* UpdateInstruction(int32_t index) {
        return &instructions[index];
    }
    int32_t NewInstructionPosition() {
        return static_cast<int32_t>(instructions.size());
    }
    int32_t InsertString(const wstring& str) {
        int32_t index = static_cast<int32_t>(strings.size());
        auto [iter, b] = stringMap.insert(pair(str, index));
        if (b == true) {
            strings.push_back(str);
            return index;
        } else {
            return iter->second;
        }
    }
    void RegistMainClosureOffest(vector<int32_t> mainClosureOffest) {
        this->mainClosureOffest = std::move(mainClosureOffest);
    }
public:
    vector<Instruction> instructions;
    vector<int> instructionLines;
    vector<int32_t> mainClosureOffest;
    vector<wstring> strings;
    map<wstring, int32_t> stringMap;
};

class ExpressionCodeGenerate : public AbstractSyntaxVisitor {
public:
    ExpressionCodeGenerate(CodeGenerateStack& stack, CodeGenerateEnvironment& environment) : stack(stack), environment(environment) {}
    vector<function<void()>> Handle(Expression& type) {
        type.Accept(*this);
        return std::move(handleList);
    }
    void BinaryOperate(BinaryOperation& type, InstructionEnum instructionEnum) {
        vector<function<void()>> resultList;
        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.left);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.right);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        Instruction instruction;
        instruction.type = instructionEnum;
        instruction.offest = stack.MoveOffest(-1);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Multiply& type) override {
        BinaryOperate(type, InstructionEnum::Multiply);
    }
    void Visit(Divide& type) override {
        BinaryOperate(type, InstructionEnum::Divide);
    }
    void Visit(Modulus& type) override {
        BinaryOperate(type, InstructionEnum::Modulus);
    }
    void Visit(Add& type) override {
        BinaryOperate(type, InstructionEnum::Add);
    }
    void Visit(Subtract& type) override {
        BinaryOperate(type, InstructionEnum::Subtract);
    }
    void Visit(Less& type) override {
        BinaryOperate(type, InstructionEnum::Less);
    }
    void Visit(LessEquals& type) override {
        BinaryOperate(type, InstructionEnum::LessEquals);
    }
    void Visit(Greater& type) override {
        BinaryOperate(type, InstructionEnum::Greater);
    }
    void Visit(GreaterEquals& type) override {
        BinaryOperate(type, InstructionEnum::GreaterEquals);
    }
    void Visit(Equals& type) override {
        BinaryOperate(type, InstructionEnum::Equals);
    }
    void Visit(NotEquals& type) override {
        BinaryOperate(type, InstructionEnum::NotEquals);
    }
    void Visit(Or& type) override {
        BinaryOperate(type, InstructionEnum::Or);
    }
    void Visit(And& type) override {
        BinaryOperate(type, InstructionEnum::And);
    }
    //------------------------------
    void UnaryOperate(UnaryOperation& type, InstructionEnum instructionEnum) {
        handleList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        Instruction instruction;
        instruction.type = instructionEnum;
        instruction.offest = stack.GetOffest();
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Not& type) override {
        UnaryOperate(type, InstructionEnum::Not);
    }
    //----------------------------------
    void Visit(SpecialOperationList& type) override;
    void Visit(Null& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::GetNull;
        instruction.offest = stack.MoveOffest(1);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Bool& type) override {

        Instruction instruction;
        if (type.value) {
            instruction.type = InstructionEnum::GetTrue;
        } else {
            instruction.type = InstructionEnum::GetFalse;
        }
        instruction.offest = stack.MoveOffest(1);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Char& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::CreateChar;
        instruction.offest = stack.MoveOffest(1);
        instruction.value.word = type.value;
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Int& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::CreateInt;
        instruction.offest = stack.MoveOffest(1);
        instruction.value.intValue = type.value;
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Float& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::CreateFloat;
        instruction.offest = stack.MoveOffest(1);
        instruction.value.floatValue = type.value;
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(String& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::CreateString;
        instruction.offest = stack.MoveOffest(1);
        instruction.value.intValue = environment.InsertString(type.value);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Array& type) override {
        handleList = ExpressionCodeGenerate(stack, environment).Handle(*type.length);
        Instruction instruction;
        instruction.type = InstructionEnum::CreateArray;
        instruction.offest = stack.GetOffest();
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(Function& type) override;
    void Visit(Object& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::CreateObject;
        instruction.offest = stack.MoveOffest(1);
        environment.AddInstruction(instruction, type.line);
    }
private:
    vector<function<void()>> handleList;
    CodeGenerateStack& stack;
    CodeGenerateEnvironment& environment;
};

class FunctionBlockCodeGenerate : public AbstractSyntaxVisitor {
public:
    FunctionBlockCodeGenerate(CodeGenerateEnvironment& environment) : environment(environment) {}
    void Handle(FunctionBlock& type, const vector<wstring>& idList);
private:
    CodeGenerateStack stack;
    CodeGenerateEnvironment& environment;
};

void ExpressionCodeGenerate::Visit(Function& type) {
    for (auto& idName : type.functionBlock.closure) {
        auto data = stack.GetVariableData(idName);
        Instruction closureItem;
        if (data.closure) {
            closureItem.type = InstructionEnum::GetClosureItemByOffest;
        } else {
            closureItem.type = InstructionEnum::GetVariableByOffest;
        }
        closureItem.offest = stack.MoveOffest(1);
        closureItem.value.offestOrLength = data.offest;
        environment.AddInstruction(closureItem, type.line);
    }

    int16_t closureLength = static_cast<int16_t>(type.functionBlock.closure.size());
    Instruction createClosure;
    createClosure.type = InstructionEnum::CreateClosure;
    createClosure.offest = stack.MoveOffest(1 - closureLength);
    createClosure.value.offestOrLength = closureLength;
    environment.AddInstruction(createClosure, type.line);

    Instruction createFunction;
    createFunction.type = InstructionEnum::Unused;
    createFunction.offest = stack.GetOffest();
    createFunction.reserved = static_cast<int8_t>(type.idList.size());
    int32_t index = environment.AddInstruction(createFunction, type.line);

    auto handle = [&environment = this->environment, &type, index]() {
        auto createFunction = environment.UpdateInstruction(index);
        createFunction->type = InstructionEnum::CreateFunction;
        createFunction->value.intValue = environment.NewInstructionPosition();
        FunctionBlockCodeGenerate(environment).Handle(type.functionBlock, type.idList);
    };
    handleList.push_back(std::move(handle));
}

class SpecialOperationListCodeGenerate : public AbstractSyntaxVisitor {
public:
    SpecialOperationListCodeGenerate(CodeGenerateStack& stack, CodeGenerateEnvironment& environment) : stack(stack), environment(environment) {}
    vector<function<void()>> Handle(SpecialOperationList& type)&& {
        type.Accept(*this);
        return std::move(handleList);
    }
    void Visit(SpecialOperationList& type) override {
        auto variable = stack.GetVariableData(type.id);
        Instruction instruction;
        if (variable.closure == true) {
            instruction.type = InstructionEnum::GetClosureItemByOffest;
        } else {
            instruction.type = InstructionEnum::GetVariableByOffest;
        }
        instruction.offest = stack.MoveOffest(1);
        instruction.value.offestOrLength = variable.offest;
        environment.AddInstruction(instruction, type.line);
        for (auto& item : type.specialOperations) {
            item->Accept(*this);
        }
    }
    void Visit(AccessField& type) override {
        Instruction instruction;
        instruction.type = InstructionEnum::AccessField;
        instruction.offest = stack.GetOffest();
        instruction.value.intValue = environment.InsertString(type.id);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(AccessArray& type) override {
        auto resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.index);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        Instruction instruction;
        instruction.type = InstructionEnum::AccessArray;
        instruction.offest = stack.MoveOffest(-1);
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(FunctionCall& type) override {
        for (int i = 0; i < 3; i++) {
            Instruction instruction;
            instruction.type = InstructionEnum::GetNull;
            instruction.offest = stack.MoveOffest(1);
            environment.AddInstruction(instruction, type.line);
        }
        for (auto& item : type.expressionList) {
            auto resultList = ExpressionCodeGenerate(stack, environment).Handle(*item);
            std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        }
        int16_t parameterCount = static_cast<int16_t>(type.expressionList.size());
        int16_t functionCallPosition = -parameterCount - 3;

        Instruction instruction;
        instruction.type = InstructionEnum::FunctionCall;
        instruction.offest = stack.MoveOffest(functionCallPosition);
        instruction.value.offestOrLength = parameterCount;
        environment.AddInstruction(instruction, type.line);
    }
private:
    vector<function<void()>> handleList;
    CodeGenerateStack& stack;
    CodeGenerateEnvironment& environment;
};

void ExpressionCodeGenerate::Visit(SpecialOperationList& type) {
    handleList = SpecialOperationListCodeGenerate(stack, environment).Handle(type);
}

class DefaultBlockCodeGenerate : public AbstractSyntaxVisitor {
public:
    DefaultBlockCodeGenerate(CodeGenerateStack& stack, CodeGenerateEnvironment& environment) : stack(stack), environment(environment) {}
    vector<function<void()>> Handle(DefaultBlock& type)&& {
        type.Accept(*this);
        return std::move(handleList);
    }
    void Visit(DefaultBlock& type) override;
private:
    vector<function<void()>> handleList;
    CodeGenerateStack& stack;
    CodeGenerateEnvironment& environment;
};

class StatementCodeGenerate : public AbstractSyntaxVisitor {
public:
    StatementCodeGenerate(CodeGenerateStack& stack, CodeGenerateEnvironment& environment) : stack(stack), environment(environment) {}
    vector<function<void()>> Handle(Statement& type) {
        type.Accept(*this);
        return std::move(handleList);
    }
    void Visit(StatementDefineVariable& type) override {
        auto resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        stack.DefineVariable(type.id);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
    }

    void Visit(StatementDefineFunction& type) override {
        optional<int16_t> selfOffest;
        int16_t closureItemOffest = 0;
        for (auto& idName : type.functionBlock.closure) {
            if (idName == type.id) {
                Instruction nullType;
                nullType.type = InstructionEnum::GetNull;
                nullType.offest = stack.MoveOffest(1);
                selfOffest = closureItemOffest;
                environment.AddInstruction(nullType, type.line);
            } else {
                auto data = stack.GetVariableData(idName);
                Instruction closureItem;
                if (data.closure) {
                    closureItem.type = InstructionEnum::GetClosureItemByOffest;
                } else {
                    closureItem.type = InstructionEnum::GetVariableByOffest;
                }
                closureItem.offest = stack.MoveOffest(1);
                closureItem.value.offestOrLength = data.offest;
                environment.AddInstruction(closureItem, type.line);
            }
            closureItemOffest += 1;
        }

        int16_t closureLength = static_cast<int16_t>(type.functionBlock.closure.size());
        Instruction createClosure;
        createClosure.type = InstructionEnum::CreateClosure;
        createClosure.offest = stack.MoveOffest(1 - closureLength);
        createClosure.value.offestOrLength = closureLength;
        environment.AddInstruction(createClosure, type.line);

        Instruction createFunction;
        createFunction.type = InstructionEnum::Unused;
        createFunction.reserved = static_cast<int8_t>(type.idList.size());
        createFunction.offest = stack.GetOffest();
        int32_t index = environment.AddInstruction(createFunction, type.line);

        if (selfOffest.has_value()) {
            Instruction addRecursiveFunctionItem;
            addRecursiveFunctionItem.type = InstructionEnum::AddRecursiveFunctionItem;
            addRecursiveFunctionItem.offest = stack.GetOffest();
            addRecursiveFunctionItem.value.offestOrLength = selfOffest.value();
            environment.AddInstruction(addRecursiveFunctionItem, type.line);
        }

        stack.DefineVariable(type.id);

        auto handle = [&environment = this->environment, &type, index]() {
            auto createFunction = environment.UpdateInstruction(index);
            createFunction->type = InstructionEnum::CreateFunction;
            createFunction->value.intValue = environment.NewInstructionPosition();
            FunctionBlockCodeGenerate(environment).Handle(type.functionBlock, type.idList);
        };
        handleList.push_back(std::move(handle));
    }
    void Visit(StatementAssignmentId& type) override {
        auto resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        auto data = stack.GetVariableData(type.id);
        Instruction instruction;
        if (data.closure) {
            instruction.type = InstructionEnum::SetClosureItemByOffest;
        } else {
            instruction.type = InstructionEnum::SetVariableByOffest;
        }
        instruction.offest = stack.GetOffest();
        instruction.value.offestOrLength = data.offest;
        environment.AddInstruction(instruction, type.line);
    }
    void Visit(StatementAssignmentArray& type) override {
        vector<function<void()>> resultList;
        resultList = SpecialOperationListCodeGenerate(stack, environment).Handle(*type.specialOperationList);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.index);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        Instruction assignmentArray;
        assignmentArray.type = InstructionEnum::AssignmentArray;
        assignmentArray.offest = stack.MoveOffest(-2);
        environment.AddInstruction(assignmentArray, type.line);
    }
    void Visit(StatementAssignmentField& type) override {
        vector<function<void()>> resultList;
        resultList = SpecialOperationListCodeGenerate(stack, environment).Handle(*type.specialOperationList);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        Instruction assignmentField;
        assignmentField.type = InstructionEnum::AssignmentField;
        assignmentField.offest = stack.MoveOffest(-1);
        assignmentField.value.intValue = environment.InsertString(type.field);
        environment.AddInstruction(assignmentField, type.line);
    }
    void Visit(StatementCall& type) override {
        auto resultList = SpecialOperationListCodeGenerate(stack, environment).Handle(*type.specialOperationList);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        stack.MoveOffest(-1);
    }
    void Visit(StatementIf& type) override {
        vector<function<void()>> resultList;

        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.condition);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        Instruction conditionJump;
        conditionJump.offest = stack.GetOffest();
        int32_t conditionJumpIndex = environment.AddInstruction(conditionJump, type.line);

        resultList = DefaultBlockCodeGenerate(stack, environment).Handle(type.elseBlock);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        Instruction jump;
        jump.offest = stack.GetOffest();
        int32_t jumpIndex = environment.AddInstruction(jump, type.line);

        auto pConditionJump = environment.UpdateInstruction(conditionJumpIndex);
        pConditionJump->type = InstructionEnum::ConditionJump;
        pConditionJump->value.intValue = environment.NewInstructionPosition();

        resultList = DefaultBlockCodeGenerate(stack, environment).Handle(type.ifBlock);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        auto pJump = environment.UpdateInstruction(jumpIndex);
        pJump->type = InstructionEnum::Jump;
        pJump->value.intValue = environment.NewInstructionPosition();

    }
    void Visit(StatementWhile& type) override {
        stack.EnterWhileBlock(environment.NewInstructionPosition());

        Instruction jump;
        jump.offest = stack.GetOffest();

        int32_t jumpPosition = environment.AddInstruction(jump, type.line);
        int32_t conditionJumpIndex = environment.NewInstructionPosition();

        vector<function<void()>> resultList;
        for (auto& item : type.whileBlock.statements) {
            resultList = StatementCodeGenerate(stack, environment).Handle(*item);
            std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        }

        auto pJump = environment.UpdateInstruction(jumpPosition);
        pJump->type = InstructionEnum::Jump;
        pJump->value.intValue = environment.NewInstructionPosition();

        resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.condition);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));

        Instruction conditionJump;
        conditionJump.type = InstructionEnum::ConditionJump;
        conditionJump.offest = stack.GetOffest();
        conditionJump.value.intValue = conditionJumpIndex;
        environment.AddInstruction(conditionJump, type.line);

        stack.ExitWhileBlock(environment.NewInstructionPosition());
    }
    void Visit(StatementBreak& type) override {
        Instruction jump;
        jump.type = InstructionEnum::Jump;
        jump.offest = stack.GetOffest();
        int32_t index = environment.AddInstruction(jump, type.line);
        stack.RegistBreak([&environment = this->environment, index](int32_t newPosition) {
            environment.UpdateInstruction(index)->value.intValue = newPosition;
        });
    }
    void Visit(StatementContinue& type) override {
        Instruction jump;
        jump.type = InstructionEnum::Jump;
        jump.offest = stack.GetOffest();
        int32_t index = environment.AddInstruction(jump, type.line);
        stack.RegistContinue([&environment = this->environment, index](int32_t whileBegin) {
            environment.UpdateInstruction(index)->value.intValue = whileBegin;
        });
    }
    void Visit(StatementReturn& type) override {
        auto resultList = ExpressionCodeGenerate(stack, environment).Handle(*type.expression);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        Instruction ret;
        ret.type = InstructionEnum::Return;
        ret.offest = stack.GetOffest();
        environment.AddInstruction(ret, type.line);
    }

public:
    vector<function<void()>> handleList;
    CodeGenerateStack& stack;
    CodeGenerateEnvironment& environment;
};

void DefaultBlockCodeGenerate::Visit(DefaultBlock& type) {
    stack.EnterBlock();
    for (auto& item : type.statements) {
        auto resultList = StatementCodeGenerate(stack, environment).Handle(*item);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
    }
    stack.ExitBlock();
}

void FunctionBlockCodeGenerate::Handle(FunctionBlock& type, const vector<wstring>& idList) {
    vector<wstring> closure;
    std::copy(type.closure.begin(), type.closure.end(), std::back_inserter(closure));
    stack = CodeGenerateStack(closure, idList);

    vector<function<void()>> handleList;
    for (auto& item : type.statements) {
        auto resultList = StatementCodeGenerate(stack, environment).Handle(*item);
        std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
    }
    for (auto& handle : handleList) {
        handle();
    }
}

class MainBlockCodeGenerate : public AbstractSyntaxVisitor {
public:
    CodeGenerateEnvironment Handle(MainBlock& type, const RegisteredNameList& nameList)&& {
        vector<wstring> closure;
        std::copy(type.closure.begin(), type.closure.end(), std::back_inserter(closure));
        stack = CodeGenerateStack(closure, vector<wstring>());

        //closure 是集合  registered是数组 
        //可能会导致顺序不相同
        vector<int> mainClosureOffest;
        for (auto& closureItem : closure) {
            for (int i = 0; i < nameList.registeredNames.size(); i++) {
                if (closureItem == nameList.registeredNames[i]) {
                    mainClosureOffest.push_back(i);
                    break;
                }
            }

        }
        environment.RegistMainClosureOffest(std::move(mainClosureOffest));


        vector<function<void()>> handleList;
        for (auto& item : type.statements) {
            auto resultList = StatementCodeGenerate(stack, environment).Handle(*item);
            std::move(resultList.begin(), resultList.end(), std::back_inserter(handleList));
        }
        for (auto& handle : handleList) {
            handle();
        }
        return std::move(environment);
    }
private:
    CodeGenerateStack stack;
    CodeGenerateEnvironment environment;
};

VMRuntimeData CreateVMRuntimeData(const RegisteredNameList& nameList, AbstractSyntaxTreeTransform&& abstractSyntaxTree) {
    CodeGenerateEnvironment environment = MainBlockCodeGenerate().Handle(abstractSyntaxTree.root, nameList);
    VMRuntimeData data;
    data.registeredNames = nameList.registeredNames;
    data.instruction = std::move(environment.instructions);
    data.instructionLine = std::move(environment.instructionLines);
    data.staticString = std::move(environment.strings);
    data.mainClosureOffest = std::move(environment.mainClosureOffest);
    data.stringMap = std::move(environment.stringMap);
    return std::move(data);
}