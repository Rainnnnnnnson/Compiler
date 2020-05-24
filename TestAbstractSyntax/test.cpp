#include "pch.h"
#include "Parse.h"
#include "AbstractSyntax.h"

AbstractSyntaxTreeTransform TestSemanticAnalysis(const vector<wstring>& names, const wstring& str) {
    static DFA dfa = CreateDefaultDFA();
    static PredictiveParsingTable ppt = CreateDefaultPredictiveParsingTable();
    static GenerateSATypeFunctionMap gMap = CreateDefaultGenerateSATypeFunctionMap();

    auto la = LexicalAnalysis(dfa, str);
    auto la2 = LexicalAnalysisResultRemoveBlank(std::move(la));
    auto pt = CreateParseTree(ppt, gMap, std::move(la2));
    auto namelist = CreateRegisteredNameList(dfa, names);
    auto ast = CreateAbstractSyntaxTree(pt);
    auto result = SemanticAnalysis(namelist, std::move(ast));
    return result;
}

TEST(SemanticAnalysis, MainBlock) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var a = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var a = 1; return null;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"return null;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L""));

    EXPECT_THROW(TestSemanticAnalysis(names, L"break;"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"continue;"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"break; var a = 1;"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"continue; var a = 1;"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"return null; var a = 1;"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"return null; return null;"), CompileException);
}

TEST(SemanticAnalysis, FunctionBlock) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function() { var a = 1; };"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function() { var a = 1; return null; };"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function() { return null; };"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function() { };"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun() { var a = 1; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun() { var a = 1; return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun() { return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun() { }"));

    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { break; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { continue; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { break; var a = 1; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { continue; var a = 1; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { return null; var a = 1; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function() { return null; return null; };"), CompileException);

    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { break; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { continue; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { break; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { continue; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { return null; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun() { return null; return null; }"), CompileException);
}

TEST(SemanticAnalysis, WhileBlock) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { var a = 1; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { var a = 1; return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { break; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { continue; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { var a = 1; break; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { var a = 1; continue; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { }"));

    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { break; break; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { continue; continue; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { break; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { continue; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { return null; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { return null; return null; }"), CompileException);
}

TEST(SemanticAnalysis, DefaultBlock) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    //之前环境不在循环中
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { var a = 1; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { var a = 1; return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { return null; }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { }"));

    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { break; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { continue; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { break; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { continue; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { return null; var a = 1; }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"if (true) { return null; return null; }"), CompileException);

    //之前环境在循环中
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { var a = 1; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { var a = 1; return null; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { return null; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { break; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { continue; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { var a = 1; break; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { var a = 1; continue; } }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { } }"));

    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { break; break; } }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { continue; continue; } }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { break; var a = 1; } }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { continue; var a = 1; } }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { return null; var a = 1; } }"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"while (true) { if (true) { return null; return null; } }"), CompileException);
}

TEST(SemanticAnalysis, Function) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    //参数环境变量定义
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function(a, b) { var c = null; };"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun(a, b) { var c = null; }"));
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function(a, b) { var a = null; };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun(a, b) { var a = null; }"), CompileException);

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function(a, b, c) { };"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"function fun(a, b, c) { }"));
    EXPECT_THROW(TestSemanticAnalysis(names, L"var fun = function(a, a, a) { };"), CompileException);
    EXPECT_THROW(TestSemanticAnalysis(names, L"function fun(a, a, a) { }"), CompileException);
}

TEST(SemanticAnalysis, Array) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[1]; var a = arr[1];"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[reg1]; var a = arr[1];"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[1]; var a = arr[reg2];"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[reg1]; var a = arr[reg2];"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = 1; var arr = array[b]; var a = arr[b];"));
}

TEST(SemanticAnalysis, Object) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; var a = o.o;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; o.o = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; o.id = false; var a = array[o.id];"));
}

TEST(SemanticAnalysis, SpecialOperation) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"reg1(1,1);"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"reg1(1);"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"reg1[1] = reg1[1];"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"reg1.id = reg1.id;"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[1]; arr[1] = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[1]; arr.i = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var arr = array[1]; arr();"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; o.id = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; o[1] = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var o = object; o();"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function(){ }; fun();"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function(){ }; fun.id = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var fun = function(){ }; fun[1] = 1;"));
}

TEST(SemanticAnalysis, Condition) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (true) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (reg1) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (reg1) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (1) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"while (1) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var a = 1; if (a) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var a = 1; while (a) { }"));
}

TEST(SemanticAnalysis, Closure) {
    auto names = vector<wstring>{L"reg1", L"reg2"};

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var a = false; if (a) { a = function() { }; a(); } a();"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"if (true) { reg1 = false; } reg1(); reg1[1] = 1;"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = false; if (true) { while (true) { b = 1; } while (b) { } }"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var reg1 = false; if (true) { if (true) { reg1 = 1; reg2 = 1; } } if(reg1) { } if(reg2) { reg2(); }"));
}

TEST(SemanticAnalysis, Expression) {
    auto names = vector<wstring>{L"reg1", L"reg2"};
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = null == null; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = true == false; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = 1.1 == 1.1; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = 1 == 1; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = 'c' == 'c'; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = \"str\" == \"str\"; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = array[1] == array[1]; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = object == object; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = function(){ } == function(){ }; while(b) { }"));
    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = reg1 == null; while(b) { }"));

    EXPECT_NO_THROW(TestSemanticAnalysis(names, L"var b = (1 + 2) * 3 + 5 <= 7 || 1 / 2 == 0; while(b) { }"));
}