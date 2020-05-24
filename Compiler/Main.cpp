#include "Complie.h"
#include "CompilerException.h"
#include "VirtualMachine.h"
#include <iostream>
#include <fstream>
using std::wifstream;
using std::wcout;
int main() {
    string path = "../demo.txt";
    std::wifstream f(path, std::ios::in);
    if (!f.is_open()) {
        std::cout << "没找到文件" + path << std::endl;
        return 0;
    }
    wstring text;
    wstring strLine;
    while (std::getline(f, strLine)) {
        text += strLine + L"\n";
    }
    try {
        auto compileData = CompileData();
        vector<wstring> regNames{
            L"Print",
            L"ArrayLength",
        };
        auto builder = VirtualMachineBuilder(GenerateVMRuntimeData(text, compileData, regNames));
        builder.RegistLocalFunction(L"Print", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
            if (parameterCount != 1) {
                throw RuntimeException("Print 参数个数不为 1");
            }
            auto heapPointer = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
            auto type = VMLocalFunctionGetType(*vm, heapPointer);
            switch (type) {
                case HeapEnum::Null:
                    wcout << "null";
                    break;
                case HeapEnum::False:
                    wcout << "false";
                    break;
                case HeapEnum::True:
                    wcout << "true";
                    break;
                case HeapEnum::Char:
                    wcout << VMLocalFunctionGetChar(*vm, heapPointer);
                    break;
                case HeapEnum::Int:
                    wcout << VMLocalFunctionGetInt(*vm, heapPointer);
                    break;
                case HeapEnum::Float:
                    wcout << VMLocalFunctionGetFloat(*vm, heapPointer);
                    break;
                case HeapEnum::String:
                    wcout << VMLocalFunctionGetStringData(*vm, heapPointer);
                    break;
                case HeapEnum::Array:
                    wcout << L"Array";
                    break;
                case HeapEnum::Object:
                    wcout << L"Object";
                    break;
                case HeapEnum::Function:
                    wcout << L"Funtion";
                    break;
                case HeapEnum::LocalFunction:
                    wcout << L"LocalFuntion";
                    break;
                default:
                    throw RuntimeException("Print 参数类型有问题");
            }
            return VMNullToHeapPointer();
        });
        builder.RegistLocalFunction(L"ArrayLength", [](VirtualMachine* vm, int16_t parameterCount) ->int32_t {
            if (parameterCount != 1) {
                throw RuntimeException("ArrayLength 参数个数不为 1");
            }
            auto heapPointerArray = VMLocalFunctionGetParameter(*vm, parameterCount, 0);
            auto type = VMLocalFunctionGetType(*vm, heapPointerArray);
            if (type != HeapEnum::Array) {
                throw RuntimeException("ArrayLength 参数类型有问题");
            }
            int32_t size = VMLocalFunctionGetArraySize(*vm, heapPointerArray);
            int32_t heapPointerInt = VMAllocateHeapMemory(*vm, HeapEnum::Int, 2);
            auto heapPointerIntPtr = VMHeapMemory(*vm, heapPointerInt);
            heapPointerIntPtr[1].value.intValue = size;
            return heapPointerInt;
        });
        auto vm = builder.Build();
        VirtualMachineInit(vm);
        VirtualMachineStart(vm);
    } catch (const exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}