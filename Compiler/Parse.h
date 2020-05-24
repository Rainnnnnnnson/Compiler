#pragma once
#include "ParseType.h"
#include "AbstractSyntax.h"
#include "CompilerException.h"
#include <typeindex>
#include <memory>
#include <vector>
#include <map>
#include <set>
using std::make_unique;
using std::type_index;
using std::unique_ptr;
using std::vector;
using std::map;
using std::set;
using Parse::ParseVisitor;
using Parse::ParseType;
using Parse::LAType;
using Parse::SAType;

struct NFA;
struct DFA;
struct LexicalAnalysisResult;
struct Production;
struct PredictiveParsingTable;
struct NullableFirstFollowTable;
struct NotBlankLATypeResult;
struct GenerateSATypeFunctionMap;
struct ParseTree;

AbstractSyntaxTree CreateAbstractSyntaxTree(const ParseTree& parseTree);
RegisteredNameList CreateRegisteredNameList(const DFA& dfa, const vector<wstring>& registeredNames);
ParseTree CreateParseTree(const PredictiveParsingTable& table, const GenerateSATypeFunctionMap& generateMap, NotBlankLATypeResult&& result);
GenerateSATypeFunctionMap CreateDefaultGenerateSATypeFunctionMap();
PredictiveParsingTable CreateDefaultPredictiveParsingTable();
PredictiveParsingTable CreatePredictiveParsingTable(NullableFirstFollowTable&& table, vector<Production>&& productions, int startIndex);
NullableFirstFollowTable CreateNullableFirstFollowTable(const vector<Production>& productions);
NotBlankLATypeResult LexicalAnalysisResultRemoveBlank(LexicalAnalysisResult&& lexicalAnalysisResult);
LexicalAnalysisResult LexicalAnalysis(const DFA& dfa, const wstring& text);
DFA CreateDefaultDFA();
DFA CreateDFA(NFA&& nfa);
NFA CompositeNFA(vector<NFA>&& nfas);

template<size_t N>
vector<Production> CreateProductionSignLevel();

template<typename Start, typename ...Next>
Production CreateProduction();

template<typename T>
NFA CreateNFA(const wstring& str);
NFA CreateNFAId();
NFA CreateNFAInt();
NFA CreateNFAFloat();
NFA CreateNFABlank();
NFA CreateNFASingleLineComment();
NFA CreateNFAMultiLineComments();
NFA CreateNFAChar();
NFA CreateNFAString();



//--------------------------------------------------
using GenerateSATypeFunction = unique_ptr<SAType>(*)();
using GenerateLATypeFunction = unique_ptr<LAType>(*)();
template<typename T>
unique_ptr<SAType> GenerateSAType();
template<typename T>
unique_ptr<LAType> GenerateLAType();
unique_ptr<LAType> NotGenerateLAType();
struct NFANode;
struct NFAEdge;
struct DFATransformData;

struct NFA {
	inline NFA(NFANode* start, vector<unique_ptr<NFANode>> nodes) : start(start), nodes(std::move(nodes)) {}
	NFANode* start;
	vector<unique_ptr<NFANode>> nodes;
};

struct NFAEdge {
	inline NFAEdge(wchar_t character, NFANode* node) : character(character), node(node) {}
	wchar_t character;
	NFANode* node;
};

struct NFANode {
	inline NFANode(NFANode* excludeEdgeAnyone, GenerateLATypeFunction generate) : anyoneCharacterEdgeNode(excludeEdgeAnyone), generate(generate) {}
	inline void Push(NFAEdge edge) {
		edges.push_back(edge);
	}
	vector<NFAEdge> edges;
	NFANode* anyoneCharacterEdgeNode;
	GenerateLATypeFunction generate;
};

struct DFATransformData {
	inline DFATransformData(size_t index) : index(index), anyoneCharacterEdge(true), character(L'\0') {}
	inline DFATransformData(size_t index, wchar_t character) : index(index), anyoneCharacterEdge(false), character(character) {}
	size_t index;
	bool anyoneCharacterEdge;
	wchar_t character;
};
inline bool operator<(const DFATransformData& l, const DFATransformData& r) {
	return std::tie(l.index, l.character, l.anyoneCharacterEdge) < std::tie(r.index, r.character, r.anyoneCharacterEdge);
}

struct DFA {
	inline DFA(map<DFATransformData, size_t> transform, vector<GenerateLATypeFunction> generates)
		: transform(std::move(transform)), generates(std::move(generates)) {}
	map<DFATransformData, size_t> transform;
	vector<GenerateLATypeFunction> generates;
};

struct LexicalAnalysisResult {
	inline LexicalAnalysisResult(vector<unique_ptr<LAType>> resultList) : resultList(std::move(resultList)) {}
	vector<unique_ptr<LAType>> resultList;
};

struct NotBlankLATypeResult {
	inline NotBlankLATypeResult(vector<unique_ptr<LAType>> resultList) : resultList(std::move(resultList)) {}
	vector<unique_ptr<LAType>> resultList;
};

template<size_t N>
inline void CreateProductionSignLevelIter(vector<Production>& productions) {
	using namespace Parse;
	if constexpr (N != 0) {
		productions.push_back(CreateProduction<ExpressionLevel<N>, ExpressionNode<N>, ExpressionNullable<N>>());
		productions.push_back(CreateProduction<ExpressionNode<N>, ExpressionLevel<N - 1>>());
		productions.push_back(CreateProduction<ExpressionNullable<N>, ExpressionNext<N>>());
		productions.push_back(CreateProduction<ExpressionNullable<N>>());
		productions.push_back(CreateProduction<ExpressionNext<N>, ExpressionSign<N>, ExpressionLevel<N>>());
		CreateProductionSignLevelIter<N - 1>(productions);
	}
}

template<size_t N>
inline vector<Production> CreateProductionSignLevel() {
	using namespace Parse;
	vector<Production> productions{
		CreateProduction<Expression, ExpressionLevel<N>>(),
		CreateProduction<ExpressionLevel<0>, ExpressionEnd>(),
	};
	CreateProductionSignLevelIter<N>(productions);
	return productions;
}

template<typename Start, typename ...Next>
inline Production CreateProduction() {
	using test = std::enable_if_t<std::is_base_of_v<SAType, Start>>;
	auto production = Production(type_index(typeid(Start)), vector<type_index>{type_index(typeid(Next))...});
	return std::move(production);
}

template<typename T>
inline NFA CreateNFA(const wstring& str) {
	/*
		一条直线一直到结尾
	*/
	vector<unique_ptr<NFANode>> nfaNodes;
	nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
	NFANode* last = &*nfaNodes[0];
	NFANode* node = nullptr;
	for (auto character : str) {
		nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
		node = &*nfaNodes.back();
		last->Push(NFAEdge(character, node));
		last = node;
	}
	last->generate = GenerateLAType<T>;

	NFANode* start = &*nfaNodes[0];
	return NFA(start, std::move(nfaNodes));
}

template<typename T>
inline unique_ptr<SAType> GenerateSAType() {
	return make_unique<T>();
}

template<typename T>
inline unique_ptr<LAType> GenerateLAType() {
	return make_unique<T>();
}

struct Production {
	inline Production(type_index start, vector<type_index> next) : head(start), result(std::move(next)) {}
	type_index head;
	vector<type_index> result;
};

struct NullableFirstFollow {
	inline NullableFirstFollow() : nullable(false) {}
	bool nullable;
	set<type_index> firstSet;
	set<type_index> followSet;
};

struct NullableFirstFollowTable {
	inline NullableFirstFollowTable(map<type_index, NullableFirstFollow> table) : table(std::move(table)) {}
	map<type_index, NullableFirstFollow> table;
};

struct PredictiveParsingTableSelect {
	inline PredictiveParsingTableSelect(type_index notEndSign, type_index endSign) : notEndSign(notEndSign), endSign(endSign) {}
	type_index notEndSign;
	type_index endSign;
};
inline bool operator<(const PredictiveParsingTableSelect& l, const PredictiveParsingTableSelect& r) {
	return std::tie(l.notEndSign, l.endSign) < std::tie(r.notEndSign, r.endSign);
}

struct PredictiveParsingTable {
	inline PredictiveParsingTable(map<PredictiveParsingTableSelect, const Production*>&& table, const Production& start, vector<Production>&& productions)
		: table(std::move(table)), start(start), productions(std::move(productions)) {}
	map<PredictiveParsingTableSelect, const Production*> table;
	const Production& start;
	vector<Production> productions;
};

struct GenerateSATypeFunctionMap {
	inline GenerateSATypeFunctionMap(map<type_index, GenerateSATypeFunction> generateMap) : generateMap(std::move(generateMap)) {}
	inline unique_ptr<SAType> Generate(type_index type) {
		auto find = generateMap.find(type);
		if (find == generateMap.end()) {
			throw CompilerError();
		}
		return find->second();
	}
	map<type_index, GenerateSATypeFunction> generateMap;
};

struct ParseTree {
	inline ParseTree(unique_ptr<ParseType> root) : root(std::move(root)) {}
	unique_ptr<ParseType> root;
};