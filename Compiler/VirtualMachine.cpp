#include "VirtualMachine.h"
#include "CompilerException.h"
#include <optional>
using std::optional;
using std::pair;
int32_t stackMin = 1024;
int32_t heapMin = 4096;
int8_t recycleMark = 0b00000001;
int8_t neverRecycleMark = 0b00000010;

VirtualMachineBuilder::VirtualMachineBuilder(VMRuntimeData data)
    : stackMax(stackMin), heapMax(heapMin), registeredNames(std::move(data.registeredNames)),
    instruction(std::move(data.instruction)), instructionLine(std::move(data.instructionLine)),
    staticString(std::move(data.staticString)), mainClosureOffest(std::move(data.mainClosureOffest)),
    stringMap(std::move(data.stringMap)) {
    auto list = vector<function<int32_t(VirtualMachine*, int16_t parameterCount)>>(
        registeredNames.size(), [](VirtualMachine* vm, int16_t parameterCount) -> int32_t {
        throw ConfigurationException(MessageHead(vm->instructionLine[vm->programCounter]) + "本地函数尚未注册");
    });
    localFunctionList = std::move(list);
}

void VirtualMachineBuilder::RegistLocalFunction(const wstring& idName, function<int32_t(VirtualMachine*, int16_t)> localFunction) {
    auto find = std::find(registeredNames.begin(), registeredNames.end(), idName);
    if (find == registeredNames.end()) {
        throw ConfigurationException(WstringToString(idName) + "注册表中不存在");
    }
    size_t index = find - registeredNames.begin();
    localFunctionList[index] = std::move(localFunction);
}

void VirtualMachineBuilder::SetStackMax(int32_t value) {
    if (value < stackMin) {
        throw ConfigurationException("栈大小 最小值为:" + std::to_string(stackMin));
    }
    stackMax = value;
}

void VirtualMachineBuilder::SetHeapMax(int32_t value) {
    if (value < heapMin) {
        throw ConfigurationException("堆大小 最小值为:" + std::to_string(heapMin));
    }
    heapMax = value;
}

int32_t VMStringCreate(VirtualMachine& vm, const wchar_t* data1, int16_t length1, const wchar_t* data2, int16_t length2) {
    int16_t length = (length1 + length2);
    int16_t memoryLength = 2 + (length + 1) / 2;
    int32_t heapPointerString = VMAllocateHeapMemory(vm, HeapEnum::String, memoryLength);
    auto heapPointerStringPtr = VMHeapMemory(vm, heapPointerString);
    heapPointerStringPtr[1].value.stringLengthOrIndex.type = StringDataType::Length;
    heapPointerStringPtr[1].value.stringLengthOrIndex.lengthOrIndex = length;
    wchar_t* charPtr = reinterpret_cast<wchar_t*>(heapPointerStringPtr + 2);
    int16_t index = 0;
    while (index < length1) {
        charPtr[index] = data1[index];
        index += 1;
    }
    int16_t index2 = 0;
    while (index2 < length2) {
        charPtr[length1 + index2] = data2[index2];
        index2 += 1;
    }
    return heapPointerString;
}

wstring_view VMStringGet(VirtualMachine& vm, HeapType* str) {
    auto type = str[1].value.stringLengthOrIndex.type;
    if (type == StringDataType::Index) {
        int16_t index = str[1].value.stringLengthOrIndex.lengthOrIndex;
        return wstring_view(vm.StaticString[index]);
    } else if (type == StringDataType::Length) {
        int16_t length = str[1].value.stringLengthOrIndex.lengthOrIndex;
        wchar_t* charPtr = reinterpret_cast<wchar_t*>(str + 2);
        return wstring_view(charPtr, length);
    } else {
        throw CompilerError();
    }
}

StackType* VMLocalFunctionParamete(VirtualMachine& vm, int16_t parameterCount) {
    return VMStackMemory(vm) + 1 - parameterCount;
}

int16_t VMLocalFunctionGetArraySize(VirtualMachine& vm, HeapType* heapPointer) {
    return heapPointer[1].value.length;
}

int16_t VMLocalFunctionGetObjectFieldSize(VirtualMachine& vm, HeapType* heapPointer) {
    int32_t heapPointerObjectFieldList = heapPointer[1].value.intValue;
    auto heapPointerObjectFieldListPtr = VMHeapMemory(vm, heapPointerObjectFieldList);
    return heapPointerObjectFieldListPtr[1].value.length;
}

wchar_t VMLocalFunctionGetChar(VirtualMachine& vm, HeapType* heapPointer) {
    return heapPointer[1].value.word[0];
}

float VMLocalFunctionGetFloat(VirtualMachine& vm, HeapType* heapPointer) {
    return heapPointer[1].value.floatValue;
}

int32_t VMLocalFunctionGetInt(VirtualMachine& vm, HeapType* heapPointer) {
    return heapPointer[1].value.intValue;
}

HeapType* VMLocalFunctionAccessArray(VirtualMachine& vm, HeapType* heapPointer, int16_t index) {
    auto heapPointerItem = heapPointer + 2;
    return VMHeapMemory(vm, heapPointerItem[index].value.intValue);
}

HeapEnum VMLocalFunctionGetType(VirtualMachine& vm, HeapType* heapPointer) {
    return heapPointer->value.typeHead.type;
}

wstring_view VMLocalFunctionGetStringData(VirtualMachine& vm, HeapType* str) {
    auto type = str[1].value.stringLengthOrIndex.type;
    auto lengthOrIndex = str[1].value.stringLengthOrIndex.lengthOrIndex;
    if (type == StringDataType::Index) {
        auto index = lengthOrIndex;
        return wstring_view(vm.StaticString[index]);
    } else if (type == StringDataType::Length) {
        auto length = lengthOrIndex;
        wchar_t* data = &str[2].value.word[0];
        return wstring_view(data, length);
    } else {
        throw CompilerError();
    }
}

HeapType* VMLocalFunctionGetParameter(VirtualMachine& vm, int16_t parameterCount, int16_t parameterIndex) {
    auto stackPointer = VMStackMemory(vm) - parameterCount + 1 + parameterIndex;
    return VMHeapMemory(vm, stackPointer->intValue);
}

int32_t VMBoolToHeapPointer(bool v) {
    if (v) {
        return 2;
    } else {
        return 1;
    }
}

int32_t VMNullToHeapPointer() {
    return 0;
}

void VMProgramCounterInc(VirtualMachine& vm) {
    vm.programCounter += 1;
}

VirtualMachine VirtualMachineBuilder::Build() {
    map<OperationKey, function<int32_t(VirtualMachine& vm, HeapType*, HeapType*)>> opMap;
    opMap.insert(pair(OperationKey(InstructionEnum::Multiply, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t value = left[1].value.intValue * right[1].value.intValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.intValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Multiply, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = static_cast<float>(left[1].value.intValue) * right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Multiply, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue * static_cast<float>(right[1].value.intValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Multiply, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue * right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));

    opMap.insert(pair(OperationKey(InstructionEnum::Divide, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t value = left[1].value.intValue / right[1].value.intValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.intValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Divide, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = static_cast<float>(left[1].value.intValue) / right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Divide, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue / static_cast<float>(right[1].value.intValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Divide, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue / right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));

    opMap.insert(pair(OperationKey(InstructionEnum::Modulus, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t value = left[1].value.intValue % right[1].value.intValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.intValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Modulus, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = fmodf(static_cast<float>(left[1].value.intValue), right[1].value.floatValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Modulus, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = fmodf(left[1].value.floatValue, static_cast<float>(right[1].value.intValue));
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Modulus, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = fmodf(left[1].value.floatValue, right[1].value.floatValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));

    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t value = left[1].value.intValue + right[1].value.intValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.intValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = static_cast<float>(left[1].value.intValue) + right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue + static_cast<float>(right[1].value.intValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue + right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));

    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Char, HeapEnum::Char), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        auto leftValue = left[1].value.word[0];
        auto rightValue = right[1].value.word[0];
        int32_t heapPointerResult = VMStringCreate(vm, &leftValue, 1, &rightValue, 1);
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::Char, HeapEnum::String), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        wchar_t value = left[1].value.word[0];
        auto strV = VMStringGet(vm, right);
        int32_t heapPointerResult = VMStringCreate(vm, &value, 1, strV.data(), static_cast<int16_t>(strV.size()));
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::String, HeapEnum::Char), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        auto strV = VMStringGet(vm, left);
        wchar_t value = right[1].value.word[0];
        int32_t heapPointerResult = VMStringCreate(vm, strV.data(), static_cast<int16_t>(strV.size()), &value, 1);
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Add, HeapEnum::String, HeapEnum::String), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        auto strVLeft = VMStringGet(vm, left);
        auto strVRight = VMStringGet(vm, right);
        int32_t heapPointerResult =
            VMStringCreate(vm, strVLeft.data(), static_cast<int16_t>(strVLeft.size()), strVRight.data(), static_cast<int16_t>(strVRight.size()));
        return heapPointerResult;
    }));

    opMap.insert(pair(OperationKey(InstructionEnum::Subtract, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t value = left[1].value.intValue - right[1].value.intValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.intValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Subtract, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = static_cast<float>(left[1].value.intValue) - right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Subtract, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue - static_cast<float>(right[1].value.intValue);
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));
    opMap.insert(pair(OperationKey(InstructionEnum::Subtract, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float value = left[1].value.floatValue - right[1].value.floatValue;
        int32_t heapPointerResult = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
        VMHeapMemory(vm, heapPointerResult)[1].value.floatValue = value;
        return heapPointerResult;
    }));

    map<OperationKey, function<bool(VirtualMachine& vm, HeapType*, HeapType*)>> compareMap;
    compareMap.insert(pair(OperationKey(InstructionEnum::Less, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.intValue < right[1].value.intValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Less, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (static_cast<float>(left[1].value.intValue) < right[1].value.floatValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Less, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue < static_cast<float>(right[1].value.intValue));
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Less, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue < right[1].value.floatValue);
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::LessEquals, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.intValue <= right[1].value.intValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::LessEquals, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (static_cast<float>(left[1].value.intValue) <= right[1].value.floatValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::LessEquals, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue <= static_cast<float>(right[1].value.intValue));
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::LessEquals, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue <= right[1].value.floatValue);
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::Greater, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.intValue > right[1].value.intValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Greater, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (static_cast<float>(left[1].value.intValue) > right[1].value.floatValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Greater, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue > static_cast<float>(right[1].value.intValue));
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Greater, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue > right[1].value.floatValue);
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::GreaterEquals, HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.intValue >= right[1].value.intValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::GreaterEquals, HeapEnum::Int, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (static_cast<float>(left[1].value.intValue) >= right[1].value.floatValue);
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::GreaterEquals, HeapEnum::Float, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue >= static_cast<float>(right[1].value.intValue));
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::GreaterEquals, HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return (left[1].value.floatValue >= right[1].value.floatValue);
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::True, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::True, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::False, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::False, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::And, HeapEnum::True, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::And, HeapEnum::True, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::And, HeapEnum::False, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::And, HeapEnum::False, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));

    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::True, HeapEnum::Nothing), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));
    compareMap.insert(pair(OperationKey(InstructionEnum::Or, HeapEnum::False, HeapEnum::Nothing), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));

    map<EqualsKey, function<bool(VirtualMachine& vm, HeapType*, HeapType*)>> eqMap;
    eqMap.insert(pair(EqualsKey(HeapEnum::Null, HeapEnum::Null), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::False, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::False, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::True, HeapEnum::False), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return false;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::True, HeapEnum::True), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return true;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Int, HeapEnum::Int), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        int32_t valueLeft = left[1].value.intValue;
        int32_t valueRight = right[1].value.intValue;
        return valueLeft == valueRight;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Float, HeapEnum::Float), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        float valueLeft = left[1].value.floatValue;
        float valueRight = right[1].value.floatValue;
        return valueLeft == valueRight;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Char, HeapEnum::Char), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        wchar_t valueLeft = left[1].value.word[0];
        wchar_t valueRight = right[1].value.word[0];
        return valueLeft == valueRight;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::String, HeapEnum::String), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        bool b = (VMStringGet(vm, left) == VMStringGet(vm, right));
        return b;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Object, HeapEnum::Object), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return left == right;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Function, HeapEnum::Function), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        return left == right;
    }));
    eqMap.insert(pair(EqualsKey(HeapEnum::Array, HeapEnum::Array), [](VirtualMachine& vm, HeapType* left, HeapType* right) ->int32_t {
        if (left == right) {
            return true;
        }
        if (left[1].value.length != left[1].value.length) {
            return false;
        }
        int16_t length = left[1].value.length;
        auto leftData = left + 2;
        auto rightData = right + 2;
        for (int16_t i = 0; i < length; i += 1) {
            auto heapPointerLeftPtr = VMHeapMemory(vm, leftData->value.intValue);
            auto heapPointerRightPtr = VMHeapMemory(vm, rightData->value.intValue);
            auto find = vm.equalsMap.find(EqualsKey(heapPointerLeftPtr->value.typeHead.type, heapPointerRightPtr->value.typeHead.type));
            if (find == vm.equalsMap.end()) {
                return false;
            }
            bool result = find->second(vm, heapPointerLeftPtr, heapPointerRightPtr);
            if (result == false) {
                return false;
            }
        }
        return true;
    }));
    //==================================
    VirtualMachine virtualMachine;
    virtualMachine.program = std::move(instruction);
    virtualMachine.heap = vector<HeapType>(heapMax);
    virtualMachine.stack = vector<StackType>(stackMax);
    virtualMachine.localFunctionList.reserve(localFunctionList.size());
    for (auto offest : mainClosureOffest) {
        virtualMachine.localFunctionList.push_back(localFunctionList[offest]);
    }
    virtualMachine.StaticString = std::move(staticString);
    virtualMachine.instructionLine = std::move(instructionLine);
    virtualMachine.operationMap = std::move(opMap);
    virtualMachine.equalsMap = std::move(eqMap);
    virtualMachine.compareMap = std::move(compareMap);
    virtualMachine.stringMap = std::move(stringMap);
    return virtualMachine;
}

//------------------------------------------------------------

void VirtualMachineInit(VirtualMachine& virtualMachine) {
    int32_t lengthNullFalseTrue = 3;
    int32_t lengthMainClosureItem = static_cast<int32_t>(virtualMachine.localFunctionList.size()) * 2;
    int32_t lengthMainClosure = 2 + static_cast<int32_t>(virtualMachine.localFunctionList.size());
    int32_t lengthMainFunction = 3;
    int32_t lengthTotal = lengthNullFalseTrue + lengthMainClosureItem + lengthMainClosure + lengthMainFunction;

    int32_t heapSize = static_cast<int32_t>(virtualMachine.heap.size());
    if (heapSize <= lengthTotal) {
        throw ConfigurationException("堆太小了,不满足最基本的运行时条件");
    }

    VirtualMachineReset(virtualMachine);


    HeapType nullType;
    nullType.value.typeHead.type = HeapEnum::Null;
    nullType.value.typeHead.reserved = neverRecycleMark;
    nullType.value.typeHead.memorylength = 1;
    virtualMachine.heap[0] = nullType;
    HeapType falseType;
    falseType.value.typeHead.type = HeapEnum::False;
    falseType.value.typeHead.reserved = neverRecycleMark;
    falseType.value.typeHead.memorylength = 1;
    virtualMachine.heap[1] = falseType;
    HeapType trueType;
    trueType.value.typeHead.type = HeapEnum::True;
    trueType.value.typeHead.reserved = neverRecycleMark;
    trueType.value.typeHead.memorylength = 1;
    virtualMachine.heap[2] = trueType;

    const int32_t heapPositionStart = 3;
    const int16_t localFunctionItemClosureMemoryLength = 2;
    const int16_t heapTypeHeadAndLength = 2;
    const int16_t localFunctionCounts = static_cast<int16_t>(virtualMachine.localFunctionList.size());

    int32_t heapPosition = heapPositionStart;
    for (int16_t i = 0; i < localFunctionCounts; i++) {
        HeapType localFunctionItem;
        localFunctionItem.value.typeHead.type = HeapEnum::LocalFunction;
        localFunctionItem.value.typeHead.reserved = neverRecycleMark;
        localFunctionItem.value.typeHead.memorylength = localFunctionItemClosureMemoryLength;
        virtualMachine.heap[heapPosition] = localFunctionItem;
        heapPosition += 1;

        virtualMachine.heap[heapPosition].value.intValue = i;
        heapPosition += 1;
    }

    HeapType localFuntionClosure;
    localFuntionClosure.value.typeHead.type = HeapEnum::Closure;
    localFuntionClosure.value.typeHead.reserved = neverRecycleMark;
    localFuntionClosure.value.typeHead.memorylength = heapTypeHeadAndLength + localFunctionCounts;
    virtualMachine.heap[heapPosition] = localFuntionClosure;
    heapPosition += 1;

    virtualMachine.heap[heapPosition].value.length = localFunctionCounts;
    heapPosition += 1;

    for (int16_t i = 0; i < localFunctionCounts; i++) {
        const int localFunctionItemPosition = heapPositionStart + localFunctionItemClosureMemoryLength * i;
        virtualMachine.heap[heapPosition].value.intValue = localFunctionItemPosition;
        heapPosition += 1;
    }

    const int16_t heapTypeHeadAndClosurePositionAndProgramPosition = 4;
    const int32_t stackbuttom = heapPosition;
    HeapType heapMainFunction;
    heapMainFunction.value.typeHead.type = HeapEnum::Function;
    heapMainFunction.value.typeHead.reserved = neverRecycleMark;
    heapMainFunction.value.typeHead.memorylength = heapTypeHeadAndClosurePositionAndProgramPosition;
    virtualMachine.heap[heapPosition] = heapMainFunction;
    heapPosition += 1;

    virtualMachine.heap[heapPosition].value.length = 0;
    heapPosition += 1;

    const int closurePosition = heapPositionStart + localFunctionItemClosureMemoryLength * localFunctionCounts;
    virtualMachine.heap[heapPosition].value.intValue = closurePosition;
    heapPosition += 1;

    virtualMachine.heap[heapPosition].value.intValue = 0;
    heapPosition += 1;

    virtualMachine.stack[0].intValue = stackbuttom;
    virtualMachine.heapOffest = heapPosition;
}

void VirtualMachineReset(VirtualMachine& virtualMachine) {
    for (auto& item : virtualMachine.stack) {
        item = StackType();
    }
    for (auto& item : virtualMachine.heap) {
        item = HeapType();
    }

    virtualMachine.programCounter = 0;
    virtualMachine.stackPointer = 0;
    virtualMachine.stackOffest = 0;
    virtualMachine.heapOffest = 0;
}

void VirtualMachineStart(VirtualMachine& virtualMachine) {
    VMFunctionCall(virtualMachine, 0, 0);
    while (virtualMachine.stackPointer != 0) {
        Instruction instruction = virtualMachine.program[virtualMachine.programCounter];
        InstructionEnum type = instruction.type;
        int16_t offest = instruction.offest;
        VMCheckStackOverflow(virtualMachine, offest);
        switch (type) {
            case InstructionEnum::Unused:
                throw CompilerError();
            case InstructionEnum::GetNull:
                VMGetNull(virtualMachine, offest);
                break;
            case InstructionEnum::GetFalse:
                VMGetFalse(virtualMachine, offest);
                break;
            case InstructionEnum::GetTrue:
                VMGetTrue(virtualMachine, offest);
                break;
            case InstructionEnum::CreateChar:
                VMCreateChar(virtualMachine, offest, instruction.value.word);
                break;
            case InstructionEnum::CreateInt:
                VMCreateInt(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::CreateFloat:
                VMCreateFloat(virtualMachine, offest, instruction.value.floatValue);
                break;
            case InstructionEnum::CreateString:
                VMCreateString(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::CreateArray:
                VMCreateArray(virtualMachine, offest);
                break;
            case InstructionEnum::CreateObject:
                VMCreateObject(virtualMachine, offest);
                break;
            case InstructionEnum::CreateClosure:
                VMCreateClosure(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::CreateFunction:
                VMCreateFunction(virtualMachine, offest, instruction.value.intValue, instruction.reserved);
                break;
            case InstructionEnum::AddRecursiveFunctionItem:
                VMAddRecursiveFunctionItem(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::GetVariableByOffest:
                VMGetVariableByOffest(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::SetVariableByOffest:
                VMSetVariableByOffest(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::GetClosureItemByOffest:
                VMGetClosureItemByOffest(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::SetClosureItemByOffest:
                VMSetClosureItemByOffest(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::AccessArray:
                VMAccessArray(virtualMachine, offest);
                break;
            case InstructionEnum::AccessField:
                VMAccessField(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::AssignmentArray:
                VMAssignmentArray(virtualMachine, offest);
                break;
            case InstructionEnum::AssignmentField:
                VMAssignmentField(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::FunctionCall:
                VMFunctionCall(virtualMachine, offest, instruction.value.offestOrLength);
                break;
            case InstructionEnum::Jump:
                VMJump(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::ConditionJump:
                VMConditionJump(virtualMachine, offest, instruction.value.intValue);
                break;
            case InstructionEnum::Return:
                VMReturn(virtualMachine, offest);
                break;
            case InstructionEnum::Multiply:
            case InstructionEnum::Divide:
            case InstructionEnum::Modulus:
            case InstructionEnum::Add:
            case InstructionEnum::Subtract:
                VMBinaryOperation(virtualMachine, offest, type);
                break;
            case InstructionEnum::Less:
            case InstructionEnum::LessEquals:
            case InstructionEnum::Greater:
            case InstructionEnum::GreaterEquals:
                VMCompareOperation(virtualMachine, offest, type);
                break;
            case InstructionEnum::Equals:
                VMEquals(virtualMachine, offest);
                break;
            case InstructionEnum::NotEquals:
                VMNotEquals(virtualMachine, offest);
                break;
            case InstructionEnum::Or:
            case InstructionEnum::And:
                VMCompareOperation(virtualMachine, offest, type);
                break;
            case InstructionEnum::Not:
                VMNot(virtualMachine, offest);
                break;
            default:
                throw CompilerError();
                break;
        }
    }
}

void VMGCRecursiveMark(VirtualMachine& vm, int32_t heapPointer) {
    auto ptr = VMHeapMemory(vm, heapPointer);
    auto& bit = ptr->value.typeHead.reserved;
    if ((bit & recycleMark) == recycleMark) {
        return;
    }
    bit |= recycleMark;
    auto type = ptr->value.typeHead.type;
    switch (type) {
        case HeapEnum::Nothing:
            throw CompilerError();
        case HeapEnum::Null:
        case HeapEnum::False:
        case HeapEnum::True:
        case HeapEnum::Char:
        case HeapEnum::Int:
        case HeapEnum::Float:
        case HeapEnum::String:
        case HeapEnum::LocalFunction:
            break;
        case HeapEnum::Array:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                VMGCRecursiveMark(vm, itemPtr[i].value.intValue);
            }
            break;
        }
        case HeapEnum::Object:
        {
            VMGCRecursiveMark(vm, ptr[1].value.intValue);
            break;
        }
        case HeapEnum::ObjectFieldList:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                VMGCRecursiveMark(vm, itemPtr[i * 2 + 1].value.intValue);
            }
            break;
        }
        case HeapEnum::Closure:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                VMGCRecursiveMark(vm, itemPtr[i].value.intValue);
            }
            break;
        }
        case HeapEnum::Function:
            VMGCRecursiveMark(vm, ptr[2].value.intValue);
            break;
        default:
            throw CompilerError();
    }
}

void VMGCRecursiveClearMark(VirtualMachine& vm, int32_t heapPointer, const map<int32_t, optional<int32_t>>& pointerMap) {
    //之前标记过垃圾位 使用该位 作为是否改变指针的依据
    //改变后清除该位
    auto ptr = VMHeapMemory(vm, heapPointer);
    auto type = ptr->value.typeHead.type;
    auto& bit = ptr->value.typeHead.reserved;
    if ((bit & recycleMark) != recycleMark) {
        return;
    }
    bit &= (~recycleMark);
    switch (type) {
        case HeapEnum::Nothing:
            throw CompilerError();
        case HeapEnum::Null:
        case HeapEnum::False:
        case HeapEnum::True:
        case HeapEnum::Char:
        case HeapEnum::Int:
        case HeapEnum::Float:
        case HeapEnum::String:
        case HeapEnum::LocalFunction:
            break;
        case HeapEnum::Array:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                auto oldHeapPointer = itemPtr[i].value.intValue;
                auto newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
                itemPtr[i].value.intValue = newHeapPointer;
                VMGCRecursiveClearMark(vm, newHeapPointer, pointerMap);
            }
            break;
        }
        case HeapEnum::Object:
        {
            auto oldHeapPointer = ptr[1].value.intValue;
            auto newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
            ptr[1].value.intValue = newHeapPointer;
            VMGCRecursiveClearMark(vm, newHeapPointer, pointerMap);
            break;
        }
        case HeapEnum::ObjectFieldList:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                int32_t offest = i * 2 + 1;
                auto oldHeapPointer = ptr[offest].value.intValue;
                auto newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
                ptr[offest].value.intValue = newHeapPointer;
                VMGCRecursiveClearMark(vm, newHeapPointer, pointerMap);
            }
            break;
        }
        case HeapEnum::Closure:
        {
            int16_t length = ptr[1].value.length;
            auto itemPtr = ptr + 2;
            for (int16_t i = 0; i < length; i++) {
                auto oldHeapPointer = itemPtr[i].value.intValue;
                auto newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
                itemPtr[i].value.intValue = newHeapPointer;
                VMGCRecursiveClearMark(vm, newHeapPointer, pointerMap);
            }
            break;
        }
        case HeapEnum::Function:
        {

            auto oldHeapPointer = ptr[2].value.intValue;
            auto newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
            ptr[2].value.intValue = newHeapPointer;
            VMGCRecursiveClearMark(vm, newHeapPointer, pointerMap);
            break;
        }
        default:
            throw CompilerError();
    }
}

void VirtualMachineGC(VirtualMachine& virtualMachine) {
    const int32_t pushStackOffest = 2;
    //根标记
    {
        int32_t stackPointer = virtualMachine.stackPointer;
        int32_t stackOffest = virtualMachine.stackOffest;
        while (stackPointer != 0) {
            while (stackOffest >= pushStackOffest) {
                int32_t stackPosition = stackPointer + stackOffest;
                int32_t heapPointer = virtualMachine.stack[stackPosition].intValue;
                VMGCRecursiveMark(virtualMachine, heapPointer);
                stackOffest -= 1;
            }
            int32_t newStackPointer = virtualMachine.stack[stackPointer].intValue;
            stackOffest = stackPointer - newStackPointer - 1;
            stackPointer = newStackPointer;
        }
    }
    //计算前后指针位置
    map<int32_t, optional<int32_t>> pointerMap;
    const int32_t heapLastOffest = virtualMachine.heapOffest;
    int32_t newHeapOffest = 0;
    {
        int32_t oldHeapOffest = 0;
        while (oldHeapOffest < heapLastOffest) {
            auto heapPtr = VMHeapMemory(virtualMachine, oldHeapOffest);
            auto& bit = heapPtr->value.typeHead.reserved;
            if ((bit & neverRecycleMark) == neverRecycleMark) {
                bit |= recycleMark;
            }
            int16_t memoryLength = heapPtr->value.typeHead.memorylength;
            if ((bit & recycleMark) == recycleMark) {
                pointerMap.insert(pair(oldHeapOffest, optional<int32_t>(newHeapOffest)));
                newHeapOffest += memoryLength;
            } else {
                pointerMap.insert(pair(oldHeapOffest, optional<int32_t>()));
            }
            oldHeapOffest += memoryLength;
        }
    }
    //转移
    {
        int32_t oldHeapOffest = 0;
        while (oldHeapOffest != heapLastOffest) {
            auto oldHeapPtr = VMHeapMemory(virtualMachine, oldHeapOffest);
            int16_t memoryLength = oldHeapPtr->value.typeHead.memorylength;
            auto newPosition = pointerMap.find(oldHeapOffest)->second;
            if (newPosition.has_value()) {
                int16_t newHeapOffest = newPosition.value();
                auto newHeapPtr = VMHeapMemory(virtualMachine, newHeapOffest);
                for (int16_t i = 0; i < memoryLength; i++) {
                    newHeapPtr[i].value.intValue = oldHeapPtr[i].value.intValue;
                }
            }
            oldHeapOffest += memoryLength;
        }
    }
    //递归清除标记 并且将内存里的指针更改为新指针
    {
        int32_t stackPointer = virtualMachine.stackPointer;
        int32_t stackOffest = virtualMachine.stackOffest;
        while (stackPointer != 0) {
            while (stackOffest >= pushStackOffest) {
                int32_t stackPosition = stackPointer + stackOffest;
                int32_t oldHeapPointer = virtualMachine.stack[stackPosition].intValue;
                int32_t newHeapPointer = pointerMap.find(oldHeapPointer)->second.value();
                virtualMachine.stack[stackPosition].intValue = newHeapPointer;
                VMGCRecursiveClearMark(virtualMachine, newHeapPointer, pointerMap);
                stackOffest -= 1;
            }
            int32_t newStackPointer = virtualMachine.stack[stackPointer].intValue;
            stackOffest = stackPointer - newStackPointer - 1;
            stackPointer = newStackPointer;
        }
    }
    //转移堆
    virtualMachine.heapOffest = newHeapOffest;
}

//----------------------------------------------------

void VMSetUpNewOffest(VirtualMachine& vm, int16_t offest) {
    vm.stackOffest = offest;
}

Instruction* VMProgramMemory(VirtualMachine& vm) {
    return &vm.program[vm.programCounter];
}

StackType* VMStackMemoryByOffest(VirtualMachine& vm, int16_t offest) {
    int64_t v = static_cast<int64_t>(vm.stackPointer) + static_cast<int64_t>(offest);
    return &vm.stack[v];
}

StackType* VMStackMemory(VirtualMachine& vm) {
    int64_t v = static_cast<int64_t>(vm.stackPointer) + static_cast<int64_t>(vm.stackOffest);
    return &vm.stack[v];
}

HeapType* VMHeapMemory(VirtualMachine& vm, int32_t heapPointer) {
    return &vm.heap[heapPointer];
}

int VMCurrentProgramLine(VirtualMachine& vm) {
    return vm.instructionLine[vm.programCounter];
}

int32_t VMAllocateHeapMemory(VirtualMachine& vm, HeapEnum type, int16_t value) {
    if (value <= 0) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "内存分配数值必须在 [1 , 2^15 - 1]之间");
    }
    int32_t heapSize = static_cast<int32_t>(vm.heap.size());
    if (vm.heapOffest + value >= heapSize) {
        VirtualMachineGC(vm);
        if (vm.heapOffest + value >= heapSize) {
            throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "内存超过最大上限");
        }
    }
    int32_t position = vm.heapOffest;
    auto* ptr = VMHeapMemory(vm, position);
    ptr->value.typeHead.type = type;
    ptr->value.typeHead.memorylength = value;
    vm.heapOffest += value;
    return position;
}

void VMCheckStackOverflow(VirtualMachine& vm, int16_t offest) {
    int32_t sp = vm.stackPointer + offest;
    int32_t stackMax = static_cast<int32_t>(vm.stack.size());
    if (sp >= stackMax) {
        throw RuntimeException("栈溢出");
    }
}

void VMGetNull(VirtualMachine& vm, int16_t offest) {
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMNullToHeapPointer();
    VMProgramCounterInc(vm);
}

void VMGetFalse(VirtualMachine& vm, int16_t offest) {
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMBoolToHeapPointer(false);
    VMProgramCounterInc(vm);
}

void VMGetTrue(VirtualMachine& vm, int16_t offest) {
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMBoolToHeapPointer(true);
    VMProgramCounterInc(vm);
}

void VMCreateChar(VirtualMachine& vm, int16_t offest, wchar_t value) {
    int32_t heapPointer = VMAllocateHeapMemory(vm, HeapEnum::Char, 2);
    VMHeapMemory(vm, heapPointer + 1)->value.word[0] = value;
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointer;
    VMProgramCounterInc(vm);
}

void VMCreateInt(VirtualMachine& vm, int16_t offest, int32_t value) {
    int32_t heapPointer = VMAllocateHeapMemory(vm, HeapEnum::Int, 2);
    VMHeapMemory(vm, heapPointer + 1)->value.intValue = value;
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointer;
    VMProgramCounterInc(vm);
}

void VMCreateFloat(VirtualMachine& vm, int16_t offest, float value) {
    int32_t heapPointer = VMAllocateHeapMemory(vm, HeapEnum::Float, 2);
    VMHeapMemory(vm, heapPointer + 1)->value.floatValue = value;
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointer;
    VMProgramCounterInc(vm);
}

void VMCreateString(VirtualMachine& vm, int16_t offest, int32_t index) {
    int32_t heapPointer = VMAllocateHeapMemory(vm, HeapEnum::String, 2);
    auto heapPointerStringDataType = VMHeapMemory(vm, heapPointer + 1);
    heapPointerStringDataType->value.stringLengthOrIndex.type = StringDataType::Index;
    heapPointerStringDataType->value.stringLengthOrIndex.lengthOrIndex = static_cast<int16_t>(index);
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointer;
    VMProgramCounterInc(vm);
}

void VMCreateArray(VirtualMachine& vm, int16_t offest) {
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerLength = VMStackMemory(vm)->intValue;
    auto heapPointerLengthPtr = VMHeapMemory(vm, heapPointerLength);
    if (heapPointerLengthPtr->value.typeHead.type != HeapEnum::Int) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组长度不为int类型");
    }
    int32_t arrayLength = heapPointerLengthPtr[1].value.intValue;
    if (arrayLength <= 0 || arrayLength > INT16_MAX) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组长度必须在[1 , 2^15 - 1]之间");
    }
    int16_t memoryArrayLength = static_cast<int16_t>(arrayLength);
    int16_t memoryRealLength = memoryArrayLength + 2;
    int32_t heapPointerArray = VMAllocateHeapMemory(vm, HeapEnum::Array, memoryRealLength);
    VMStackMemory(vm)->intValue = heapPointerArray;
    auto heapPointerArrayPtr = VMHeapMemory(vm, heapPointerArray);

    heapPointerArrayPtr[1].value.length = memoryArrayLength;

    const int16_t memoryStartOffest = 2;
    for (int16_t i = 0; i != memoryArrayLength; i += 1) {
        heapPointerArrayPtr[memoryStartOffest + i].value.intValue = 0;
    }
    VMProgramCounterInc(vm);
}

void VMCreateObject(VirtualMachine& vm, int16_t offest) {
    const int16_t headAndOtherData = 2;
    const int16_t objectFieldListInitCapacity = 16;
    const int16_t objectFieldListMemory = headAndOtherData + objectFieldListInitCapacity * headAndOtherData;
    int32_t heapPointerObjectFieldList = VMAllocateHeapMemory(vm, HeapEnum::ObjectFieldList, objectFieldListMemory);
    auto heapPointerObjectFieldListPtr = VMHeapMemory(vm, heapPointerObjectFieldList);
    heapPointerObjectFieldListPtr[1].value.length = 0;
    //容器机制先分配内存 不初始化内存 用到了在初始化

    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointerObjectFieldList;
    int32_t heapPointerObject = VMAllocateHeapMemory(vm, HeapEnum::Object, headAndOtherData);
    auto heapPointerObjectPtr = VMHeapMemory(vm, heapPointerObject);
    heapPointerObjectPtr[1].value.intValue = heapPointerObjectFieldList;
    VMStackMemory(vm)->intValue = heapPointerObject;
    VMProgramCounterInc(vm);
}

void VMCreateClosure(VirtualMachine& vm, int16_t offest, int16_t length) {
    const int16_t headAndOtherData = 2;
    const int16_t memoryLength = headAndOtherData + length;
    int32_t heapPointerClosure = VMAllocateHeapMemory(vm, HeapEnum::Closure, memoryLength);
    auto heapPointerClosurePtr = VMHeapMemory(vm, heapPointerClosure);
    heapPointerClosurePtr[1].value.length = length;

    VMSetUpNewOffest(vm, offest);

    int32_t heapPointerClosureItemStart = heapPointerClosure + headAndOtherData;
    auto* heapPointerClosureItemStartPtr = VMHeapMemory(vm, heapPointerClosureItemStart);
    for (int16_t i = 0; i != length; i += 1) {
        heapPointerClosureItemStartPtr[i].value.intValue = VMStackMemory(vm)[i].intValue;
    }
    VMStackMemory(vm)->intValue = heapPointerClosure;
    VMProgramCounterInc(vm);
}

void VMCreateFunction(VirtualMachine& vm, int16_t offest, int32_t programPointer, int8_t parameterCount) {
    const int16_t functionMemeoryLength = 4;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerFunction = VMAllocateHeapMemory(vm, HeapEnum::Function, functionMemeoryLength);
    int32_t heapPointerClosure = VMStackMemory(vm)->intValue;
    auto heapPointerFunctionPtr = VMHeapMemory(vm, heapPointerFunction);
    heapPointerFunctionPtr[1].value.length = parameterCount;
    heapPointerFunctionPtr[2].value.intValue = heapPointerClosure;
    heapPointerFunctionPtr[3].value.intValue = programPointer;
    VMStackMemory(vm)->intValue = heapPointerFunction;
    VMProgramCounterInc(vm);
}

void VMAddRecursiveFunctionItem(VirtualMachine& vm, int16_t offest, int16_t closureItemOffest) {
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerFunction = VMStackMemory(vm)->intValue;
    auto heapPointerFunctionPtr = VMHeapMemory(vm, heapPointerFunction);
    int32_t heapPointerClosure = heapPointerFunctionPtr[2].value.intValue;
    auto heapPointerClosurePtr = VMHeapMemory(vm, heapPointerClosure);
    auto heapPointerClosureItemPtr = heapPointerClosurePtr + 2;
    heapPointerClosureItemPtr[closureItemOffest].value.intValue = heapPointerFunction;
    VMProgramCounterInc(vm);
}

void VMGetVariableByOffest(VirtualMachine& vm, int16_t offest, int16_t variableOffest) {
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMStackMemoryByOffest(vm, variableOffest)->intValue;
    VMProgramCounterInc(vm);
}

void VMSetVariableByOffest(VirtualMachine& vm, int16_t offest, int16_t variableOffest) {
    VMSetUpNewOffest(vm, offest);
    VMStackMemoryByOffest(vm, variableOffest)->intValue = VMStackMemory(vm)->intValue;
    VMProgramCounterInc(vm);
}

void VMGetClosureItemByOffest(VirtualMachine& vm, int16_t offest, int16_t closureOffest) {
    const int16_t stackOffestClosure = 2;
    const int16_t head = 2;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerClosure = VMStackMemoryByOffest(vm, stackOffestClosure)->intValue;
    auto heapPointerClosurePtr = VMHeapMemory(vm, heapPointerClosure);
    auto heapPointerClosureItem = heapPointerClosurePtr + head;
    VMStackMemory(vm)->intValue = heapPointerClosureItem[closureOffest].value.intValue;
    VMProgramCounterInc(vm);
}

void VMSetClosureItemByOffest(VirtualMachine& vm, int16_t offest, int16_t closureOffest) {
    const int16_t stackOffestClosure = 2;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerClosure = VMStackMemoryByOffest(vm, stackOffestClosure)->intValue;
    auto heapPointerClosurePtr = VMHeapMemory(vm, heapPointerClosure);
    auto heapPointerClosureItemPtr = heapPointerClosurePtr + 2;
    heapPointerClosureItemPtr[closureOffest].value.intValue = VMStackMemory(vm)->intValue;
    VMProgramCounterInc(vm);
}

void VMAccessArray(VirtualMachine& vm, int16_t offest) {
    const int32_t typeHeadAndOther = 2;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerArray = VMStackMemory(vm)->intValue;
    auto heapPointerArrayPtr = VMHeapMemory(vm, heapPointerArray);
    if (heapPointerArrayPtr->value.typeHead.type != HeapEnum::Array) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "访问类型不为array");
    }

    int32_t heapPointerIndex = VMStackMemory(vm)[1].intValue;
    auto heapPointerIndexPtr = VMHeapMemory(vm, heapPointerIndex);
    if (heapPointerIndexPtr->value.typeHead.type != HeapEnum::Int) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组访问索引类型必须为int");
    }
    int32_t length = heapPointerArrayPtr[1].value.length;
    int32_t index = heapPointerIndexPtr[1].value.intValue;
    if ((index < 0) || (index >= length)) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组访问越界");
    }
    auto heapPointerArrayItemPtr = heapPointerArrayPtr + typeHeadAndOther;
    VMStackMemory(vm)->intValue = heapPointerArrayItemPtr[index].value.intValue;
    VMProgramCounterInc(vm);
}

void VMAccessField(VirtualMachine& vm, int16_t offest, int32_t index) {
    const int32_t typeHeadAndOther = 2;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerObject = VMStackMemory(vm)->intValue;
    auto heapPointerObjectPtr = VMHeapMemory(vm, heapPointerObject);
    if (heapPointerObjectPtr->value.typeHead.type != HeapEnum::Object) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "访问类型不为Object");
    }

    int32_t heapPointerObjectFieldList = heapPointerObjectPtr[1].value.intValue;
    auto heapPointerObjectFieldListPtr = VMHeapMemory(vm, heapPointerObjectFieldList);
    int16_t fieldListSize = heapPointerObjectFieldListPtr[1].value.length;
    auto heapPointerObjectFieldItemPtr = heapPointerObjectFieldListPtr + typeHeadAndOther;

    //TODO 后面改成二分查找
    int16_t arrayIndex = 0;
    while (arrayIndex != fieldListSize) {
        int16_t fieldOffest = arrayIndex * 2;
        int32_t stringIndex = heapPointerObjectFieldItemPtr[fieldOffest].value.intValue;
        int32_t fieldValue = heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue;
        if (stringIndex < index) {
            arrayIndex += 1;
        } else if (stringIndex == index) {
            VMStackMemory(vm)->intValue = fieldValue;
            VMProgramCounterInc(vm);
            return;
        } else {
            break;
        }
    }
    throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "object中不存在 id: " + WstringToString(vm.StaticString[index]));
}

void VMAssignmentArray(VirtualMachine& vm, int16_t offest) {
    const int32_t typeHeadAndOther = 2;
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerArray = VMStackMemory(vm)->intValue;
    auto heapPointerArrayPtr = VMHeapMemory(vm, heapPointerArray);
    if (heapPointerArrayPtr->value.typeHead.type != HeapEnum::Array) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "访问类型不为array");
    }

    int32_t heapPointerIndex = VMStackMemory(vm)[1].intValue;
    auto heapPointerIndexPtr = VMHeapMemory(vm, heapPointerIndex);
    if (heapPointerIndexPtr->value.typeHead.type != HeapEnum::Int) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组访问索引类型必须为int");
    }
    int32_t length = heapPointerArrayPtr[1].value.length;
    int32_t index = heapPointerIndexPtr[1].value.intValue;
    if ((index < 0) || (index >= length)) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "数组访问越界");
    }

    int32_t heapPointerExpression = VMStackMemory(vm)[2].intValue;
    auto heapPointerArrayItemPtr = heapPointerArrayPtr + typeHeadAndOther;
    heapPointerArrayItemPtr[index].value.intValue = heapPointerExpression;
    VMProgramCounterInc(vm);
}

void VMAssignmentField(VirtualMachine& vm, int16_t offest, int32_t index) {
    const int16_t typeHeadAndOther = 2;
    const int16_t objectFieldListItemLength = 2;
    int32_t heapPointerObject = VMStackMemoryByOffest(vm, offest)->intValue;
    auto heapPointerObjectPtr = VMHeapMemory(vm, heapPointerObject);
    if (heapPointerObjectPtr->value.typeHead.type != HeapEnum::Object) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "访问类型不为Object");
    }

    int32_t heapPointerExpression = VMStackMemoryByOffest(vm, offest + 1)->intValue;
    int32_t heapPointerObjectFieldList = heapPointerObjectPtr[1].value.intValue;
    auto heapPointerObjectFieldListPtr = VMHeapMemory(vm, heapPointerObjectFieldList);
    int16_t fieldListSize = heapPointerObjectFieldListPtr[1].value.length;
    auto heapPointerObjectFieldItemPtr = heapPointerObjectFieldListPtr + typeHeadAndOther;

    //TODO 后面改成二分查找 
    int16_t arrayIndex = 0;
    while (arrayIndex != fieldListSize) {
        int16_t fieldOffest = arrayIndex * objectFieldListItemLength;
        int32_t stringIndex = heapPointerObjectFieldItemPtr[fieldOffest].value.intValue;
        if (stringIndex < index) {
            arrayIndex += 1;
        } else if (stringIndex == index) {
            heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue = heapPointerExpression;
            VMSetUpNewOffest(vm, offest);
            VMProgramCounterInc(vm);
            return;
        } else {
            break;
        }
    }

    //插入

    int16_t prevousCapacity = heapPointerObjectFieldListPtr->value.typeHead.memorylength;
    int16_t prevousLength = heapPointerObjectFieldListPtr[1].value.length;
    bool objectFieldListIsFull = (prevousCapacity == (prevousLength * objectFieldListItemLength + typeHeadAndOther));
    if (objectFieldListIsFull) {
        const int16_t addtion = 16;
        int16_t currentCapacity = prevousCapacity + addtion * objectFieldListItemLength;
        int16_t currentLength = prevousLength + 1;

        int32_t heapPointerNewObjectFieldList = VMAllocateHeapMemory(vm, HeapEnum::ObjectFieldList, currentCapacity);
        auto heapPointerNewObjectFieldListPtr = VMHeapMemory(vm, heapPointerNewObjectFieldList);
        heapPointerNewObjectFieldListPtr[1].value.length = currentLength;
        auto heapPointerNewObjectFieldItemPtr = heapPointerNewObjectFieldListPtr + typeHeadAndOther;

        int16_t copyIndex = 0;
        while (copyIndex != arrayIndex) {
            int16_t fieldOffest = copyIndex * objectFieldListItemLength;
            heapPointerNewObjectFieldItemPtr[fieldOffest].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest].value.intValue;
            heapPointerNewObjectFieldItemPtr[fieldOffest + 1].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue;
            copyIndex += 1;
        }

        int16_t insertOffest = arrayIndex * 2;
        heapPointerNewObjectFieldItemPtr[insertOffest].value.intValue = index;
        heapPointerNewObjectFieldItemPtr[insertOffest + 1].value.intValue = heapPointerExpression;

        while (copyIndex != prevousLength) {
            int16_t fieldOffest = copyIndex * objectFieldListItemLength;
            int16_t fieldToOffest = fieldOffest + objectFieldListItemLength;
            heapPointerNewObjectFieldItemPtr[fieldToOffest].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest].value.intValue;
            heapPointerNewObjectFieldItemPtr[fieldToOffest + 1].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue;
            copyIndex += 1;
        }

        heapPointerObjectPtr[1].value.intValue = heapPointerNewObjectFieldList;
    } else {
        int16_t currentLength = prevousLength + 1;
        heapPointerObjectFieldListPtr[1].value.length = currentLength;;
        //倒插
        int16_t copyIndex = prevousLength;
        while (copyIndex != arrayIndex) {
            int16_t fieldOffest = copyIndex * 2 - 2;
            int16_t fieldToOffest = copyIndex * 2;
            heapPointerObjectFieldItemPtr[fieldToOffest].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest].value.intValue;
            heapPointerObjectFieldItemPtr[fieldToOffest + 1].value.intValue = heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue;
            copyIndex -= 1;
        }
        int16_t fieldOffest = arrayIndex * 2;
        heapPointerObjectFieldItemPtr[fieldOffest].value.intValue = index;
        heapPointerObjectFieldItemPtr[fieldOffest + 1].value.intValue = heapPointerExpression;
    }
    VMSetUpNewOffest(vm, offest);
    VMProgramCounterInc(vm);
}

void VMFunctionCall(VirtualMachine& vm, int16_t offest, int16_t parameterCount) {
    int32_t heapPointerFunction = VMStackMemoryByOffest(vm, offest)->intValue;
    auto heapPointerFunctionPtr = VMHeapMemory(vm, heapPointerFunction);
    auto type = heapPointerFunctionPtr->value.typeHead.type;
    if (type == HeapEnum::LocalFunction) {
        int32_t localFunctionIndex = heapPointerFunctionPtr[1].value.intValue;
        VMStackMemoryByOffest(vm, offest)->intValue = vm.localFunctionList[localFunctionIndex](&vm, parameterCount);
        VMSetUpNewOffest(vm, offest);
        VMProgramCounterInc(vm);
    } else if (type == HeapEnum::Function) {
        int16_t functionParameterCount = heapPointerFunctionPtr[1].value.length;
        if (functionParameterCount != parameterCount) {
            throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "函数参数个数不正确");
        }
        const int32_t stackPointerAndProgramCounterAndClosureOffest = 3;

        int32_t stackPointer = vm.stackPointer;
        int32_t programCounter = vm.programCounter + 1;
        int32_t heapPointerClosure = heapPointerFunctionPtr[2].value.intValue;

        int32_t newStackPointer = vm.stackPointer + offest + 1;
        int32_t newProgramCounter = heapPointerFunctionPtr[3].value.intValue;

        vm.stackPointer = newStackPointer;
        vm.programCounter = newProgramCounter;
        vm.stackOffest = stackPointerAndProgramCounterAndClosureOffest + functionParameterCount;
        VMStackMemoryByOffest(vm, 0)->intValue = stackPointer;
        VMStackMemoryByOffest(vm, 1)->intValue = programCounter;
        VMStackMemoryByOffest(vm, 2)->intValue = heapPointerClosure;
    } else {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "访问类型不为 Function 或 LocalFuntion");
    }
}

void VMJump(VirtualMachine& vm, int16_t offest, int32_t program) {
    VMSetUpNewOffest(vm, offest);
    vm.programCounter = program;
}

void VMConditionJump(VirtualMachine& vm, int16_t offest, int32_t program) {
    VMSetUpNewOffest(vm, offest);
    int32_t heapPointerCondition = VMStackMemory(vm)->intValue;
    auto heapPointerConditionPtr = VMHeapMemory(vm, heapPointerCondition);
    auto type = heapPointerConditionPtr->value.typeHead.type;
    if (type == HeapEnum::True) {
        vm.programCounter = program;
    } else if (type == HeapEnum::False) {
        VMProgramCounterInc(vm);
    } else {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "条件类型不为 bool");
    }
}

void VMReturn(VirtualMachine& vm, int16_t offest) {
    VMSetUpNewOffest(vm, offest);
    int32_t returnValue = VMStackMemory(vm)->intValue;
    int32_t stackPointer = VMStackMemoryByOffest(vm, 0)->intValue;
    int32_t programCounter = VMStackMemoryByOffest(vm, 1)->intValue;
    int32_t stackOffest = vm.stackPointer - stackPointer - 1;
    vm.stackPointer = stackPointer;
    vm.programCounter = programCounter;
    vm.stackOffest = stackOffest;
    VMStackMemory(vm)->intValue = returnValue;
}

void VMBinaryOperation(VirtualMachine& vm, int16_t offest, InstructionEnum op) {
    int32_t heapPointerLeft = VMStackMemoryByOffest(vm, offest)->intValue;
    int32_t heapPointerRight = VMStackMemoryByOffest(vm, offest + 1)->intValue;
    auto left = VMHeapMemory(vm, heapPointerLeft);
    auto right = VMHeapMemory(vm, heapPointerRight);
    auto leftType = left->value.typeHead.type;
    auto rightType = right->value.typeHead.type;

    auto find = vm.operationMap.find(OperationKey(op, leftType, rightType));
    if (find == vm.operationMap.end()) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "二元操作符 左右类型不正确");
    }
    int32_t heapPointerResult = find->second(vm, left, right);
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = heapPointerResult;
    VMProgramCounterInc(vm);
}

void VMCompareOperation(VirtualMachine& vm, int16_t offest, InstructionEnum op) {
    int32_t heapPointerLeft = VMStackMemoryByOffest(vm, offest)->intValue;
    int32_t heapPointerRight = VMStackMemoryByOffest(vm, offest + 1)->intValue;
    auto left = VMHeapMemory(vm, heapPointerLeft);
    auto right = VMHeapMemory(vm, heapPointerRight);
    auto leftType = left->value.typeHead.type;
    auto rightType = right->value.typeHead.type;

    auto find = vm.compareMap.find(OperationKey(op, leftType, rightType));
    if (find == vm.compareMap.end()) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "比较操作符 左右类型不正确");
    }
    bool result = find->second(vm, left, right);
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMBoolToHeapPointer(result);
    VMProgramCounterInc(vm);
}

void VMNot(VirtualMachine& vm, int16_t offest) {
    int32_t heapPointer = VMStackMemoryByOffest(vm, offest)->intValue;
    auto heapPointerPtr = VMHeapMemory(vm, heapPointer);
    auto type = heapPointerPtr->value.typeHead.type;
    auto find = vm.compareMap.find(OperationKey(InstructionEnum::Not, type, HeapEnum::Nothing));
    if (find == vm.compareMap.end()) {
        throw RuntimeException(MessageHead(VMCurrentProgramLine(vm)) + "非运算符 运算类型不正确");
    }
    bool result = find->second(vm, heapPointerPtr, nullptr);
    VMSetUpNewOffest(vm, offest);
    VMStackMemory(vm)->intValue = VMBoolToHeapPointer(result);
    VMProgramCounterInc(vm);
}


void VMEquals(VirtualMachine& vm, int16_t offest) {
    int32_t heapPointerLeft = VMStackMemoryByOffest(vm, offest)->intValue;
    int32_t heapPointerRight = VMStackMemoryByOffest(vm, offest + 1)->intValue;
    auto left = VMHeapMemory(vm, heapPointerLeft);
    auto right = VMHeapMemory(vm, heapPointerRight);
    auto leftType = left->value.typeHead.type;
    auto rightType = right->value.typeHead.type;

    VMSetUpNewOffest(vm, offest);
    auto find = vm.equalsMap.find(EqualsKey(leftType, rightType));
    if (find == vm.equalsMap.end()) {
        VMStackMemory(vm)->intValue = VMBoolToHeapPointer(false);
    } else {
        bool result = find->second(vm, left, right);
        if (result) {
            VMStackMemory(vm)->intValue = VMBoolToHeapPointer(true);
        } else {
            VMStackMemory(vm)->intValue = VMBoolToHeapPointer(false);
        }
    }
    VMProgramCounterInc(vm);
}

void VMNotEquals(VirtualMachine& vm, int16_t offest) {
    int32_t heapPointerLeft = VMStackMemoryByOffest(vm, offest)->intValue;
    int32_t heapPointerRight = VMStackMemoryByOffest(vm, offest + 1)->intValue;
    auto left = VMHeapMemory(vm, heapPointerLeft);
    auto right = VMHeapMemory(vm, heapPointerRight);
    auto leftType = left->value.typeHead.type;
    auto rightType = right->value.typeHead.type;

    VMSetUpNewOffest(vm, offest);
    auto find = vm.equalsMap.find(EqualsKey(leftType, rightType));
    if (find == vm.equalsMap.end()) {
        VMStackMemory(vm)->intValue = VMBoolToHeapPointer(true);
    } else {
        bool result = find->second(vm, left, right);
        if (result) {
            VMStackMemory(vm)->intValue = VMBoolToHeapPointer(false);
        } else {
            VMStackMemory(vm)->intValue = VMBoolToHeapPointer(true);
        }
    }
    VMProgramCounterInc(vm);
}