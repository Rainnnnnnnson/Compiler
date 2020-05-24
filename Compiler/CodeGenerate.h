#pragma once
#include<cstdint>
#include<vector>
#include<string>
#include<map>
using std::map;
using std::vector;
using std::wstring;




/*
    ������ֵ�����⴦�� ֱ�Ӵ���ڶ��� ��������ֱ�ӻ�ȡ��λ��
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
    AddRecursiveFunctionItem, //�������ڵݹ麯������ʱ��ʹ��

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
    1.����
    2.����
    3.�뵱ǰջָ���ƫ��
    4.���Դ�float int bool char ���� ָ�� ��һϵ��ֵ
      �����offest word ʹ�õ�һ�� �ڶ�������;

    offest     ��ʾָ���ָ�������ջ��λ�� ������Ҫѹջ�򱣳�ԭ��offest ������������
    stackOrder ����ʹ��ָ��֮ǰ��Ҫѹջ������

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
    Nothing������������ʱ�ж�
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
    1.����
    2.λ����
    00000010 ��Ϊ�������յı��λ
    00000001 ��Ϊ�������յı��λ
    3.�ڶ��еĳ���(��������)
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
    ����32�ֽ�
    bool char �洢�� int ռ��4�ֽ�

    word   ���� string   �洢 �����ַ�
    length ���� function �洢 �������� �� �հ�����
    length ���� object   �洢 �������� �� ��������
    length ���� array    �洢 ���鳤�� (�ڶ������� ��0)

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

    ������� ָ��ѵ�ָ�� ���� SP PC


     ѹջ��ʽ:


     ����ǰ
     [   ......     ]
     [ parameter 3  ]
     [ parameter 2  ]
     [ parameter 1  ]
     [     Null     ]  ѹջ����ΪNull
     [     Null     ]  ѹջ����ΪNull
     [     Null     ]  ѹջ����ΪNull
     [   Function1  ]
     [     var3     ]
     [     var2     ]
     [     var1     ]
     [  ClosureMain ]
     [    PCMain    ]
     [    SPMain    ]
     [ FunctionMain ]

     ���ú�
     [   ......     ]
     [     var2     ]
     [     var1     ]
     [   ......     ]
     [ parameter 3  ]
     [ parameter 2  ]
     [ parameter 1  ]  (ѹ������λ��) ��ʼλ�� Ϊ SP1 + 2
     [  Closure1    ] ��Function1�е�Closure��ȡ����
     [      PC1     ]
     [      SP1     ]  (ջ�ο�ʼ�ĵط�)   ����ָ��SPMain
     [   Function1  ]  ��һ����ú�����λ�� ��������ֵ�����滻��
     [     var3     ]
     [     var2     ]
     [     var1     ]   ���������ָ��
     [  ClosureMain ]   ��FunctionMain�е�Closure��ȡ����
     [    PCMain    ]   ֵΪ0  ������
     [    SPMain    ]   ֵΪ0  �������ж��Ƿ��������
     [ FunctionMain ]   �����һ��ʼֻ����MainFunction �����￪ʼ��һ�������ĵ���

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