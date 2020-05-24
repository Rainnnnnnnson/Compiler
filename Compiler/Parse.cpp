#include "Parse.h"
#include<set>
#include<array>
using namespace Parse;
using std::make_pair;
using std::array;
using std::pair;
using std::set;

struct DFANode {
    DFANode(set<NFANode*> nfaNodes) : nfaNodes(std::move(nfaNodes)) {}
    set<NFANode*> nfaNodes;
};
bool operator<(const DFANode& l, const DFANode& r) {
    return l.nfaNodes < r.nfaNodes;
}
bool operator==(const DFANode& l, const DFANode& r) {
    return l.nfaNodes == r.nfaNodes;
}

struct DFAState {
    DFAState(DFANode dfaNode, GenerateLATypeFunction generate) : dfaNode(std::move(dfaNode)), generate(generate) {}
    DFANode dfaNode;
    GenerateLATypeFunction generate = NotGenerateLAType;
};
bool operator<(const DFAState& l, const DFAState& r) {
    return std::tie(l.dfaNode, l.generate) < std::tie(r.dfaNode, r.generate);
}
bool operator==(const DFAState& l, const DFAState& r) {
    return std::tie(l.dfaNode, l.generate) == std::tie(r.dfaNode, r.generate);
}

class GenerateDFAProcess {
public:
    GenerateDFAProcess(NFA nfa) : dfaStateIndex(0), compositeNFA(std::move(nfa)) {
        //��ȡ���ܻ���ֵ��ַ�
        for (auto& node : compositeNFA.nodes) {
            for (auto edge : node->edges) {
                characterSet.insert(edge.character);
            }
        }
        auto nfaFirstNode = &*compositeNFA.nodes[0];
        dfaStates.push_back(DFAState(DFANode(set<NFANode*>{nfaFirstNode}), nfaFirstNode->generate));
    }

    void Run() {
        while (dfaStateIndex < dfaStates.size()) {
            CheckCurrentDFAState();
            UpdateByCharacterSet();
            UpdateByAnyoneCharacterEdge();
            dfaStateIndex += 1;
        }
    }

    DFA GenerateDFA() {
        //����NFANode ���������ɺ���
        vector<GenerateLATypeFunction> generates;
        for (auto& dfaState : dfaStates) {
            generates.push_back(dfaState.generate);
        }
        return DFA(std::move(transformMap), std::move(generates));
    }

private:
    void CheckCurrentDFAState();
    void UpdateByCharacterSet();
    void UpdateByAnyoneCharacterEdge();
private:
    size_t dfaStateIndex;
    map<DFATransformData, size_t> transformMap;
    vector<DFAState> dfaStates;

    set<wchar_t> characterSet;
    NFA compositeNFA;
};

void GenerateDFAProcess::CheckCurrentDFAState() {
    auto& currentDFAState = dfaStates[dfaStateIndex];

    //DFAState���ֻ�ܴ���һ��anyoneCharacterEdge
    int anyoneCharacterEdgeCount = 0;
    for (auto nfaNode : currentDFAState.dfaNode.nfaNodes) {
        if (nfaNode->anyoneCharacterEdgeNode != nullptr) {
            anyoneCharacterEdgeCount += 1;
            if (anyoneCharacterEdgeCount > 1) {
                throw CompilerError();
            }
        }
    }
}

void GenerateDFAProcess::UpdateByCharacterSet() {
    //�����ַ������
    for (auto character : characterSet) {
        auto& currentDFAState = dfaStates[dfaStateIndex];

        set<NFANode*> nfaNodes;

        //�ӵ�ǰDFAState���� Ѱ������ͬһ���ַ���NFANode ����set
        for (auto nfaNode : currentDFAState.dfaNode.nfaNodes) {
            for (auto edge : nfaNode->edges) {
                if (edge.character == character) {
                    nfaNodes.insert(edge.node);
                }
            }
        }

        //�ռ��ϴ��� û�п����ɵ�DFAState ֱ�ӱ�����һ���ַ�
        if (nfaNodes.empty()) {
            continue;
        }

        /*
            ����ü���ѡ���ĸ����ɺ���
            Id���ȼ����������ؼ��ֺ���
            ������ǰ���� if8 �������� if �� int(8)
            �����ں����� if8 �������� id(if8)
        */
        GenerateLATypeFunction generate = NotGenerateLAType;
        for (auto nfaNode : nfaNodes) {
            if (nfaNode->generate == NotGenerateLAType) {
                continue;
            }
            if (nfaNode->generate != GenerateLAType<Id>) {
                generate = nfaNode->generate;
                break;
            }
            generate = GenerateLAType<Id>;
        }

        auto newDFAState = DFAState(DFANode(std::move(nfaNodes)), generate);

        //�ж��Ƿ������ͬ��DFAState
        size_t nextDFAStateIndex = 0;
        bool exist = false;
        for (auto& dfaState : dfaStates) {
            if (dfaState == newDFAState) {
                exist = true;
                break;
            }
            nextDFAStateIndex += 1;
        }

        //����    ������·�� 
        //������  �򼯺�������DFAState �����Ӹ�·��
        if (exist) {
            transformMap[DFATransformData(dfaStateIndex, character)] = nextDFAStateIndex;
        } else {
            transformMap[DFATransformData(dfaStateIndex, character)] = dfaStates.size();
            dfaStates.push_back(std::move(newDFAState));
        }

    }
}

void GenerateDFAProcess::UpdateByAnyoneCharacterEdge() {
    auto& currentDFAState = dfaStates[dfaStateIndex];


    NFANode* newNFANode = nullptr;
    auto generate = NotGenerateLAType;

    //Ѱ�ҵ�ǰDFAState�������ַ��ߴ����Ľ��
    //��¼�����ɺ���
    for (auto nfaNode : currentDFAState.dfaNode.nfaNodes) {
        auto anyoneCharacterEdgeNode = nfaNode->anyoneCharacterEdgeNode;
        if (anyoneCharacterEdgeNode != nullptr) {
            generate = anyoneCharacterEdgeNode->generate;
            newNFANode = anyoneCharacterEdgeNode;
            break;
        }
    }

    //��û�������ַ��ߵĽ��������
    if (newNFANode == nullptr) {
        return;
    }

    auto newDFAState = DFAState(DFANode(set<NFANode*>{newNFANode}), generate);

    //�ж��Ƿ������ͬ��DFAState
    size_t nextDFAStateIndex = 0;
    bool exist = false;
    for (auto& dfaState : dfaStates) {
        if (dfaState == newDFAState) {
            exist = true;
            break;
        }
        nextDFAStateIndex += 1;
    }

    //����    ������·�� 
    //������  �򼯺�������DFAState �����Ӹ�·��
    if (exist) {
        transformMap[DFATransformData(dfaStateIndex)] = nextDFAStateIndex;
    } else {
        transformMap[DFATransformData(dfaStateIndex)] = dfaStates.size();
        dfaStates.push_back(std::move(newDFAState));
    }
}

class CreateLATypeProcess : ParseVisitor {
public:
    CreateLATypeProcess() : line(1) {}
    unique_ptr<LAType> Handle(GenerateLATypeFunction generate, wstring str) {
        this->str = std::move(str);
        auto ptr = generate();
        //��ִ���ڼ��к� ��֤�ַ���ͷ����¼λ��
        ptr->Accept(*this);
        for (auto c : this->str) {
            if (c == '\n') {
                line += 1;
            }
        }
        return ptr;
    }
    int GetLine() {
        return line;
    }
    wstring GetString() {
        return str;
    }
    void VisitLAType(LAType& type) override {
        type.line = line;
    }
    void Visit(Id& type) override {
        type.value = std::move(str);
        type.line = line;
    }
    void Visit(Int& type) override {
        try {
            type.value = std::stoi(str);
        } catch (exception) {
            throw ParseException("�޷�ת��Ϊint����");
        }
        type.line = line;
    }
    void Visit(Float& type) override {
        try {
            type.value = std::stof(str);
        } catch (exception) {
            throw ParseException("�޷�ת��Ϊfloat����");
        }
        type.line = line;
    }
    void Visit(Char& type) override {
        wchar_t result = '\0';
        if (str[1] == L'\\') {
            switch (str[2]) {
                case L't':
                    result = L'\t';
                    break;
                case L'r':
                    result = L'\r';
                    break;
                case L'n':
                    result = L'\n';
                    break;
                case L'\\':
                    result = L'\\';
                    break;
                case L'\'':
                    result = L'\'';
                    break;
                case L'"':
                    result = L'\"';
                    break;
                default:
                    throw CompilerError();
            }
        } else {
            result = str[1];
        }
        type.value = result;
        type.line = line;
    }
    void Visit(String& type) override {
        wstring result;
        if (str.size() == 2) {
            type.value = L"";
            return;
        }
        auto iter = str.begin() + 1;
        auto endIter = str.end() - 1;
        while (iter < endIter) {
            if (*iter == L'\\') {
                switch (*(iter + 1)) {
                    case L't':
                        result += L'\t';
                        break;
                    case L'r':
                        result += L'\r';
                        break;
                    case L'n':
                        result += L'\n';
                        break;
                    case L'\\':
                        result += L'\\';
                        break;
                    case L'\'':
                        result += L'\'';
                        break;
                    case L'"':
                        result += L'\"';
                        break;
                    default:
                        throw CompilerError();
                }
                iter += 2;
            } else {
                result += *iter;
                iter += 1;
            }
        }
        type.value = result;
        type.line = line;
    }
private:
    wstring str;
    int line;
};

NFA CreateNFAId() {
    /*
                                      [a-zA-Z_0-9]
                                       (ѭ������)
        0------------>(1)---------------->(2)
           [A-Za-z_]        [A-Za-z_0-9]
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 3; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring setNotNumber = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
    wstring setHasNumber = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789";
    for (auto character : setNotNumber) {
        nfaNodes[0]->Push(NFAEdge(character, &*nfaNodes[1]));
    }
    for (auto character : setHasNumber) {
        nfaNodes[1]->Push(NFAEdge(character, &*nfaNodes[2]));
        nfaNodes[2]->Push(NFAEdge(character, &*nfaNodes[2]));
    }
    nfaNodes[1]->generate = GenerateLAType<Id>;
    nfaNodes[2]->generate = GenerateLAType<Id>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFAInt() {
    /*
                [0-9]
              (ѭ������)
        0-------->(1)
           [0-9]

        0-------->2-------->(1)
           [-]       [0-9]
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 3; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring numberSet = L"0123456789";
    nfaNodes[0]->Push(NFAEdge(L'-', &*nfaNodes[2]));
    for (auto character : numberSet) {
        nfaNodes[0]->Push(NFAEdge(character, &*nfaNodes[1]));
        nfaNodes[1]->Push(NFAEdge(character, &*nfaNodes[1]));
        nfaNodes[2]->Push(NFAEdge(character, &*nfaNodes[1]));
    }
    nfaNodes[1]->generate = GenerateLAType<Int>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFAFloat() {
    /*
                [0-9]                [0-9]
              (ѭ������)           (ѭ������)
        0-------->1-------->2--------->(3)
           [0-9]     "."      [0-9]

        0-------->4-------->1
            [-]      [0-9]
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 5; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring numberSet = L"0123456789";
    for (auto character : numberSet) {
        nfaNodes[0]->Push(NFAEdge(character, &*nfaNodes[1]));
        nfaNodes[1]->Push(NFAEdge(character, &*nfaNodes[1]));
        nfaNodes[2]->Push(NFAEdge(character, &*nfaNodes[3]));
        nfaNodes[3]->Push(NFAEdge(character, &*nfaNodes[3]));
        nfaNodes[4]->Push(NFAEdge(character, &*nfaNodes[1]));
    }
    nfaNodes[0]->Push(NFAEdge(L'-', &*nfaNodes[4]));
    nfaNodes[1]->Push(NFAEdge(L'.', &*nfaNodes[2]));
    nfaNodes[3]->generate = GenerateLAType<Float>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFABlank() {
    /*
                  [ \t\r\n]
                  (ѭ������)
        0------------>(1)
            [ \t\r\n]
        (�ո� �Ʊ�� �س� ����)
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 2; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring characterSet = L" \t\r\n";
    for (auto character : characterSet) {
        nfaNodes[0]->Push(NFAEdge(character, &*nfaNodes[1]));
        nfaNodes[1]->Push(NFAEdge(character, &*nfaNodes[1]));
    }
    nfaNodes[1]->generate = GenerateLAType<Blank>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFASingleLineComment() {
    /*
                        [����]
                      (ѭ������)
        0------->1------->(2)-------->(3)
           [/]      [/]        [\n]

           ���һ�л����û�л��з������ ����2Ҳ���ս��־
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 4; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    nfaNodes[0]->Push(NFAEdge(L'/', &*nfaNodes[1]));
    nfaNodes[1]->Push(NFAEdge(L'/', &*nfaNodes[2]));
    nfaNodes[2]->anyoneCharacterEdgeNode = &*nfaNodes[2];
    nfaNodes[2]->Push(NFAEdge(L'\n', &*nfaNodes[3]));
    nfaNodes[2]->generate = GenerateLAType<Blank>;
    nfaNodes[3]->generate = GenerateLAType<Blank>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFAMultiLineComments() {
    /*
                        [����]      [*]
                      (ѭ������)  (ѭ������)
        0------->1------->2--------->3------>(4)
           [/]      [*]      [*]        [/]

        3------->2
          [����]

    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 5; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    nfaNodes[0]->Push(NFAEdge(L'/', &*nfaNodes[1]));
    nfaNodes[1]->Push(NFAEdge(L'*', &*nfaNodes[2]));
    nfaNodes[2]->Push(NFAEdge(L'*', &*nfaNodes[3]));
    nfaNodes[2]->anyoneCharacterEdgeNode = &*nfaNodes[2];
    nfaNodes[3]->Push(NFAEdge(L'*', &*nfaNodes[3]));
    nfaNodes[3]->Push(NFAEdge(L'/', &*nfaNodes[4]));
    nfaNodes[3]->anyoneCharacterEdgeNode = &*nfaNodes[2];
    nfaNodes[4]->generate = GenerateLAType<Blank>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFAChar() {
    /*

        0------->1------->2--------->3------->(4)
           [']      [\]      [trn'"\]      [']

        1-------->3
          [����]

        1-------->5
           ['����]

        ֱ��ʹ�� ������ ���з� �ı��ϻ���� ������ת���ַ�
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 6; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring characterSet = L"trn'\"\\";
    nfaNodes[0]->Push(NFAEdge(L'\'', &*nfaNodes[1]));
    nfaNodes[1]->Push(NFAEdge(L'\\', &*nfaNodes[2]));
    nfaNodes[1]->Push(NFAEdge(L'\n', &*nfaNodes[5]));
    nfaNodes[1]->Push(NFAEdge(L'\'', &*nfaNodes[5]));
    nfaNodes[1]->anyoneCharacterEdgeNode = &*nfaNodes[3];
    for (auto character : characterSet) {
        nfaNodes[2]->Push(NFAEdge(character, &*nfaNodes[3]));
    }
    nfaNodes[3]->Push(NFAEdge(L'\'', &*nfaNodes[4]));
    nfaNodes[4]->generate = GenerateLAType<Char>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

NFA CreateNFAString() {
    /*
               [����]
             (ѭ������)
        0------->1--------->(2)
           ["]      ["]

        1------->3---------->1
            [\]      [trn'"\]

        1-------->4
           ["\n]

        ֱ��ʹ�� ˫���� ���з� �ı��ϻ���� ������ת���ַ�
    */
    vector<unique_ptr<NFANode>> nfaNodes;
    for (int i = 0; i < 5; i++) {
        nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    }
    wstring characterSet = L"trn'\"\\";
    nfaNodes[0]->Push(NFAEdge(L'\"', &*nfaNodes[1]));
    nfaNodes[1]->Push(NFAEdge(L'\"', &*nfaNodes[2]));
    nfaNodes[1]->Push(NFAEdge(L'\\', &*nfaNodes[3]));
    nfaNodes[1]->Push(NFAEdge(L'"', &*nfaNodes[4]));
    nfaNodes[1]->Push(NFAEdge(L'\n', &*nfaNodes[4]));
    nfaNodes[1]->anyoneCharacterEdgeNode = &*nfaNodes[1];
    for (auto character : characterSet) {
        nfaNodes[3]->Push(NFAEdge(character, &*nfaNodes[1]));
    }
    nfaNodes[2]->generate = GenerateLAType<String>;
    NFANode* start = &*nfaNodes[0];
    return NFA(start, std::move(nfaNodes));
}

unique_ptr<LAType> NotGenerateLAType() {
    throw ParseException();
}

NFA CompositeNFA(vector<NFA>&& nfas) {
    vector<unique_ptr<NFANode>> nfaNodes;
    nfaNodes.push_back(make_unique<NFANode>(nullptr, NotGenerateLAType));
    auto start = &*nfaNodes[0];
    for (auto& nfa : nfas) {

        //����ʼ���ı��ƶ����½ڵ�
        auto root = &*nfa.nodes[0];
        for (auto edge : root->edges) {
            start->Push(edge);
        }
        //���ӵڶ����ڵ��Ժ��ָ���ƶ���������
        auto iter = nfa.nodes.begin() + 1;
        auto end = nfa.nodes.end();
        while (iter != end) {
            nfaNodes.push_back(std::move(*iter));
            iter++;
        }
    }

    return NFA(start, std::move(nfaNodes));
}

DFA CreateDFA(NFA&& nfa) {
    auto generateDFAProcess = GenerateDFAProcess(std::move(nfa));
    generateDFAProcess.Run();
    return generateDFAProcess.GenerateDFA();
}

DFA CreateDefaultDFA() {
    vector<NFA> nfas;
    nfas.push_back(CreateNFAId());
    nfas.push_back(CreateNFAChar());
    nfas.push_back(CreateNFAInt());
    nfas.push_back(CreateNFAFloat());
    nfas.push_back(CreateNFAString());
    nfas.push_back(CreateNFABlank());
    nfas.push_back(CreateNFASingleLineComment());
    nfas.push_back(CreateNFAMultiLineComments());

    nfas.push_back(CreateNFA<Null>(L"null"));
    nfas.push_back(CreateNFA<False>(L"false"));
    nfas.push_back(CreateNFA<True>(L"true"));
    nfas.push_back(CreateNFA<Var>(L"var"));
    nfas.push_back(CreateNFA<Function>(L"function"));
    nfas.push_back(CreateNFA<Array>(L"array"));
    nfas.push_back(CreateNFA<Object>(L"object"));
    nfas.push_back(CreateNFA<If>(L"if"));
    nfas.push_back(CreateNFA<Else>(L"else"));
    nfas.push_back(CreateNFA<While>(L"while"));
    nfas.push_back(CreateNFA<Break>(L"break"));
    nfas.push_back(CreateNFA<Continue>(L"continue"));
    nfas.push_back(CreateNFA<Return>(L"return"));

    nfas.push_back(CreateNFA<Add>(L"+"));
    nfas.push_back(CreateNFA<Subtract>(L"-"));
    nfas.push_back(CreateNFA<Multiply>(L"*"));
    nfas.push_back(CreateNFA<Divide>(L"/"));
    nfas.push_back(CreateNFA<Modulus>(L"%"));
    nfas.push_back(CreateNFA<Less>(L"<"));
    nfas.push_back(CreateNFA<LessEquals>(L"<="));
    nfas.push_back(CreateNFA<Greater>(L">"));
    nfas.push_back(CreateNFA<GreaterEquals>(L">="));
    nfas.push_back(CreateNFA<NotEquals>(L"!="));
    nfas.push_back(CreateNFA<DoubleEquals>(L"=="));
    nfas.push_back(CreateNFA<DoubleAnd>(L"&&"));
    nfas.push_back(CreateNFA<DoubleOr>(L"||"));

    nfas.push_back(CreateNFA<Equals>(L"="));
    nfas.push_back(CreateNFA<Not>(L"!"));

    nfas.push_back(CreateNFA<ParentheseSmallLeft>(L"("));
    nfas.push_back(CreateNFA<ParentheseSmallRight>(L")"));
    nfas.push_back(CreateNFA<ParentheseMediumLeft>(L"["));
    nfas.push_back(CreateNFA<ParentheseMediumRight>(L"]"));
    nfas.push_back(CreateNFA<ParentheseBigLeft>(L"{"));
    nfas.push_back(CreateNFA<ParentheseBigRight>(L"}"));

    nfas.push_back(CreateNFA<Period>(L"."));
    nfas.push_back(CreateNFA<Comma>(L","));
    nfas.push_back(CreateNFA<Semicolon>(L";"));

    return CreateDFA(CompositeNFA(std::move(nfas)));
}

LexicalAnalysisResult LexicalAnalysis(const DFA& dfa, const wstring& str) {
    if (str.empty()) {
        vector<unique_ptr<LAType>> v;
        v.push_back(make_unique<TextEnd>());
        return LexicalAnalysisResult(std::move(v));
    }
    //��ȡ�����ַ����ַ���
    set<wchar_t> characterSet;
    for (auto& data : dfa.transform) {
        auto character = data.first.character;
        characterSet.insert(character);
    }

    vector<unique_ptr<LAType>> typeList;
    CreateLATypeProcess process;
    try {
        //ͨ������DFA�ıߵõ��ַ���
        auto clipStrBegin = str.begin();
        auto clipStrEnd = str.begin();
        size_t index = 0;
        const auto strEnd = str.end();
        while (clipStrEnd != strEnd) {
            auto character = *clipStrEnd;

            //Ѱ�ҵ�һ���ַ���
            auto iter = dfa.transform.find(DFATransformData(index, character));
            if (iter != dfa.transform.end()) {
                index = iter->second;
                clipStrEnd += 1;
                continue;
            }

            //Ѱ�ҵ�һ�������ַ���
            iter = dfa.transform.find(DFATransformData(index));
            if (iter != dfa.transform.end()) {
                index = iter->second;
                clipStrEnd += 1;
                continue;
            }

            //�ַ����д��ڸ��ַ� ��û�ҵ���ֱ�ӽ�β
            if (characterSet.find(character) != characterSet.end()) {
                typeList.push_back(process.Handle(dfa.generates[index], wstring(clipStrBegin, clipStrEnd)));
                clipStrBegin = clipStrEnd;
                index = 0;
                continue;
            }

            //�쳣
            throw ParseException(wstring(clipStrBegin, clipStrEnd));
        }
        //�����ټ����β����
        typeList.push_back(process.Handle(dfa.generates[index], wstring(clipStrBegin, clipStrEnd)));
        typeList.push_back(process.Handle(GenerateLAType<TextEnd>, L""));
    } catch (ParseException e) {
        auto s = WstringToString(process.GetString());
        throw ParseException(MessageHead(process.GetLine()) + s + "  " + e.what());
    }
    return typeList;
}

//--------------------------------------------------------------------------------

class CreateNullableFirstFollowTableProcess {
public:
    CreateNullableFirstFollowTableProcess(const vector<Production>& parProductions)
        : productions(parProductions) {
        for (auto& production : productions) {
            table[production.head];
        }
    }
    NullableFirstFollowTable Value()&& {
        SetUpProductionNullable();
        SetUpNullable();
        SetUpFirstSet();
        SetUpFollowSetFromFirstSet();
        SetUpFollowSetFromFollowSet();
        return NullableFirstFollowTable(std::move(table));
    }
private:
    bool SetAddSet(set<type_index>& to, const set<type_index>& form);
    bool FirstAddFirst(type_index to, type_index form);
    bool FollowAddFirst(type_index to, type_index form);
    bool FollowAddFollow(type_index to, type_index form);
    bool IsNullable(type_index type);

    void SetUpProductionNullable();
    void SetUpNullable();
    void SetUpFirstSet();
    void SetUpFollowSetFromFirstSet();
    void SetUpFollowSetFromFollowSet();
private:
    const vector<Production>& productions;
    map<type_index, NullableFirstFollow> table;
};

bool CreateNullableFirstFollowTableProcess::SetAddSet(set<type_index>& to, const set<type_index>& form) {
    bool changed = false;
    for (auto& item : form) {
        changed |= to.insert(item).second;
    }
    return changed;
}

bool CreateNullableFirstFollowTableProcess::FirstAddFirst(type_index to, type_index form) {
    bool isEnd = (table.find(form) == table.end());
    auto& toSet = table[to].firstSet;
    if (isEnd) {
        set<type_index> formSet{form};
        return SetAddSet(toSet, formSet);
    } else {
        auto& formSet = table[form].firstSet;
        return SetAddSet(toSet, formSet);
    }
}

bool CreateNullableFirstFollowTableProcess::FollowAddFirst(type_index to, type_index form) {
    //follow���ϲ������ս��
    if (table.find(to) == table.end()) {
        return false;
    }
    bool isEnd = (table.find(form) == table.end());
    auto& toSet = table[to].followSet;
    if (isEnd) {
        set<type_index> formSet{form};
        return SetAddSet(toSet, formSet);
    } else {
        auto& formSet = table[form].firstSet;
        return SetAddSet(toSet, formSet);
    }
}

bool CreateNullableFirstFollowTableProcess::FollowAddFollow(type_index to, type_index form) {
    //follow���ϲ������ս��
    if (table.find(to) == table.end()) {
        return false;
    }
    if (table.find(form) == table.end()) {
        return false;
    }
    auto& formSet = table[form].followSet;
    auto& toSet = table[to].followSet;
    return SetAddSet(toSet, formSet);
}

bool CreateNullableFirstFollowTableProcess::IsNullable(type_index type) {
    //�����Ҳ���˵��Ϊ�ս�� �ս�����ɿ�
    auto iter = table.find(type);
    if (iter == table.end()) {
        return false;
    }
    return iter->second.nullable;
}

void CreateNullableFirstFollowTableProcess::SetUpProductionNullable() {
    //����ʽ����ɿ� ����ֱ�ӱ�ʶΪ��
    for (auto& production : productions) {
        if (production.result.empty()) {
            table[production.head].nullable = true;
        }
    }
}

void CreateNullableFirstFollowTableProcess::SetUpNullable() {
    //һֱ����ֱ��û���κθı�
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& production : productions) {

            //�жϱ��пɿ�
            bool& tableNullable = table.find(production.head)->second.nullable;
            if ((tableNullable == true)) {
                continue;
            }

            //�жϲ���ʽ���Ƿ�ȫ���ɿ�
            bool productionResultAllNullable = true;
            for (auto& item : production.result) {
                if (!IsNullable(item)) {
                    productionResultAllNullable = false;
                    break;
                }
            }

            //�ж��Ƿ���ı�
            if ((tableNullable == false) && (productionResultAllNullable == true)) {
                tableNullable = true;
                changed = true;
            }
        }
    }
}

void CreateNullableFirstFollowTableProcess::SetUpFirstSet() {
    //һֱ����ֱ��û���κθı�
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& production : productions) {
            if (production.result.empty()) {
                continue;
            }
            /*
                X -> Y[0] Y[1] Y[2] Y[3]....Y[k]
                �� Y[0] Y[1] Y[2] Y[3]....Y[i-1] �ɿ�
                �� First[X] += First[Y[i]]

                First[X] += First[Y[0]] һ��ִ��
            */
            auto& X = production.head;
            auto& Y = production.result;
            int64_t last = production.result.size() - 1;

            changed |= FirstAddFirst(X, Y[0]);
            for (int64_t i = 1; i <= last; i++) {
                if (IsNullable(Y[i - 1])) {
                    changed |= FirstAddFirst(X, Y[i]);
                } else {
                    break;
                }
            }
        }
    }
}

void CreateNullableFirstFollowTableProcess::SetUpFollowSetFromFirstSet() {
    //һֱ����ֱ��û���κθı�
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& production : productions) {
            if (production.result.empty()) {
                continue;
            }
            /*
                X -> Y[0] Y[1] Y[2] Y[3]....Y[k]
                �� Y[i+1] ....Y[j-1] �ɿ�
                �� Follow[Y[i]] += First[Y[j]]

                Follow[Y[i]] += First[Y[i+1]] һ����ִ��
            */
            auto& X = production.head;
            auto& Y = production.result;
            int64_t iLast = production.result.size() - 2;
            int64_t jLast = production.result.size() - 1;

            vector<bool> nullables;
            for (auto& item : production.result) {
                nullables.push_back(IsNullable(item));
            }

            for (int64_t i = 0; i <= iLast; i++) {
                changed |= FollowAddFirst(Y[i], Y[i + 1]);
                for (int64_t j = i + 2; j <= jLast; j++) {
                    if (nullables[j - 1]) {
                        changed |= FollowAddFirst(Y[i], Y[j]);
                    } else {
                        break;
                    }
                }
            }
        }
    }
}

void CreateNullableFirstFollowTableProcess::SetUpFollowSetFromFollowSet() {
    //һֱ����ֱ��û���κθı�
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& production : productions) {
            if (production.result.empty()) {
                continue;
            }
            /*
                X -> Y[0] Y[1] Y[2] Y[3]....Y[k]
                �� Y[i+1] ....Y[k] �ɿ�
                �� Follow[Y[i]] += Follow[X]

                Follow[Y[k]] += Follow[X]
            */
            auto& X = production.head;
            auto& Y = production.result;
            int64_t last = production.result.size() - 1;

            changed |= FollowAddFollow(Y[last], X);
            for (int64_t i = last - 1; i >= 0; i--) {
                if (IsNullable(Y[i + 1])) {
                    changed |= FollowAddFollow(Y[i], X);
                } else {
                    break;
                }
            }
        }
    }
}

NullableFirstFollowTable CreateNullableFirstFollowTable(const vector<Production>& productions) {
    return CreateNullableFirstFollowTableProcess(productions).Value();
}

NotBlankLATypeResult LexicalAnalysisResultRemoveBlank(LexicalAnalysisResult&& lexicalAnalysisResult) {
    vector<unique_ptr<LAType>> resultList;
    for (auto& item : lexicalAnalysisResult.resultList) {
        if (typeid(*item) != typeid(Blank)) {
            resultList.push_back(std::move(item));
        }
    }
    return NotBlankLATypeResult(std::move(resultList));
}

PredictiveParsingTable CreatePredictiveParsingTable(NullableFirstFollowTable&& table, vector<Production>&& productions, int startIndex) {
    map<PredictiveParsingTableSelect, const Production*> resultTable;
    //�������� �������ظ��� ���򱨴�
    for (auto& production : productions) {
        //����ʽΪ�� 
        if (production.result.empty()) {
            auto head = production.head;
            auto& followSet = table.table[head].followSet;
            for (auto item : followSet) {
                if (resultTable.insert(pair(PredictiveParsingTableSelect(head, item), &production)).second == false) {
                    throw CompilerError();
                }
            }
            continue;
        }
        auto firstItem = production.result[0];
        auto find = table.table.find(firstItem);
        //�ս��
        if (find == table.table.end()) {
            auto head = production.head;
            if (resultTable.insert(pair(PredictiveParsingTableSelect(head, firstItem), &production)).second == false) {
                throw CompilerError();
            }
            continue;
        }
        //���ɿ�:ȡ��First����
        //  �ɿ�:ȡ��Follow����
        set<type_index>* selectSet = nullptr;
        if (find->second.nullable) {
            selectSet = &find->second.followSet;
        } else {
            selectSet = &find->second.firstSet;
        }
        for (auto item : *selectSet) {
            auto head = production.head;
            if (resultTable.insert(pair(PredictiveParsingTableSelect(head, item), &production)).second == false) {
                throw CompilerError();
            }
        }
    }
    auto& start = productions[startIndex];
    return PredictiveParsingTable(std::move(resultTable), start, std::move(productions));
}

PredictiveParsingTable CreateDefaultPredictiveParsingTable() {
    //�뿴 ParseType.h ��ͷ�ǲ���
    vector<Production> vec{
        CreateProduction<Text, StatementNullable, TextEnd>(),

        CreateProduction<StatementDefineFunction, Function, Id, FunctionParameter, StatementBlock>(),
        CreateProduction<StatementDefineVariable, Var, Id, Equals, Expression, Semicolon>(),
        CreateProduction<StatementOperate, UnknownOperate, AssignmentNullable, Semicolon>(),
        CreateProduction<StatementIf, If, Condition, StatementBlock, IfNullable>(),
        CreateProduction<StatementWhile, While, Condition, StatementBlock>(),
        CreateProduction<StatementBreak, Break, Semicolon>(),
        CreateProduction<StatementContinue, Continue, Semicolon>(),
        CreateProduction<StatementReturn, Return, Expression, Semicolon>(),
        CreateProduction<StatementNext, Statement, StatementNullable>(),
        CreateProduction<StatementBlock, ParentheseBigLeft, StatementNullable, ParentheseBigRight>(),
        CreateProduction<StatementNullable, StatementNext>(),
        CreateProduction<StatementNullable>(),
        CreateProduction<Statement, StatementDefineFunction>(),
        CreateProduction<Statement, StatementDefineVariable>(),
        CreateProduction<Statement, StatementOperate>(),
        CreateProduction<Statement, StatementIf>(),
        CreateProduction<Statement, StatementWhile>(),
        CreateProduction<Statement, StatementBreak>(),
        CreateProduction<Statement, StatementContinue>(),
        CreateProduction<Statement, StatementReturn>(),

        CreateProduction<Condition, ParentheseSmallLeft, Expression, ParentheseSmallRight>(),
        CreateProduction<IfNullable, IfNext>(),
        CreateProduction<IfNullable>(),
        CreateProduction<IfNext, Else, ElseNext>(),
        CreateProduction<ElseNext, StatementIf>(),
        CreateProduction<ElseNext, StatementBlock>(),

        CreateProduction<AssignmentNullable, Assignment>(),
        CreateProduction<AssignmentNullable>(),
        CreateProduction<Assignment, Equals, Expression>(),

        CreateProduction<ExpressionEnd, ExpressionNot>(),
        CreateProduction<ExpressionEnd, ExpressionBrackets>(),
        CreateProduction<ExpressionEnd, Unknown>(),
        CreateProduction<ExpressionNot, Not, Unknown>(),
        CreateProduction<ExpressionBrackets, ParentheseSmallLeft, Expression, ParentheseSmallRight>(),

        CreateProduction<Unknown, Type>(),
        CreateProduction<Unknown, UnknownOperate>(),
        CreateProduction<UnknownOperate, Id, UnknownNullable>(),
        CreateProduction<UnknownNullable, UnknownNext>(),
        CreateProduction<UnknownNullable>(),
        CreateProduction<UnknownNext, UnknownOperateNode, UnknownNullable>(),
        CreateProduction<UnknownOperateNode, AccessArray>(),
        CreateProduction<UnknownOperateNode, AccessObject>(),
        CreateProduction<UnknownOperateNode, FunctionCall>(),

        CreateProduction<Type, Null>(),
        CreateProduction<Type, Bool>(),
        CreateProduction<Type, Char>(),
        CreateProduction<Type, Int>(),
        CreateProduction<Type, Float>(),
        CreateProduction<Type, String>(),
        CreateProduction<Type, Object>(),
        CreateProduction<Type, ArrayType>(),
        CreateProduction<Type, FunctionType>(),
        CreateProduction<Bool, False>(),
        CreateProduction<Bool, True>(),

        CreateProduction<ArrayType, Array, AccessArray>(),
        CreateProduction<FunctionType, Function, FunctionParameter, StatementBlock>(),
        CreateProduction<AccessObject, Period, Id>(),
        CreateProduction<AccessArray, ParentheseMediumLeft, Expression, ParentheseMediumRight>(),
        CreateProduction<FunctionCall, ParentheseSmallLeft, ExpressionListNullable, ParentheseSmallRight>(),
        CreateProduction<FunctionParameter, ParentheseSmallLeft, IdListNullable, ParentheseSmallRight>(),

        CreateProduction<ExpressionListNullable, ExpressionListNotNull>(),
        CreateProduction<ExpressionListNullable>(),
        CreateProduction<ExpressionListNotNull, Expression, ExpressionListNextNullable>(),
        CreateProduction<ExpressionListNextNullable, Comma, Expression, ExpressionListNextNullable>(),
        CreateProduction<ExpressionListNextNullable>(),

        CreateProduction<IdListNullable, IdListNotNull>(),
        CreateProduction<IdListNullable>(),
        CreateProduction<IdListNotNull, Id, IdListNextNullable>(),
        CreateProduction<IdListNextNullable, Comma, Id, IdListNextNullable>(),
        CreateProduction<IdListNextNullable>(),
    };
    vector<Production> sign{
        CreateProduction<ExpressionSign<5>, DoubleOr>(),
        CreateProduction<ExpressionSign<5>, DoubleAnd>(),

        CreateProduction<ExpressionSign<4>, DoubleEquals>(),
        CreateProduction<ExpressionSign<4>, NotEquals>(),

        CreateProduction<ExpressionSign<3>, LessEquals>(),
        CreateProduction<ExpressionSign<3>, GreaterEquals>(),
        CreateProduction<ExpressionSign<3>, Less>(),
        CreateProduction<ExpressionSign<3>, Greater>(),

        CreateProduction<ExpressionSign<2>, Add>(),
        CreateProduction<ExpressionSign<2>, Subtract>(),

        CreateProduction<ExpressionSign<1>, Multiply>(),
        CreateProduction<ExpressionSign<1>, Divide>(),
        CreateProduction<ExpressionSign<1>, Modulus>(),
    };
    auto recursive = CreateProductionSignLevel<5>();
    for (int i = 0; i < sign.size(); i++) {
        vec.push_back(std::move(sign[i]));
    }
    for (int i = 0; i < recursive.size(); i++) {
        vec.push_back(std::move(recursive[i]));
    }
    auto nullableFirstFollowTable = CreateNullableFirstFollowTable(vec);
    return CreatePredictiveParsingTable(std::move(nullableFirstFollowTable), std::move(vec), 0);
}

GenerateSATypeFunctionMap CreateDefaultGenerateSATypeFunctionMap() {
#define DerivedSAType(Type) pair(type_index(typeid(Type)), GenerateSAType<Type>)
    map<type_index, GenerateSATypeFunction> generateMap{
        DerivedSAType(Text),
        DerivedSAType(StatementDefineFunction),
        DerivedSAType(StatementDefineVariable),
        DerivedSAType(StatementOperate),
        DerivedSAType(StatementIf),
        DerivedSAType(StatementWhile),
        DerivedSAType(StatementBreak),
        DerivedSAType(StatementContinue),
        DerivedSAType(StatementReturn),
        DerivedSAType(StatementNext),
        DerivedSAType(StatementBlock),
        DerivedSAType(StatementNullable),
        DerivedSAType(Statement),
        DerivedSAType(Condition),
        DerivedSAType(IfNullable),
        DerivedSAType(IfNext),
        DerivedSAType(ElseNext),
        DerivedSAType(AssignmentNullable),
        DerivedSAType(Assignment),
        DerivedSAType(Expression),
        DerivedSAType(ExpressionEnd),
        DerivedSAType(ExpressionNot),
        DerivedSAType(ExpressionBrackets),
        DerivedSAType(Unknown),
        DerivedSAType(UnknownOperate),
        DerivedSAType(UnknownOperateNode),
        DerivedSAType(UnknownNullable),
        DerivedSAType(UnknownNext),
        DerivedSAType(Type),
        DerivedSAType(Bool),
        DerivedSAType(ArrayType),
        DerivedSAType(FunctionType),
        DerivedSAType(AccessObject),
        DerivedSAType(AccessArray),
        DerivedSAType(FunctionCall),
        DerivedSAType(FunctionParameter),
        DerivedSAType(ExpressionListNullable),
        DerivedSAType(ExpressionListNotNull),
        DerivedSAType(ExpressionListNextNullable),
        DerivedSAType(IdListNullable),
        DerivedSAType(IdListNotNull),
        DerivedSAType(IdListNextNullable),
        DerivedSAType(ExpressionLevel<0>),
#define TemplateDerivedSAType(N)              \
		DerivedSAType(ExpressionLevel<N>),    \
		DerivedSAType(ExpressionNode<N>),     \
		DerivedSAType(ExpressionNullable<N>), \
		DerivedSAType(ExpressionNext<N>),     \
		DerivedSAType(ExpressionSign<N>),
        TemplateDerivedSAType(5)
        TemplateDerivedSAType(4)
        TemplateDerivedSAType(3)
        TemplateDerivedSAType(2)
        TemplateDerivedSAType(1)
#undef TemplateDerivedSAType
    };
#undef DerivedSAType
    return GenerateSATypeFunctionMap(std::move(generateMap));
}
//-------------------------------------------------------------------------------------------------

class CreateParseTreeProcess {
public:
    CreateParseTreeProcess(const PredictiveParsingTable& table, const GenerateSATypeFunctionMap& generateMap, NotBlankLATypeResult&& result)
        : table(table), generateMap(generateMap), terminalList(std::move(result)), index(0) {}
    ParseTree operator()()&& {
        auto root = Recursive(table.start);
        return ParseTree(std::move(root));
    }
    unique_ptr<ParseType> Recursive(const Production& production) {
        //��������ʽ�������
        //�жϵ�ǰ �ս�� ������ 
        auto ptr = generateMap.generateMap.find(production.head)->second();
        for (auto& item : production.result) {
            //���ս����ֱͬ�Ӽ���
            auto& iter = *terminalList.resultList[index];
            if (item == type_index(typeid(iter))) {
                ptr->parseTypes.push_back(std::move(terminalList.resultList[index]));
                index += 1;
                continue;
            }
            //�����Ƿ������һ����ʽ
            auto find = table.table.find(PredictiveParsingTableSelect(item, type_index(typeid(iter))));
            if (find == table.table.end()) {
                throw ParseException(MessageHead(iter.line) + "�﷨��������");
            } else {
                ptr->parseTypes.push_back(Recursive(*find->second));
            }
        }
        //��¼��һ����Ϊ�к�
        if (!ptr->parseTypes.empty()) {
            ptr->line = ptr->parseTypes[0]->line;
        }
        return std::move(ptr);
    }
private:
    const PredictiveParsingTable& table;
    const GenerateSATypeFunctionMap& generateMap;
    NotBlankLATypeResult terminalList;
    int index;
};

ParseTree CreateParseTree(const PredictiveParsingTable& table, const GenerateSATypeFunctionMap& generateMap, NotBlankLATypeResult&& result) {
    return CreateParseTreeProcess(table, generateMap, std::move(result))();
}


//--------------------------------------------------------------------------------------------------
//�����������16������
const int functionParameterCountMax = 16;

/*
    Builder��β������һ��AbstractSyntaxType�Ľ�� �� �ý����Ҫ������
    Transform��β����ת��֮��һ��ParseVisitor
*/

class StatementBlockTransform : public ParseVisitor {
public:
    void Visit(Parse::StatementNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Parse::StatementNext& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Parse::Statement& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Parse::StatementBreak& type) override {
        auto p = make_unique<AbstractSyntax::StatementBreak>();
        p->line = type.line;
        ptr->statements.push_back(std::move(p));
    }
    void Visit(Parse::StatementContinue& type) override {
        auto p = make_unique<AbstractSyntax::StatementContinue>();
        p->line = type.line;
        ptr->statements.push_back(std::move(p));
    }
    void Visit(Parse::ParentheseBigLeft& type) override {}
    void Visit(Parse::ParentheseBigRight& type) override {}

    void Visit(Parse::StatementDefineFunction& type) override;
    void Visit(Parse::StatementDefineVariable& type) override;
    void Visit(Parse::StatementOperate& type) override;
    void Visit(Parse::StatementIf& type) override;
    void Visit(Parse::StatementWhile& type) override;
    void Visit(Parse::StatementReturn& type) override;
protected:
    AbstractSyntax::StatementBlock* ptr = nullptr;
};

class MainBlockBuilder : public StatementBlockTransform {
public:
    AbstractSyntax::MainBlock operator()(ParseType& type) {
        ptr = &result;
        type.Accept(*this);
        result.line = type.line;
        return std::move(result);
    }
    void Visit(Text& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(TextEnd& type) override {}
private:
    AbstractSyntax::MainBlock result;
};

class FunctionBlockBuilder : public StatementBlockTransform {
public:
    AbstractSyntax::FunctionBlock operator()(ParseType& type) {
        ptr = &result;
        type.Accept(*this);
        result.line = type.line;
        return std::move(result);
    }
    void Visit(StatementBlock& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
private:
    AbstractSyntax::FunctionBlock result;
};

class DefaultBlockBuilder : public StatementBlockTransform {
public:
    AbstractSyntax::DefaultBlock operator()(ParseType& type) {
        ptr = &result;
        type.Accept(*this);
        result.line = type.line;
        return std::move(result);
    }
    void Visit(StatementBlock& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
private:
    AbstractSyntax::DefaultBlock result;
};

class WhileBlockBuilder : public StatementBlockTransform {
public:
    AbstractSyntax::WhileBlock operator()(ParseType& type) {
        ptr = &result;
        type.Accept(*this);
        result.line = type.line;
        return std::move(result);
    }
    void Visit(StatementBlock& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
private:
    AbstractSyntax::WhileBlock result;
};


class ExpressionBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Expression> operator()(ParseType& type) {
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(Expression& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionLevel<5>& type) override;
private:
    unique_ptr<AbstractSyntax::Expression> result;
};

class ConditionTransform : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Expression> operator()(ParseType& type) {
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(Condition& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ParentheseSmallLeft& type) override {}
    void Visit(Expression& type) override {
        result = ExpressionBuilder()(type);
    }
    void Visit(ParentheseSmallRight& type) override {}
private:
    unique_ptr<AbstractSyntax::Expression> result;
};

class ExpressionListBuilder : public ParseVisitor {
public:
    vector<unique_ptr<AbstractSyntax::Expression>> operator()(ParseType& type) {
        type.Accept(*this);
        return std::move(result);
    }
    void Visit(ExpressionListNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionListNotNull& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionListNextNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Expression& type) override {
        result.push_back(ExpressionBuilder()(type));
    }
    void Visit(Comma& type) override {}
private:
    vector<unique_ptr<AbstractSyntax::Expression>> result;
};

class IdListBuilder : public ParseVisitor {
public:
    vector<wstring> operator()(ParseType& type) {
        type.Accept(*this);
        return std::move(result);
    }
    void Visit(IdListNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(IdListNotNull& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(IdListNextNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Id& type) override {
        result.push_back(std::move(type.value));
    }
    void Visit(Comma& type) override {}
private:
    vector<wstring> result;
};

//������ͨ��ParseType���� ������AST��������

class FunctionCallBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::SpecialOperation> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::FunctionCall>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(FunctionCall& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ParentheseSmallLeft& type) override {}
    void Visit(ExpressionListNullable& type) override {
        result->expressionList = ExpressionListBuilder()(type);
        if (result->expressionList.size() > functionParameterCountMax) {
            throw ParseException(MessageHead(type.line) + "�������ò�������ܳ���" + std::to_string(functionParameterCountMax));
        }
    }
    void Visit(ParentheseSmallRight& type) override {}
private:
    unique_ptr<AbstractSyntax::FunctionCall> result;
};

class AccessArrayBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::SpecialOperation> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::AccessArray>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(AccessArray& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ParentheseMediumLeft& type) override {}
    void Visit(Expression& type) override {
        result->index = ExpressionBuilder()(type);
    }
    void Visit(ParentheseMediumRight& type) override {}
private:
    unique_ptr<AbstractSyntax::AccessArray> result;
};

class AccessFieldBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::SpecialOperation> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::AccessField>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(AccessObject& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Period& type) override {}
    void Visit(Id& type) override {
        result->id = std::move(type.value);
    }
private:
    unique_ptr<AbstractSyntax::AccessField> result;
};

class FunctionBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Function> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::Function>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(FunctionType& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(FunctionParameter& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Function& type) override {}
    void Visit(ParentheseSmallLeft& type) override {}
    void Visit(IdListNullable& type) override {
        result->idList = IdListBuilder()(type);
        if (result->idList.size() > functionParameterCountMax) {
            throw ParseException(MessageHead(type.line) + "����������������ܳ���" + std::to_string(functionParameterCountMax));
        }
    }
    void Visit(ParentheseSmallRight& type) override {}
    void Visit(StatementBlock& type) override {
        result->functionBlock = FunctionBlockBuilder()(type);
    }
private:
    unique_ptr<AbstractSyntax::Function> result;
};

class ArrayBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Array> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::Array>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(ArrayType& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(AccessArray& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Array& type) override {}
    void Visit(ParentheseMediumLeft& type) override {}
    void Visit(Expression& type) override {
        result->length = ExpressionBuilder()(type);
    }
    void Visit(ParentheseMediumRight& type) override {}
private:
    unique_ptr<AbstractSyntax::Array> result;
};

class SpecialOperationListBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::SpecialOperationList> operator()(ParseType& type) {
        type.Accept(*this);
        auto p = make_unique<AbstractSyntax::SpecialOperationList>();
        p->line = type.line;
        p->id = std::move(id);
        p->specialOperations = std::move(specialOperations);
        return p;
    }
    void Visit(UnknownOperate& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(UnknownNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(UnknownNext& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(UnknownOperateNode& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(FunctionCall& type) override {
        specialOperations.push_back(FunctionCallBuilder()(type));
    }
    void Visit(AccessArray& type) override {
        specialOperations.push_back(AccessArrayBuilder()(type));
    }
    void Visit(AccessObject& type) override {
        specialOperations.push_back(AccessFieldBuilder()(type));
    }
    void Visit(Id& type) override {
        id = std::move(type.value);
    }
private:
    wstring id;
    vector<unique_ptr<AbstractSyntax::SpecialOperation>> specialOperations;
};

class BinaryOperationBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::BinaryOperation> operator()(ParseType& type) {
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(DoubleOr& type) override {
        result = make_unique<AbstractSyntax::Or>();
    }
    void Visit(DoubleAnd& type) override {
        result = make_unique<AbstractSyntax::And>();
    }
    void Visit(DoubleEquals& type) override {
        result = make_unique<AbstractSyntax::Equals>();
    }
    void Visit(NotEquals& type) override {
        result = make_unique<AbstractSyntax::NotEquals>();
    }
    void Visit(Less& type) override {
        result = make_unique<AbstractSyntax::Less>();
    }
    void Visit(LessEquals& type) override {
        result = make_unique<AbstractSyntax::LessEquals>();
    }
    void Visit(Greater& type) override {
        result = make_unique<AbstractSyntax::Greater>();
    }
    void Visit(GreaterEquals& type) override {
        result = make_unique<AbstractSyntax::GreaterEquals>();
    }
    void Visit(Add& type) override {
        result = make_unique<AbstractSyntax::Add>();
    }
    void Visit(Subtract& type) override {
        result = make_unique<AbstractSyntax::Subtract>();
    }
    void Visit(Multiply& type) override {
        result = make_unique<AbstractSyntax::Multiply>();
    }
    void Visit(Divide& type) override {
        result = make_unique<AbstractSyntax::Divide>();
    }
    void Visit(Modulus& type) override {
        result = make_unique<AbstractSyntax::Modulus>();
    }
private:
    unique_ptr<AbstractSyntax::BinaryOperation> result;
};

class UnknownBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Expression> operator()(ParseType& type) {
        type.Accept(*this);
        return std::move(result);
    }
    void Visit(Unknown& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(UnknownOperate& type) override {
        result = SpecialOperationListBuilder()(type);
    }
    void Visit(Type& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Null& type) override {
        auto p = make_unique<AbstractSyntax::Null>();
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(Bool& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(False& type) override {
        auto p = make_unique<AbstractSyntax::Bool>();
        p->value = false;
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(True& type) override {
        auto p = make_unique<AbstractSyntax::Bool>();
        p->value = true;
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(Char& type) override {
        auto p = make_unique<AbstractSyntax::Char>();
        p->value = type.value;
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(Int& type) override {
        auto p = make_unique<AbstractSyntax::Int>();
        p->value = type.value;
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(Float& type) override {
        auto p = make_unique<AbstractSyntax::Float>();
        p->value = type.value;
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(String& type) override {
        auto p = make_unique<AbstractSyntax::String>();
        p->value = std::move(type.value);
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(Object& type) override {
        auto p = make_unique<AbstractSyntax::Object>();
        p->line = type.line;
        result = std::move(p);
    }
    void Visit(ArrayType& type) override {
        result = ArrayBuilder()(type);
    }
    void Visit(FunctionType& type) override {
        result = FunctionBuilder()(type);
    }
private:
    unique_ptr<AbstractSyntax::Expression> result;
};

template<int N>
class ExpressionLevelTemplateBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Expression> operator()(ParseType& type) {
        type.Accept(*this);
        auto node = std::move(expressions[0]);
        auto binIter = binaryOperations.begin();
        auto expIter = expressions.begin() + 1;
        while (binIter != binaryOperations.end()) {
            (*binIter)->left = std::move(node);
            (*binIter)->right = std::move(*expIter);
            node = std::move(*binIter);
            expIter += 1;
            binIter += 1;
        }
        return node;
    }
    void Visit(ExpressionLevel<N>& type) {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionNode<N>& type) {
        expressions.push_back(ExpressionLevelTemplateBuilder<N - 1>()(*type.parseTypes[0]));
    }
    void Visit(ExpressionNullable<N>& type) {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionNext<N>& type) {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionSign<N>& type) {
        binaryOperations.push_back(BinaryOperationBuilder()(*type.parseTypes[0]));
    }
protected:
    vector<unique_ptr<AbstractSyntax::Expression>> expressions;
    vector<unique_ptr<AbstractSyntax::BinaryOperation>> binaryOperations;
};

template<>
class ExpressionLevelTemplateBuilder<0> : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Expression> operator()(ParseType& type) {
        type.Accept(*this);
        return std::move(result);
    }
    void Visit(ExpressionLevel<0>& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ExpressionEnd& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Unknown& type) override {
        result = UnknownBuilder()(type);
    }
    void Visit(ExpressionNot& type) override {
        auto p = make_unique<AbstractSyntax::Not>();
        p->expression = UnknownBuilder()(*type.parseTypes[1]);
        result = std::move(p);
    }
    void Visit(ExpressionBrackets& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ParentheseSmallLeft& type) override {}
    void Visit(Expression& type) override {
        result = ExpressionBuilder()(type);
    }
    void Visit(ParentheseSmallRight& type) override {}
private:
    unique_ptr<AbstractSyntax::Expression> result;
};

/*
    ע������̳���AbstractSyntax::AbstractSyntaxVisitor
    �����﷨���ɵ�ParseTree�Ǵ������ҵ�
    ������Ҫ�жϴ��������һ��AST�Ľ��
*/
class StatementAssignmentBuilder : public AbstractSyntax::AbstractSyntaxVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(unique_ptr<AbstractSyntax::SpecialOperationList> specialOperationList,
                                                     unique_ptr<AbstractSyntax::Expression> expression) {
        //û��������� ֻ��һ��Id
        if (specialOperationList->specialOperations.empty()) {
            auto p = make_unique<AbstractSyntax::StatementAssignmentId>();
            p->id = std::move(specialOperationList->id);
            p->expression = std::move(expression);
            return std::move(p);
        }
        //�����������Ҫ�ж����һ�� Ȼ���Ƴ�
        this->specialOperationList = std::move(specialOperationList);
        this->expression = std::move(expression);
        lastSpecialOperation = std::move(this->specialOperationList->specialOperations.back());
        this->specialOperationList->specialOperations.pop_back();
        lastSpecialOperation->Accept(*this);
        return std::move(result);
    }
    void Visit(AbstractSyntax::FunctionCall& type) override {
        throw ParseException(MessageHead(type.line) + "��ֵ������಻�Ե��ý�β");
    }
    void Visit(AbstractSyntax::AccessArray& type) override {
        auto p = make_unique<AbstractSyntax::StatementAssignmentArray>();
        p->specialOperationList = std::move(specialOperationList);
        p->index = std::move(type.index);
        p->expression = std::move(expression);
        result = std::move(p);
    }
    void Visit(AbstractSyntax::AccessField& type) override {
        auto p = make_unique<AbstractSyntax::StatementAssignmentField>();
        p->specialOperationList = std::move(specialOperationList);
        p->field = std::move(type.id);
        p->expression = std::move(expression);
        result = std::move(p);
    }
private:
    unique_ptr<AbstractSyntax::SpecialOperationList> specialOperationList;
    unique_ptr<AbstractSyntax::SpecialOperation> lastSpecialOperation;
    unique_ptr<AbstractSyntax::Expression> expression;
    unique_ptr<AbstractSyntax::Statement> result;
};

class StatementAssignmentTransform : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        type.Accept(*this);
        auto p = StatementAssignmentBuilder()(std::move(specialOperationList), std::move(expression));
        p->line = type.line;
        return std::move(p);
    }
    void Visit(StatementOperate& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(UnknownOperate& type) override {
        specialOperationList = SpecialOperationListBuilder()(type);
    }
    void Visit(AssignmentNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Assignment& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Equals& type) override {}
    void Visit(Expression& type) override {
        expression = ExpressionBuilder()(type);
    }
    void Visit(Semicolon& type) override {}
private:
    unique_ptr<AbstractSyntax::SpecialOperationList> specialOperationList;
    unique_ptr<AbstractSyntax::Expression> expression;
};

class StatementCallBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        type.Accept(*this);
        if (result->specialOperationList->specialOperations.empty()) {
            throw ParseException(MessageHead(type.line) + "�Ǹ�ֵ�������Ժ������ý�β");
        }
        if (typeid(*result->specialOperationList->specialOperations.back()) != typeid(AbstractSyntax::FunctionCall)) {
            throw ParseException(MessageHead(type.line) + "�Ǹ�ֵ�������Ժ������ý�β");
        }
        result->line = type.line;
        return std::move(result);
    }
    void Visit(StatementOperate& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(AssignmentNullable& type) override {}
    void Visit(Semicolon& type) override {}
    void Visit(UnknownOperate& type) override {
        auto p = make_unique<AbstractSyntax::StatementCall>();
        p->specialOperationList = SpecialOperationListBuilder()(type);
        result = std::move(p);
    }
private:
    unique_ptr<AbstractSyntax::StatementCall> result;
};

class StatementOperateTransform : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        ptr = &type;
        type.Accept(*this);
        return std::move(result);
    }
    void Visit(StatementOperate& type) override {
        type.parseTypes[1]->Accept(*this);
    }
    void Visit(AssignmentNullable& type) override {
        if (type.parseTypes.empty()) {
            result = StatementCallBuilder()(*ptr);
        } else {
            result = StatementAssignmentTransform()(*ptr);
        }
    }
private:
    ParseType* ptr = nullptr;
    unique_ptr<AbstractSyntax::Statement> result;
};

class StatementDefineFunctionBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::StatementDefineFunction>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(StatementDefineFunction& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(FunctionParameter& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Function& type) override {}
    void Visit(Id& type) override {
        result->id = std::move(type.value);
    }
    void Visit(ParentheseSmallLeft& type) override {}
    void Visit(IdListNullable& type) override {
        result->idList = IdListBuilder()(type);
    }
    void Visit(ParentheseSmallRight& type) override {}
    void Visit(StatementBlock& type) override {
        result->functionBlock = FunctionBlockBuilder()(type);
    }
private:
    unique_ptr<AbstractSyntax::StatementDefineFunction> result;
};

class StatementDefineVariableBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::StatementDefineVariable>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(StatementDefineVariable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Var& type) override {}
    void Visit(Id& type) override {
        result->id = std::move(type.value);
    }
    void Visit(Equals& type) override {}
    void Visit(Expression& type) override {
        result->expression = ExpressionBuilder()(type);
    }
    void Visit(Semicolon& type) override {}
private:
    unique_ptr<AbstractSyntax::StatementDefineVariable> result;
};

class StatementIfBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::StatementIf>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    /*
        ��Ҫ���������������ֶ�ν���ͬһ����
        1. if (true) { }
        2. if (true) { } else { }
        3. if (true) { } else     <�ݹ�1����2>
    */
    void Visit(StatementIf& type) override {
        if (number == 1) {
            for (auto& item : type.parseTypes) {
                item->Accept(*this);
            }
        } else if (number == 2) {
            AbstractSyntax::DefaultBlock defualtBlock;
            defualtBlock.statements.push_back(StatementIfBuilder()(type));
            result->elseBlock = std::move(defualtBlock);
        } else {
            throw CompilerError();
        }
    }
    void Visit(StatementBlock& type) override {
        if (number == 1) {
            result->ifBlock = DefaultBlockBuilder()(type);
        } else if (number == 2) {
            result->elseBlock = DefaultBlockBuilder()(type);
        } else {
            throw CompilerError();
        }
        number += 1;
    }
    void Visit(If& type) override {}
    void Visit(Condition& type) override {
        result->condition = ConditionTransform()(type);
    }
    void Visit(IfNullable& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(IfNext& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(ElseNext& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Else& type) override {}
    void Visit(Semicolon& type) override {}
private:
    int number = 1;
    unique_ptr<AbstractSyntax::StatementIf> result;
};

class StatementWhileBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::StatementWhile>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(StatementWhile& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(While& type) override {}
    void Visit(Condition& type) override {
        result->condition = ConditionTransform()(type);
    }
    void Visit(StatementBlock& type) override {
        result->whileBlock = WhileBlockBuilder()(type);
    }
private:
    unique_ptr<AbstractSyntax::StatementWhile> result;
};

class StatementReturnBuilder : public ParseVisitor {
public:
    unique_ptr<AbstractSyntax::Statement> operator()(ParseType& type) {
        result = make_unique<AbstractSyntax::StatementReturn>();
        type.Accept(*this);
        result->line = type.line;
        return std::move(result);
    }
    void Visit(StatementReturn& type) override {
        for (auto& item : type.parseTypes) {
            item->Accept(*this);
        }
    }
    void Visit(Return& type) override {}
    void Visit(Expression& type) override {
        result->expression = ExpressionBuilder()(type);
    }
    void Visit(Semicolon& type) override {}
private:
    unique_ptr<AbstractSyntax::StatementReturn> result;
};

void StatementBlockTransform::Visit(StatementDefineFunction& type) {
    ptr->statements.push_back(StatementDefineFunctionBuilder()(type));
}
void StatementBlockTransform::Visit(StatementDefineVariable& type) {
    ptr->statements.push_back(StatementDefineVariableBuilder()(type));
}
void StatementBlockTransform::Visit(StatementOperate& type) {
    ptr->statements.push_back(StatementOperateTransform()(type));
}
void StatementBlockTransform::Visit(StatementIf& type) {
    ptr->statements.push_back(StatementIfBuilder()(type));
}
void StatementBlockTransform::Visit(Parse::StatementWhile& type) {
    ptr->statements.push_back(StatementWhileBuilder()(type));
}
void StatementBlockTransform::Visit(StatementReturn& type) {
    ptr->statements.push_back(StatementReturnBuilder()(type));
}
void ExpressionBuilder::Visit(ExpressionLevel<5>& type) {
    result = ExpressionLevelTemplateBuilder<5>()(type);
}

AbstractSyntaxTree CreateAbstractSyntaxTree(const ParseTree& parseTree) {
    return AbstractSyntaxTree(MainBlockBuilder()(*parseTree.root));
}

class RegisteredNameGenerateProcess : public ParseVisitor {
public:
    wstring operator()(LAType& type) {
        type.Accept(*this);
        return std::move(name);
    }
    void VisitLAType(LAType& type) override {
        throw ParseException();
    }
    void Visit(Id& type) override {
        name = std::move(type.value);
    }
private:
    wstring name;
};

RegisteredNameList CreateRegisteredNameList(const DFA& dfa, const vector<wstring>& registeredNames) {
    set<wstring> nameSet(registeredNames.begin(), registeredNames.end());
    if (registeredNames.size() != nameSet.size()) {
        throw ParseException("ע��������ظ�");
    }
    vector<wstring> nameList;
    for (auto& name : nameSet) {
        if (name.empty()) {
            throw ParseException("ע�������Ϊ��");
        }
        try {
            LexicalAnalysisResult laResult = LexicalAnalysis(dfa, name);
            if (laResult.resultList.size() != 2) {
                throw ParseException();
            }
            nameList.push_back(RegisteredNameGenerateProcess()(*laResult.resultList[0]));
        } catch (ParseException) {
            throw ParseException("ע�����������������������ȷ :" + WstringToString(name));
        }
    }
    return RegisteredNameList(std::move(nameList));
}

//------------------------------------------------------------------------------