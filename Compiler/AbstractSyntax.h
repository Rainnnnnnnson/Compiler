#pragma once
#include"AbstractSyntaxType.h"
#include"CodeGenerate.h"
#include<map>
using std::pair;
using std::map;
using AbstractSyntax::AbstractSyntaxType;

struct AbstractSyntaxTree;
struct AbstractSyntaxTreeTransform;
struct RegisteredNameList;
struct VMRuntimeData;

VMRuntimeData CreateVMRuntimeData(const RegisteredNameList& nameList, AbstractSyntaxTreeTransform&& abstractSyntaxTree);

AbstractSyntaxTreeTransform SemanticAnalysis(const RegisteredNameList& nameList, AbstractSyntaxTree&& abstractSyntaxTree);

struct RegisteredNameList {
    inline RegisteredNameList(vector<wstring> registeredNames) : registeredNames(std::move(registeredNames)) {}
    vector<wstring> registeredNames;
};

struct AbstractSyntaxTree {
    inline AbstractSyntaxTree(AbstractSyntax::MainBlock&& root) : root(std::move(root)) {}
    AbstractSyntax::MainBlock root;
};

struct AbstractSyntaxTreeTransform {
    inline AbstractSyntaxTreeTransform(AbstractSyntax::MainBlock&& root) : root(std::move(root)) {}
    AbstractSyntax::MainBlock root;
};