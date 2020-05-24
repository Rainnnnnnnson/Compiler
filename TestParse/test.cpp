#include "pch.h"
#include "Parse.h"
#include "CompilerException.h"
using namespace Parse;
template<typename T>
void TestNFAParseString(const NFA& nfa, const wstring& str) {
    auto node = nfa.start;
    for (auto character : str) {
        if (node->edges.empty() && (node->anyoneCharacterEdgeNode == nullptr)) {
            throw ParseException();
        }
        bool hasEdge = false;
        for (auto edge : node->edges) {
            if (edge.character == character) {
                node = edge.node;
                hasEdge = true;
                break;
            }
        }
        if (hasEdge) {
            continue;
        }
        if (node->anyoneCharacterEdgeNode != nullptr) {
            node = node->anyoneCharacterEdgeNode;
            continue;
        }
        throw ParseException();
    }
    dynamic_cast<T&>(*node->generate());
}

TEST(NFA, CreateNFA) {
    auto nfa = CreateNFA<DoubleAnd>(L"&&");

    EXPECT_THROW(TestNFAParseString<DoubleAnd>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<DoubleAnd>(nfa, L"&"), exception);
    EXPECT_NO_THROW(TestNFAParseString<DoubleAnd>(nfa, L"&&"));
    EXPECT_THROW(TestNFAParseString<DoubleAnd>(nfa, L"&&&"), exception);

}

TEST(NFA, CreateNFAId) {
    auto nfa = CreateNFAId();
    EXPECT_THROW(TestNFAParseString<Id>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Id>(nfa, L"#3124"), exception);
    EXPECT_THROW(TestNFAParseString<Id>(nfa, L"123ASDR"), exception);
    EXPECT_THROW(TestNFAParseString<Id>(nfa, L"ASDR#"), exception);

    EXPECT_NO_THROW(TestNFAParseString<Id>(nfa, L"S"));
    EXPECT_NO_THROW(TestNFAParseString<Id>(nfa, L"_234"));
    EXPECT_NO_THROW(TestNFAParseString<Id>(nfa, L"asd_2134"));
    EXPECT_NO_THROW(TestNFAParseString<Id>(nfa, L"Ssdf_234sdfASD"));
}

TEST(NFA, CreateNFAInt) {
    auto nfa = CreateNFAInt();

    EXPECT_THROW(TestNFAParseString<Int>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Int>(nfa, L"123.345"), exception);
    EXPECT_THROW(TestNFAParseString<Int>(nfa, L"#124241"), exception);
    EXPECT_THROW(TestNFAParseString<Int>(nfa, L"-"), exception);
    EXPECT_THROW(TestNFAParseString<Int>(nfa, L"-123.345"), exception);
    EXPECT_THROW(TestNFAParseString<Int>(nfa, L"-#124241"), exception);


    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"123"));
    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"000123"));
    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"0"));
    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"-123"));
    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"-000123"));
    EXPECT_NO_THROW(TestNFAParseString<Int>(nfa, L"-0"));
}

TEST(NFA, CreateNFAFloat) {
    auto nfa = CreateNFAFloat();
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"#123"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L".11"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"1.1.1"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"00."), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"123"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-#123"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-.11"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-1.1.1"), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-00."), exception);
    EXPECT_THROW(TestNFAParseString<Float>(nfa, L"-123"), exception);

    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"123.0"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"0.1"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"00000.1"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"0123456789.123456789000000"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"-123.0"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"-0.1"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"-00000.1"));
    EXPECT_NO_THROW(TestNFAParseString<Float>(nfa, L"-0123456789.123456789000000"));
}

TEST(NFA, CreateNFABlank) {
    auto nfa = CreateNFABlank();
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"   \\   "), exception);

    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L" "));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"\t\r\n \n\n"));
}

TEST(NFA, CreateNFASingleLineComment) {
    auto nfa = CreateNFASingleLineComment();
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"/SADF"), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"/SADF//"), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"//SADF\n\n"), exception);

    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"//"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"//\n"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"//asdf"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"//asdf\n"));
}

TEST(NFA, CreateNFAMultiLineComments) {
    auto nfa = CreateNFAMultiLineComments();
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"/*asdf"), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"/*asdf*"), exception);
    EXPECT_THROW(TestNFAParseString<Blank>(nfa, L"/**3*/sdfsdf*/"), exception);

    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"/**/"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"/*1234*/"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"/*1234\n*/"));
    EXPECT_NO_THROW(TestNFAParseString<Blank>(nfa, L"/*1234\n2134\n\n\n */"));
}

TEST(NFA, CreateNFAChar) {
    auto nfa = CreateNFAChar();

    EXPECT_THROW(TestNFAParseString<Char>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<Char>(nfa, L"'"), exception);
    EXPECT_THROW(TestNFAParseString<Char>(nfa, L"''"), exception);
    EXPECT_THROW(TestNFAParseString<Char>(nfa, L"'12'"), exception);
    EXPECT_THROW(TestNFAParseString<Char>(nfa, L"'\\x'"), exception);
    EXPECT_THROW(TestNFAParseString<Char>(nfa, L"'\n'"), exception);

    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\r'"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\n'"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\t'"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\''"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\\\'"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\\\"'"));
    EXPECT_NO_THROW(TestNFAParseString<Char>(nfa, L"'\"'"));
}

TEST(NFA, CreateNFAString) {
    auto nfa = CreateNFAString();

    EXPECT_THROW(TestNFAParseString<String>(nfa, L""), exception);
    EXPECT_THROW(TestNFAParseString<String>(nfa, L"\""), exception);
    EXPECT_THROW(TestNFAParseString<String>(nfa, L"\"\"\""), exception);
    EXPECT_THROW(TestNFAParseString<String>(nfa, L"\"\\\""), exception);
    EXPECT_THROW(TestNFAParseString<String>(nfa, L"\"\n\""), exception);

    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\t\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\" \""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\r\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\t\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\\\"\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"\\\'\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"'\""));
    EXPECT_NO_THROW(TestNFAParseString<String>(nfa, L"\"asdfdsf\""));
}
template<typename T>
void TestLexicalAnalysis(const LexicalAnalysisResult& result, int index, int line) {
    dynamic_cast<T&>(*result.resultList[index]);
    if (line != result.resultList[index]->line) {
        throw exception();
    }
}

TEST(DFA, Id) {

    vector<NFA> nfas;
    nfas.push_back(CreateNFAId());
    nfas.push_back(CreateNFABlank());
    nfas.push_back(CreateNFA<If>(L"if"));
    auto nfa = CompositeNFA(std::move(nfas));
    auto dfa = CreateDFA(std::move(nfa));

    GenerateLATypeFunction genertate = dfa.generates[4];
    auto result = LexicalAnalysis(dfa, L"i if if8 if8if ifif");

    EXPECT_NO_THROW(TestLexicalAnalysis<Id>(result, 0, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<If>(result, 2, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Id>(result, 4, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Id>(result, 6, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Id>(result, 8, 1));
}

TEST(DFA, DoubleAnd) {
    vector<NFA> nfas;
    nfas.push_back(CreateNFA<DoubleAnd>(L"&&"));
    auto nfa = CompositeNFA(std::move(nfas));
    auto dfa = CreateDFA(std::move(nfa));

    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"&&"));

    EXPECT_THROW(LexicalAnalysis(dfa, L"&"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"&&&"), ParseException);
}

TEST(DFA, Int) {
    vector<NFA> nfas;
    nfas.push_back(CreateNFAInt());
    auto nfa = CompositeNFA(std::move(nfas));
    auto dfa = CreateDFA(std::move(nfa));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"2147483647"));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"-2147483648"));
    EXPECT_THROW(LexicalAnalysis(dfa, L"2147483648"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"-2147483649"), ParseException);
}

TEST(DFA, Char) {
    auto dfa = CreateDFA(CreateNFAChar());

    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"'a'"));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"'#'"));
    EXPECT_THROW(LexicalAnalysis(dfa, L"'a'#"), ParseException);

}

TEST(DFA, Text) {
    DFA dfa = CreateDefaultDFA();

    const wchar_t* newLine = L"\n";

    auto str = wstring() +
        L"var if8if = \" 2134ASDF\";" + newLine +
        L"//   ASDF" + newLine +
        L"/*    a = -2.2;" + newLine
        + newLine
        + L"*/" + newLine +
        L"//ASDF";

    auto result = LexicalAnalysis(dfa, str);
    //第一行
    EXPECT_NO_THROW(TestLexicalAnalysis<Var>(result, 0, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 1, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Id>(result, 2, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 3, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Equals>(result, 4, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 5, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<String>(result, 6, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Semicolon>(result, 7, 1));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 8, 1));
    //第二行
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 9, 2));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 10, 3));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 11, 5));
    EXPECT_NO_THROW(TestLexicalAnalysis<Blank>(result, 12, 6));
    EXPECT_NO_THROW(TestLexicalAnalysis<TextEnd>(result, 13, 6));
    EXPECT_EQ(result.resultList.size(), 14);

    //字符判断
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"\"###\""));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"/*###*/"));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"'#'"));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"//####"));
    EXPECT_NO_THROW(LexicalAnalysis(dfa, L"if"));

    EXPECT_THROW(LexicalAnalysis(dfa, L"\"###\"#####"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"/*###*/#####"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"'#'#####"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"//####\n#####"), ParseException);
    EXPECT_THROW(LexicalAnalysis(dfa, L"if#####"), ParseException);
}

struct TestX : SAType {};
struct TestY : SAType {};
struct TestZ : SAType {};
struct Testa : LAType {};
struct Testc : LAType {};
struct Testd : LAType {};

template<typename T>
bool TestCanFindNullableFirstFollow(const NullableFirstFollowTable& table) {
    return table.table.find(type_index(typeid(T))) != table.table.end();
}

template<typename T>
bool TestJudgmentNullable(NullableFirstFollowTable& table, bool b) {
    return table.table[type_index(typeid(T))].nullable == b;
}

template<typename T, typename ...P>
bool TestJudgmentFirstSet(NullableFirstFollowTable& table) {
    return table.table[type_index(typeid(T))].firstSet == set<type_index>{type_index(typeid(P))...};
}

template<typename T, typename ...P>
bool TestJudgmentFollowSet(NullableFirstFollowTable& table) {
    return table.table[type_index(typeid(T))].followSet == set<type_index>{type_index(typeid(P))...};
}

TEST(CreateNullableFirstFollowTable, Test1) {
    /*
        Z -> d
        Z -> X Y Z
        Y -> ε
        Y -> c
        X -> Y
        X -> a
    */
    vector<Production> vec{
        CreateProduction<TestX, TestY>(),
        CreateProduction<TestX, Testa>(),
        CreateProduction<TestY>(),
        CreateProduction<TestY, Testc>(),
        CreateProduction<TestZ, Testd>(),
        CreateProduction<TestZ, TestX, TestY, TestZ>(),
    };
    auto table = CreateNullableFirstFollowTable(std::move(vec));
    EXPECT_EQ(table.table.size(), 3);

    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestX>(table));
    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestY>(table));
    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestZ>(table));

    EXPECT_TRUE(TestJudgmentNullable<TestX>(table, true));
    EXPECT_TRUE(TestJudgmentNullable<TestY>(table, true));
    EXPECT_TRUE(TestJudgmentNullable<TestZ>(table, false));

    bool b = false;
    b = TestJudgmentFirstSet<TestX, Testa, Testc>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFirstSet<TestY, Testc>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFirstSet<TestZ, Testa, Testc, Testd>(table);
    EXPECT_TRUE(b);


    b = TestJudgmentFollowSet<TestX, Testa, Testc, Testd>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFollowSet<TestY, Testa, Testc, Testd>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFollowSet<TestZ>(table);
    EXPECT_TRUE(b);
}

TEST(CreateNullableFirstFollowTable, Test2) {
    /*
        X -> a Y Z
        Y -> c
        Y -> ε
        Z -> X d
        Z -> ε
    */
    vector<Production> vec{
        CreateProduction<TestX, Testa, TestY, TestZ>(),
        CreateProduction<TestY, Testc>(),
        CreateProduction<TestY>(),
        CreateProduction<TestZ, TestX, Testd>(),
        CreateProduction<TestZ>(),
    };
    auto table = CreateNullableFirstFollowTable(vec);
    EXPECT_EQ(table.table.size(), 3);

    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestX>(table));
    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestY>(table));
    EXPECT_TRUE(TestCanFindNullableFirstFollow<TestZ>(table));

    EXPECT_TRUE(TestJudgmentNullable<TestX>(table, false));
    EXPECT_TRUE(TestJudgmentNullable<TestY>(table, true));
    EXPECT_TRUE(TestJudgmentNullable<TestZ>(table, true));

    bool b = false;
    b = TestJudgmentFirstSet<TestX, Testa>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFirstSet<TestY, Testc>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFirstSet<TestZ, Testa>(table);
    EXPECT_TRUE(b);


    b = TestJudgmentFollowSet<TestX, Testd>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFollowSet<TestY, Testa, Testd>(table);
    EXPECT_TRUE(b);
    b = TestJudgmentFollowSet<TestZ, Testd>(table);
    EXPECT_TRUE(b);
}

TEST(CreatePredictiveParsingTable, Test1) {
    vector<Production> vec{
        CreateProduction<TestX, TestY>(),
        CreateProduction<TestX, Testa>(),
        CreateProduction<TestY>(),
        CreateProduction<TestY, Testc>(),
        CreateProduction<TestZ, Testd>(),
        CreateProduction<TestZ, TestX, TestY, TestZ>(),
    };
    auto nffTable = CreateNullableFirstFollowTable(vec);
    EXPECT_THROW(CreatePredictiveParsingTable(std::move(nffTable), std::move(vec), 0), CompilerError);
}

template<typename First, typename Second>
bool TestPredictiveParsingTableItem(const PredictiveParsingTable& table, const Production& production) {
    auto find = table.table.find(PredictiveParsingTableSelect(type_index(typeid(First)), type_index(typeid(Second))));
    if (find == table.table.end()) {
        return false;
    }
    return find->second == &production;
}

TEST(CreatePredictiveParsingTable, Test2) {
    vector<Production> vec{
        CreateProduction<TestX, Testa, TestY, TestZ>(),
        CreateProduction<TestY, Testc>(),
        CreateProduction<TestY>(),
        CreateProduction<TestZ, TestX, Testd>(),
        CreateProduction<TestZ>(),
    };
    auto nffTable = CreateNullableFirstFollowTable(vec);
    auto table = CreatePredictiveParsingTable(std::move(nffTable), std::move(vec), 0);
    EXPECT_EQ(table.table.size(), 6);

    bool b = false;

    b = TestPredictiveParsingTableItem<TestX, Testa>(table, table.productions[0]);
    EXPECT_TRUE(b);
    b = TestPredictiveParsingTableItem<TestY, Testa>(table, table.productions[2]);
    EXPECT_TRUE(b);
    b = TestPredictiveParsingTableItem<TestY, Testc>(table, table.productions[1]);
    EXPECT_TRUE(b);
    b = TestPredictiveParsingTableItem<TestY, Testd>(table, table.productions[2]);
    EXPECT_TRUE(b);
    b = TestPredictiveParsingTableItem<TestZ, Testa>(table, table.productions[3]);
    EXPECT_TRUE(b);
    b = TestPredictiveParsingTableItem<TestZ, Testd>(table, table.productions[4]);
    EXPECT_TRUE(b);
}

TEST(CreatePredictiveParsingTable, Default) {
    EXPECT_NO_THROW(CreateDefaultPredictiveParsingTable());
}

SAType* Cast(unique_ptr<ParseType>& type) {
    return dynamic_cast<SAType*>(&*type);
}
template<typename T>
bool TestParseTreeNode(unique_ptr<ParseType>& type) {
    return dynamic_cast<T*>(&*type) != nullptr;
}

AbstractSyntaxTree TestAbstractSyntaxTree(const wstring& str) {
    static DFA dfa = CreateDefaultDFA();
    static PredictiveParsingTable ppt = CreateDefaultPredictiveParsingTable();
    static GenerateSATypeFunctionMap gMap = CreateDefaultGenerateSATypeFunctionMap();

    auto la = LexicalAnalysis(dfa, str);
    auto la2 = LexicalAnalysisResultRemoveBlank(std::move(la));
    auto pt = CreateParseTree(ppt, gMap, std::move(la2));
    auto result = CreateAbstractSyntaxTree(pt);
    return result;
}

TEST(CreateAbstractSyntaxTree, Statement) {
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"function fun(p0, p1){ }"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"var fun = function(p0, p1){ };"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id = 1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id[1] = 1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id.id = 1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id();"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"if(true) {}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"if(true) {} else {}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"if(true) {} else if (false) {}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"if(true) {} else if (false) {} else {}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"while(true) {}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"break;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"continue;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return 1;"));

    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"var a = 1;a = 2; a = 3;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"if(true){var a = 1;a = 2; a = 3;}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"while(true){var a = 1;a = 2; a = 3;}"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"var fun = function(){var a = 1;a = 2; a = 3;};"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"function fun(){var a = 1;a = 2; a = 3;}"));
    //这几种情况无意义
    EXPECT_THROW(TestAbstractSyntaxTree(L"id;"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"id[1];"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"id.id;"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"id() = 1;"), ParseException);
}

TEST(CreateAbstractSyntaxTree, UnknownOperate) {
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id.id[1]().id[1][1][1] = id.id[1]().id[1][1][1]()(1,1)(1,1,1)(1,1,1);"));
}

TEST(CreateAbstractSyntaxTree, FunctionCall) {
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"id(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16);"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"var fun = function(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16){};"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"function fun(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16){}"));
    //参数不超过16
    EXPECT_THROW(TestAbstractSyntaxTree(L"id(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,17);"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"var fun = function(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,17){};"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"function fun(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,17){}"), ParseException);
}

TEST(CreateAbstractSyntaxTree, Type) {
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return 1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return 1.1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return true;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return false;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return \"string\";"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return 'c';"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return null;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return object;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return array[1];"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return function(){};"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return function(a, b, c){};"));
    EXPECT_THROW(TestAbstractSyntaxTree(L"return function(1, 2, 3){}"), ParseException);
}

TEST(CreateAbstractSyntaxTree, Subtract) {
    //负号和数字在一起    判定为 数字
    //负号和数字有空格    判定为 减号 和 数字
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return  1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return -1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return  1 -  1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return  1 - -1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return  a -  a;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return  0 -  a;"));

    EXPECT_THROW(TestAbstractSyntaxTree(L"retrun -a;"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"retrun  1   -1;"), ParseException);
    EXPECT_THROW(TestAbstractSyntaxTree(L"retrun    -  1;"), ParseException);
}

TEST(CreateAbstractSyntaxTree, Not) {
    //限定只能与Unknown一起用
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !id;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !id[1];"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !id();"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !id.id;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !1;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !false;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !true;"));
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return !false||!true&&!1;"));

    EXPECT_THROW(TestAbstractSyntaxTree(L"return !!!false;"), ParseException);
}

TEST(CreateAbstractSyntaxTree, BinaryOperation) {
    EXPECT_NO_THROW(TestAbstractSyntaxTree(L"return 1 + 2 * 3 || 4 != 5 <= 6 - (7) / (1 + 2 * (3 - 4)) && id() || id % array[1];"));
}

TEST(CreateRegisteredNameList, Test) {
    auto dfa = CreateDefaultDFA();
    EXPECT_NO_THROW(CreateRegisteredNameList(dfa, vector<wstring>()));
    EXPECT_NO_THROW(CreateRegisteredNameList(dfa, vector<wstring>{L"f234", L"ABEC"}));

    EXPECT_THROW(CreateRegisteredNameList(dfa, vector<wstring>{L""}), ParseException);
    EXPECT_THROW(CreateRegisteredNameList(dfa, vector<wstring>{L"1234", L"##"}), ParseException);
    EXPECT_THROW(CreateRegisteredNameList(dfa, vector<wstring>{L"f123", L"f123"}), ParseException);
}