#pragma once
#include "CodeGenerate.h"
#include<functional>
#include<map>
#include<string_view>
using std::wstring_view;
using std::map;
using std::function;

struct VirtualMachine;

struct VirtualMachineBuilder {
public:
    VirtualMachineBuilder(VMRuntimeData data);
    void RegistLocalFunction(const wstring& idName, function<int32_t(VirtualMachine*, int16_t)> localFunction);
    void SetStackMax(int32_t value);
    void SetHeapMax(int32_t value);
    VirtualMachine Build();
private:
    int32_t stackMax;
    int32_t heapMax;
    vector<Instruction> instruction;
    vector<int> instructionLine;
    vector<wstring> registeredNames;
    vector<wstring> staticString;
    vector<int> mainClosureOffest;
    map<wstring, int32_t> stringMap;
    vector<function<int32_t(VirtualMachine*, int16_t)>> localFunctionList;
};

void VirtualMachineInit(VirtualMachine& virtualMachine);
void VirtualMachineReset(VirtualMachine& virtualMachine);
void VirtualMachineStart(VirtualMachine& virtualMachine);
void VirtualMachineGC(VirtualMachine& VirtualMachine);

/*
   用于注册本地函数时方便操作 在进行需要分配内存的操作后 需要重新从栈获取指针
*/
int16_t VMLocalFunctionGetArraySize(VirtualMachine& vm, HeapType* heapPointer);
int16_t VMLocalFunctionGetObjectFieldSize(VirtualMachine& vm, HeapType* heapPointer);
wchar_t VMLocalFunctionGetChar(VirtualMachine& vm, HeapType* heapPointer);
float VMLocalFunctionGetFloat(VirtualMachine& vm, HeapType* heapPointer);
int32_t VMLocalFunctionGetInt(VirtualMachine& vm, HeapType* heapPointer);
HeapType* VMLocalFunctionAccessArray(VirtualMachine& vm, HeapType* heapPointer, int16_t index);
HeapEnum VMLocalFunctionGetType(VirtualMachine& vm, HeapType* heapPointer);
wstring_view VMLocalFunctionGetStringData(VirtualMachine& vm, HeapType* str);
HeapType* VMLocalFunctionGetParameter(VirtualMachine& vm, int16_t parameterCount, int16_t parameterIndex);


int32_t VMBoolToHeapPointer(bool v);
int32_t VMNullToHeapPointer();
void VMProgramCounterInc(VirtualMachine& vm);
void VMSetUpNewOffest(VirtualMachine& vm, int16_t offest);
Instruction* VMProgramMemory(VirtualMachine& vm);
StackType* VMStackMemoryByOffest(VirtualMachine& vm, int16_t offest);
StackType* VMStackMemory(VirtualMachine& vm);
HeapType* VMHeapMemory(VirtualMachine& vm, int32_t heapPointer);
int VMCurrentProgramLine(VirtualMachine& vm);
int32_t VMAllocateHeapMemory(VirtualMachine& vm, HeapEnum type, int16_t value);
void VMCheckStackOverflow(VirtualMachine& vm, int16_t offest);

void VMGetNull(VirtualMachine& vm, int16_t offest);
void VMGetFalse(VirtualMachine& vm, int16_t offest);
void VMGetTrue(VirtualMachine& vm, int16_t offest);
void VMCreateChar(VirtualMachine& vm, int16_t offest, wchar_t value);
void VMCreateInt(VirtualMachine& vm, int16_t offest, int32_t value);
void VMCreateFloat(VirtualMachine& vm, int16_t offest, float value);
void VMCreateString(VirtualMachine& vm, int16_t offest, int32_t index);
void VMCreateArray(VirtualMachine& vm, int16_t offest);
void VMCreateObject(VirtualMachine& vm, int16_t offest);
void VMCreateClosure(VirtualMachine& vm, int16_t offest, int16_t length);
void VMCreateFunction(VirtualMachine& vm, int16_t offest, int32_t programPointer, int8_t parameterCount);
void VMAddRecursiveFunctionItem(VirtualMachine& vm, int16_t offest, int16_t closureItemOffest);
void VMGetVariableByOffest(VirtualMachine& vm, int16_t offest, int16_t variableOffest);
void VMSetVariableByOffest(VirtualMachine& vm, int16_t offest, int16_t variableOffest);
void VMGetClosureItemByOffest(VirtualMachine& vm, int16_t offest, int16_t closureOffest);
void VMSetClosureItemByOffest(VirtualMachine& vm, int16_t offest, int16_t closureOffest);
void VMAccessArray(VirtualMachine& vm, int16_t offest);
void VMAccessField(VirtualMachine& vm, int16_t offest, int32_t index);
void VMAssignmentArray(VirtualMachine& vm, int16_t offest);
void VMAssignmentField(VirtualMachine& vm, int16_t offest, int32_t index);
void VMFunctionCall(VirtualMachine& vm, int16_t offest, int16_t parameterCount);
void VMJump(VirtualMachine& vm, int16_t offest, int32_t program);
void VMConditionJump(VirtualMachine& vm, int16_t offest, int32_t program);
void VMReturn(VirtualMachine& vm, int16_t offest);

void VMBinaryOperation(VirtualMachine& vm, int16_t offest, InstructionEnum op);
void VMCompareOperation(VirtualMachine& vm, int16_t offest, InstructionEnum op);
void VMNot(VirtualMachine& vm, int16_t offest);
void VMEquals(VirtualMachine& vm, int16_t offest);
void VMNotEquals(VirtualMachine& vm, int16_t offest);

struct OperationKey {
    inline OperationKey(InstructionEnum op, HeapEnum left, HeapEnum right) : op(op), left(left), right(right) {}
    InstructionEnum op;
    HeapEnum left;
    HeapEnum right;
};
inline bool operator==(const OperationKey& l, const OperationKey& r) {
    return std::tie(l.op, l.left, l.right) == std::tie(r.op, r.left, r.right);
}
inline bool operator<(const OperationKey& l, const OperationKey& r) {
    return std::tie(l.op, l.left, l.right) < std::tie(r.op, r.left, r.right);
}

struct EqualsKey {
    inline EqualsKey(HeapEnum left, HeapEnum right) : left(left), right(right) {}
    HeapEnum left;
    HeapEnum right;
};
inline bool operator==(const EqualsKey& l, const EqualsKey& r) {
    return std::tie(l.left, l.right) == std::tie(r.left, r.right);
}
inline bool operator<(const EqualsKey& l, const EqualsKey& r) {
    return std::tie(l.left, l.right) < std::tie(r.left, r.right);
}

struct VirtualMachine {
    vector<Instruction> program;
    vector<StackType> stack;
    vector<HeapType> heap;
    int32_t programCounter = 0;
    int32_t stackPointer = 0;
    int32_t stackOffest = 0;
    int32_t heapOffest = 0;

    vector<function<int32_t(VirtualMachine* vm, int16_t parameterCount)>> localFunctionList;
    vector<wstring> StaticString;
    vector<int> instructionLine;
    map<wstring, int32_t> stringMap;
    map<OperationKey, function<int32_t(VirtualMachine& vm, HeapType*, HeapType*)>> operationMap;
    map<OperationKey, function<bool(VirtualMachine& vm, HeapType*, HeapType*)>> compareMap;
    map<EqualsKey, function<bool(VirtualMachine& vm, HeapType*, HeapType*)>> equalsMap;
};