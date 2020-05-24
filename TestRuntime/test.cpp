#include "pch.h"
#include "Complie.h"
#include"VirtualMachine.h"

static auto compileData = CompileData();



TEST(VirtualMachine, RegistFunction1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1();";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 0);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, RegistFunction2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1();";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    EXPECT_THROW(VirtualMachineStart(vm), ConfigurationException);
}

TEST(VirtualMachine, RegistFunction3) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(false);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    EXPECT_THROW(builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMNullToHeapPointer();
    }), ConfigurationException);
}

TEST(VirtualMachine, False) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(false);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, True) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(true);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Char) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1('C');";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto word = VMLocalFunctionGetChar(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::Char);
        EXPECT_EQ(word, L'C');
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Int) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(12345);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 12345);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Float) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(1.5);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(L"reg1(1.5);", compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetFloat(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::Float);
        EXPECT_EQ(value, 1.5f);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, StringIndex) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(\"asd\");";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto view = VMLocalFunctionGetStringData(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::String);
        EXPECT_EQ(view, L"asd");
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, StringLength) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"reg1(\"asd\" + 'f' + 'g');";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto view = VMLocalFunctionGetStringData(*vm, heapPointer);
        EXPECT_EQ(parameterCount, 1);
        EXPECT_EQ(type, HeapEnum::String);
        EXPECT_EQ(view, L"asdfg");
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Array) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"var arr = array[3]; arr[0] = 1; arr[1] = false; reg1(arr[0],arr[1],arr[2]);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 3);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto heapPointer1 = VMLocalFunctionGetParameter(*vm, parameterCount, 1);
        auto heapPointer2 = VMLocalFunctionGetParameter(*vm, parameterCount, 2);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        auto type1 = VMLocalFunctionGetType(*vm, heapPointer1);
        auto type2 = VMLocalFunctionGetType(*vm, heapPointer2);
        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, 1);
        EXPECT_EQ(type1, HeapEnum::False);
        EXPECT_EQ(type2, HeapEnum::Null);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Object) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
    };
    wstring text = L"var o = object; o.a = true; o.b = false; reg1(o.a,o.b,o);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 3);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto heapPointer1 = VMLocalFunctionGetParameter(*vm, parameterCount, 1);
        auto heapPointer2 = VMLocalFunctionGetParameter(*vm, parameterCount, 2);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto type1 = VMLocalFunctionGetType(*vm, heapPointer1);
        auto type2 = VMLocalFunctionGetType(*vm, heapPointer2);
        auto count2 = VMLocalFunctionGetObjectFieldSize(*vm, heapPointer2);
        EXPECT_EQ(type0, HeapEnum::True);
        EXPECT_EQ(type1, HeapEnum::False);
        EXPECT_EQ(type2, HeapEnum::Object);
        EXPECT_EQ(count2, 2);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, LocalFunctionCall) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var a = true; reg1(a); var b = false; reg2(b); reg3(false);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionCall1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var fun1 = function(){ return null; }; reg1(fun1());";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Null);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionCall2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"function fun1(){ return null; } reg1(fun1());";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Null);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionCall3) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"function fun1(){ return false; } reg1(fun1());";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionCall4) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var fun1 = function(){ return false; }; reg1(fun1());";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionCall5) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"reg3(reg2(reg1(1)));";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, 1);
        return VMBoolToHeapPointer(false);
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMBoolToHeapPointer(true);
    });
    builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionClosure1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var i = 1; function fun(){ reg1(i); i = false; reg2(i); return null;} fun(); reg1(i); ";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, FunctionClosure2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var i = 1; function fun1(){ function fun2(){ reg1(i); i = false; reg2(i); } fun2(); reg1(i); } fun1(); ";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, If1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var b = null; if(true){ b = reg1(); } else{ b = reg2(); } reg3(b);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(true);
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(false);
    });
    builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, If2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var b = null; if(false){ b = reg1(); } else{ b = reg2(); } reg3(b);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(true);
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(false);
    });
    builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, If3) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var t = true; var f = false; if(reg1()){ if(reg2()){ reg3(f); } else { reg3(t); } } else { reg3(f); } ";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(true);
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(false);
    });
    builder.RegistLocalFunction(L"reg3", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, If4) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var fun = null; if(reg1()){ fun = function(){ return 1;};  } reg2(fun()); ";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        return VMBoolToHeapPointer(true);
    });
    builder.RegistLocalFunction(L"reg2", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, 1);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, While1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var i = 0; while( i < 10 ){ reg1(i); i = i + 1; } reg2(i);";
    int result = 0;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, result);
        result += 1;
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);
        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, 10);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Add) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"reg1(1 + 1, 1 + 1.5, 1.5 + 1, 1.5 + 1.5); reg2('a' + 'b', 'a' + \"bc\", \"ab\" + 'c', \"ab\" + \"cd\");";
    int result = 0;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 4);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetInt(*vm, heapPointer0);

        auto heapPointer1 = VMLocalFunctionGetParameter(*vm, parameterCount, 1);
        auto type1 = VMLocalFunctionGetType(*vm, heapPointer1);
        auto value1 = VMLocalFunctionGetFloat(*vm, heapPointer1);

        auto heapPointer2 = VMLocalFunctionGetParameter(*vm, parameterCount, 2);
        auto type2 = VMLocalFunctionGetType(*vm, heapPointer2);
        auto value2 = VMLocalFunctionGetFloat(*vm, heapPointer2);

        auto heapPointer3 = VMLocalFunctionGetParameter(*vm, parameterCount, 3);
        auto type3 = VMLocalFunctionGetType(*vm, heapPointer3);
        auto value3 = VMLocalFunctionGetFloat(*vm, heapPointer3);

        EXPECT_EQ(type0, HeapEnum::Int);
        EXPECT_EQ(value0, 2);

        EXPECT_EQ(type1, HeapEnum::Float);
        EXPECT_EQ(value1, 2.5f);

        EXPECT_EQ(type2, HeapEnum::Float);
        EXPECT_EQ(value2, 2.5f);

        EXPECT_EQ(type3, HeapEnum::Float);
        EXPECT_EQ(value3, 3.0f);

        return VMNullToHeapPointer();
    });

    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 4);
        auto heapPointer0 = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type0 = VMLocalFunctionGetType(*vm, heapPointer0);
        auto value0 = VMLocalFunctionGetStringData(*vm, heapPointer0);

        auto heapPointer1 = VMLocalFunctionGetParameter(*vm, parameterCount, 1);
        auto type1 = VMLocalFunctionGetType(*vm, heapPointer1);
        auto value1 = VMLocalFunctionGetStringData(*vm, heapPointer1);

        auto heapPointer2 = VMLocalFunctionGetParameter(*vm, parameterCount, 2);
        auto type2 = VMLocalFunctionGetType(*vm, heapPointer2);
        auto value2 = VMLocalFunctionGetStringData(*vm, heapPointer2);

        auto heapPointer3 = VMLocalFunctionGetParameter(*vm, parameterCount, 3);
        auto type3 = VMLocalFunctionGetType(*vm, heapPointer3);
        auto value3 = VMLocalFunctionGetStringData(*vm, heapPointer3);

        EXPECT_EQ(type0, HeapEnum::String);
        EXPECT_EQ(value0, L"ab");

        EXPECT_EQ(type1, HeapEnum::String);
        EXPECT_EQ(value1, L"abc");

        EXPECT_EQ(type2, HeapEnum::String);
        EXPECT_EQ(value2, L"abc");

        EXPECT_EQ(type3, HeapEnum::String);
        EXPECT_EQ(value3, L"abcd");

        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Equals) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"reg1(1 == 1,1 != 2, null == null, function(){} != function(){}, 1 != null, 1 != 1.0);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 6);
        for (int16_t i = 0; i < parameterCount; i++) {
            auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, i);
            auto type = VMLocalFunctionGetType(*vm, heapPointer);
            EXPECT_EQ(type, HeapEnum::True);
        }
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Compare) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var fun = function(){ return -1; }; var a = 1; reg1(1 < 2, 2 <= 2, 2 > 1, 3 >= 3, fun() == -1);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 5);
        for (int16_t i = 0; i < parameterCount; i++) {
            auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, i);
            auto type = VMLocalFunctionGetType(*vm, heapPointer);
            EXPECT_EQ(type, HeapEnum::True);
        }
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Recursive) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var i = 5; function fun(i){ if(i > 0){ fun(i - 1); } else { reg1(i); } return i; } fun(i);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 0);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, StackOverFlow) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"function fun(){ fun(); } fun();";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    EXPECT_THROW(VirtualMachineStart(vm), RuntimeException);
}

TEST(VirtualMachine, Break1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"while (true) { break; }";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Break2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L" var i = 2;  while (true) {if (i > 0) { i = i - 1; } else { reg1(i); break; } }";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 0);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Break3) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L" var i = 0;  while (true) { if (true) { i = i + 1; if (true) { i = i + 1; break; } } } reg1(i);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 2);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Continue1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var a = 2; while(true){ if (a > 0){ a = a - 1; continue; } break; } reg1(a);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 0);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Continue2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var i = 0; while(true){ if(true){ if(true) { i = i + 1; if (i == 2) { break; } else { continue; } } } break; } reg1(i);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 2);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, ObjectField1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = wstring() +
        L"var p6 = null;\n"
        L"var p10 = null;\n"
        L"var p16 = null;\n"
        L"var p17 = null;\n"
        L"var o = object;\n"
        L"o.p1 = 1;\n"
        L"o.p2 = 2;\n"
        L"o.p3 = 3;\n"
        L"o.p4 = 4;\n"
        L"o.p5 = 5;\n"
        L"o.p6 = 6;\n"
        L"o.p7 = 7;\n"
        L"o.p8 = 8;\n"
        L"o.p9 = 9;\n"
        L"o.p10 = 0;\n"
        L"o.p11 = 1;\n"
        L"o.p12 = 2;\n"
        L"o.p13 = 3;\n"
        L"o.p14 = 4;\n"
        L"o.p15 = 5;\n"
        L"o.p16 = 6;\n"
        L"reg1(o.p6);\n"
        L"reg2(o.p5);\n"
        L"reg3(o.p16);\n"
        ;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 6);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 5);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 6);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, ObjectField2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = wstring() +
        L"var p5 = null;\n"
        L"var p10 = null;\n"
        L"var p16 = null;\n"
        L"var p17 = null;\n"
        L"var o = object;\n"
        L"o.p1 = 1;\n"
        L"o.p2 = 2;\n"
        L"o.p3 = 3;\n"
        L"o.p4 = 4;\n"
        L"o.p5 = 5;\n"
        L"o.p6 = 6;\n"
        L"o.p7 = 7;\n"
        L"o.p8 = 8;\n"
        L"o.p9 = 9;\n"
        L"o.p10 = 0;\n"
        L"o.p11 = 1;\n"
        L"o.p12 = 2;\n"
        L"o.p13 = 3;\n"
        L"o.p14 = 4;\n"
        L"o.p15 = 5;\n"
        L"o.p16 = 6;\n"
        L"o.p17 = 7;\n"
        L"reg1(o.p5);\n"
        L"reg2(o.p17);\n"
        L"reg3(o.p16);\n"
        ;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 5);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 7);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 6);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, HeapOutMemory) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var arr = array[4096];";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    EXPECT_THROW(VirtualMachineStart(vm), RuntimeException);
}

TEST(VirtualMachine, GC1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = wstring() +
        L"var a = array[5];\n"
        L"reg2(a);\n"
        L"var fun = function(){\n "
        L"    var a = array[5];\n"
        L"    var b = object;\n"
        L"    b.o = 1; \n"
        L"};\n"
        L"var b = object;\n"
        L"fun();\n"
        L"reg1();\n"
        L"reg2(a);\n"
        L"reg3(b);\n"
        ;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        VirtualMachineGC(*vm);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto length = VMLocalFunctionGetArraySize(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Array);
        EXPECT_EQ(length, 5);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Object);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, GC2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
        L"reg4",
        L"reg5",
        L"reg6",
        L"reg7",
    };
    wstring text = wstring() +
        L"var a = array[1];\n"
        L"var fun = function(){\n"
        L"    var b = array[3];\n"
        L"    var c = array[2];\n"
        L"    b[0] = false;\n"
        L"    b[1] = c;\n"
        L"    b[2] = 100;\n"
        L"    c[0] = 1;\n"
        L"    c[1] = b;\n"
        L"    return b;\n"
        L"};\n"
        L"a = fun();\n"
        L"reg1();\n"
        L"reg2(a);\n"
        L"reg3(a[0]);\n"
        L"reg4(a[1]);\n"
        L"reg5(a[2]);\n"
        L"reg6(a[1][0]);\n"
        L"reg7(a[1][1]);\n"
        ;
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        VirtualMachineGC(*vm);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto length = VMLocalFunctionGetArraySize(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Array);
        EXPECT_EQ(length, 3);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::False);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg4", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto length = VMLocalFunctionGetArraySize(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Array);
        EXPECT_EQ(length, 2);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg5", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 100);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg6", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 1);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg7", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto length = VMLocalFunctionGetArraySize(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Array);
        EXPECT_EQ(length, 3);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Op1) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"reg1(1 + 2 * 3 + 4); reg2((1+2) * (3 + 4)); reg3( 1 + 2 * 3 + 4 <= (1+2) * (3 + 4) || false);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 11);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg2", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        auto value = VMLocalFunctionGetInt(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::Int);
        EXPECT_EQ(value, 21);
        return VMNullToHeapPointer();
    });
    builder.RegistLocalFunction(L"reg3", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}

TEST(VirtualMachine, Op2) {
    vector<wstring> regNames{
        L"reg1",
        L"reg2",
        L"reg3",
    };
    wstring text = L"var a = array[2]; a[0] = 5; var b = array[2]; b[0] = 5; reg1(a == b);";
    auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
    builder.RegistLocalFunction(L"reg1", [&](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
        EXPECT_EQ(parameterCount, 1);
        auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
        auto type = VMLocalFunctionGetType(*vm, heapPointer);
        EXPECT_EQ(type, HeapEnum::True);
        return VMNullToHeapPointer();
    });
    auto vm = builder.Build();
    VirtualMachineInit(vm);
    VirtualMachineStart(vm);
}