#include "Complie.h"
CompileData::CompileData()
    : dfa(CreateDefaultDFA()),
    table(CreateDefaultPredictiveParsingTable()),
    generateMap(CreateDefaultGenerateSATypeFunctionMap()) {}

VMRuntimeData GenerateVMRuntimeData(const wstring& text, const CompileData& data, const vector<wstring>& registeredNames) {
    auto la = LexicalAnalysis(data.dfa, text);
    auto la2 = LexicalAnalysisResultRemoveBlank(std::move(la));
    auto pt = CreateParseTree(data.table, data.generateMap, std::move(la2));
    auto namelist = CreateRegisteredNameList(data.dfa, registeredNames);
    auto ast = CreateAbstractSyntaxTree(pt);
    auto result = SemanticAnalysis(namelist, std::move(ast));
    auto registeredNameList = RegisteredNameList(registeredNames);
    return CreateVMRuntimeData(registeredNameList, std::move(result));
}

