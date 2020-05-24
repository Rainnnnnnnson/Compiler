#pragma once
#include<cstdint>
#include<vector>
#include<string>
#include<map>
using std::map;
using std::vector;
using std::wstring;




/*
    这三个值做特殊处理 直接存放在堆中 各种运算直接获取其位置
    Null  heap[0]
    False heap[1]
    True  heap[2]
*/
enum class InstructionEnum : int8_t {
    Unused = 0,
    GetNull,
    GetFalse,
    GetTrue,
    CreateChar,
    CreateInt,
    CreateFloat,
    CreateString,
    CreateArray,
    CreateObject,
    CreateClosure,
    CreateFunction,
    AddRecursiveFunctionItem, //这条用于递归函数创建时候使用

    GetVariableByOffest,
    SetVariableByOffest,

    GetClosureItemByOffest,
    SetClosureItemByOffest,

    AccessArray,
    AccessField,
    AssignmentArray,
    AssignmentField,
    FunctionCall,

    Jump,
    ConditionJump,
    Return,

    Multiply,
    Divide,
    Modulus,
    Add,
    Subtract,
    Less,
    LessEquals,
    Greater,
    GreaterEquals,
    Equals,
    NotEquals,
    Or,
    And,

    Not,
};
/*
    [-----64-----]
    [8][8][16][32]
    1.类型
    2.保留
    3.与当前栈指针的偏移
    4.可以存float int bool char 或者 指针 等一系列值
      如果是offest word 使用第一个 第二个保留;

    offest     表示指向该指令相对于栈的位置 若不需要压栈则保持原有offest 用于垃圾回收
    stackOrder 代表使用指令之前需要压栈的类型

    type                reserved       offest           value                 stackOrder
    Unused
    GetNull                                             intValue (0)
    GetFalse                                            intValue (1)
    GetTrue                                             intValue (2)
    CreateChar                                          word
    CreateInt                                           intValue
    CreateFloat                                         floatValue
    CreateString                                        intValue(index)
    CreateArray                                                               Expression(length)
    CreateObject
    CreateClosure                                       offestOrLength
    CreateFunction    parameterCount                    intValue(position)    Closure
    AddRecursiveFunctionItem                            offestOrLength        Function

    GetVariableByOffest                                 offestOrLength
    SetVariableByOffest                                 offestOrLength

    GetClosureItemByOffest                              offestOrLength
    SetClosureItemByOffest                              offestOrLength

    AccessArray                                                               Array               Expression
    AccessField                                         intValue(index)       Object
    AssignmentArray                                                           Array               Expression(index)   Expression
    AssignmentField                                     intValue(index)       Object              Expression
    FunctionCall                                        offestOrLength        Function            Null                Null            Null       parameter.....

    Jump                                                intValue(position)
    ConditionJump                                       intValue(position)    Expression
    Return                                                                    Expression

    BinaryOperation                                                           Expression          Expression
    UnaryOperation                                                            Expression
*/
struct Instruction {
    InstructionEnum type = InstructionEnum::Unused;
    int8_t reserved = 0;
    int16_t offest = 0;
    union {
        int16_t offestOrLength;
        wchar_t word;
        int32_t intValue = 0;
        float floatValue;
    } value;
};

/*
    Nothing用于垃圾回收时判断
*/
enum class HeapEnum : int8_t {
    Nothing = 0,
    Null,
    False,
    True,
    Char,
    Int,
    Float,
    String,
    Array,
    Object,
    ObjectFieldList,
    Closure,
    Function,
    LocalFunction,
};

/*
    [---32---]
    [8][8][16]
    1.类型
    2.位作用
    00000010 作为永不回收的标记位
    00000001 作为垃圾回收的标记位
    3.在堆中的长度(包含此项)
*/
struct HeapTypeHead {
    HeapEnum type;
    int8_t reserved;
    int16_t memorylength;
};

enum class StringDataType : int16_t {
    Index,
    Length,
};

struct StringLengthOrIndex {
    int16_t lengthOrIndex;
    StringDataType type;
};

/*
    对齐32字节
    bool char 存储在 int 占用4字节

    word   用于 string   存储 两个字符
    length 用于 function 存储 参数个数 和 闭包个数
    length 用于 object   存储 容器容量 和 容器总数
    length 用于 array    存储 数组长度 (第二个保留 填0)

    type              reserved              memoryLength               value
    Nothing                                      1
    Null              00000010                   1
    False             00000010                   1
    True              00000010                   1
    Char                                         2                     word
    Int                                          2                     intValue
    Float                                        2                     floatValue
    String                                       2                     intValue(index)
    Array                                    2 + length                length                   intValue(heapPosition)......
    Object                                       2                     intValue(heapPosition)
    ObjectFieldList                          2 + length                length                   [ intValue(index)   intValue(heapPosition) ]........
    Closure                                  2 + length                length                   intValue(heapPosition)......
    Function                                     4                     length(parameterCount)   intValue(heapPosition)   intValue(grogramPosition)
    LocalFunction                                2                     intValue(localList)
*/
struct HeapType {
    union {
        HeapTypeHead typeHead = HeapTypeHead{HeapEnum::Nothing, 0, 0};
        int32_t intValue;
        float floatValue;
        wchar_t word[2];
        int16_t length;
        StringLengthOrIndex stringLengthOrIndex;
    } value;
};
/*
    [--32--]
    [  32  ]

    仅仅存放 指向堆的指针 或者 SP PC


     压栈方式:


     调用前
     [   ......     ]
     [ parameter 3  ]
     [ parameter 2  ]
     [ parameter 1  ]
     [     Null     ]  压栈设置为Null
     [     Null     ]  压栈设置为Null
     [     Null     ]  压栈设置为Null
     [   Function1  ]
     [     var3     ]
     [     var2     ]
     [     var1     ]
     [  ClosureMain ]
     [    PCMain    ]
     [    SPMain    ]
     [ FunctionMain ]

     调用后
     [   ......     ]
     [     var2     ]
     [     var1     ]
     [   ......     ]
     [ parameter 3  ]
     [ parameter 2  ]
     [ parameter 1  ]  (压参数的位置) 起始位置 为 SP1 + 2
     [  Closure1    ] 将Function1中的Closure提取出来
     [      PC1     ]
     [      SP1     ]  (栈段开始的地方)   内容指向SPMain
     [   Function1  ]  上一层调用函数的位置 函数返回值将会替换它
     [     var3     ]
     [     var2     ]
     [     var1     ]   变量保存堆指针
     [  ClosureMain ]   将FunctionMain中的Closure提取出来
     [    PCMain    ]   值为0  无意义
     [    SPMain    ]   值为0  会用于判断是否结束程序
     [ FunctionMain ]   虚拟机一开始只保存MainFunction 从这里开始第一个函数的调用

*/

struct StackType {
    int32_t intValue = 0;
};

struct VMRuntimeData {
    vector<Instruction> instruction;
    vector<wstring> registeredNames;
    vector<wstring> staticString;
    vector<int> mainClosureOffest;
    vector<int> instructionLine;
    map<wstring, int32_t> stringMap;
};