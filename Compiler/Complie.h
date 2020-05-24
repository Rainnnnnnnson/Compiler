#pragma once
#include "Parse.h"
#include "CodeGenerate.h"

struct CompileData {
    CompileData();
    DFA dfa;
    PredictiveParsingTable table;
    GenerateSATypeFunctionMap generateMap;
};

VMRuntimeData GenerateVMRuntimeData(const wstring& text, const CompileData& data, const vector<wstring>& registeredNames);