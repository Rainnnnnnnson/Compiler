#pragma once
#include<string>
using std::string;
using std::wstring;
using std::exception;

inline string MessageHead(int line) {
    return "�����к� : " + std::to_string(line) + "\n";
}

inline string WstringToString(const wstring& wstr) {
    size_t count = 0;
    char arr[1024];
    wcstombs_s(&count, arr, 1024, wstr.c_str(), wstr.size());
    auto s = string(arr, count);
    return arr;
}

/*
    ����д������ҪDEBUG
*/
class CompilerError : public exception {
public:
    inline CompilerError() : str("����������\n") {}
    inline CompilerError(const string& str) : str("����������\n" + str) {}
    inline CompilerError(const wstring& wstr) : str("����������\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    �����ַ��������쳣ʹ��
*/
class ParseException : public exception {
public:
    inline ParseException() : str("�����쳣\n") {}
    inline ParseException(const string& str) : str("�����쳣\n" + str) {}
    inline ParseException(const wstring& wstr) : str("�����쳣\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    �������ɴ�������쳣ʹ��
*/
class CompileException : public exception {
public:
    inline CompileException() : str("�����쳣\n") {}
    inline CompileException(const string& str) : str("�����쳣\n" + str) {}
    inline CompileException(const wstring& wstr) : str("�����쳣\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    ���������������������쳣ʹ��
*/
class ConfigurationException : public exception {
public:
    inline ConfigurationException() : str("�����쳣\n") {}
    inline ConfigurationException(const string& str) : str("�����쳣\n" + str) {}
    inline ConfigurationException(const wstring& wstr) : str("�����쳣\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    ����VM�����쳣ʱ��ʹ��
*/
class RuntimeException : public exception {
public:
    inline RuntimeException() : str("�����쳣\n") {}
    inline RuntimeException(const string& str) : str("�����쳣\n" + str) {}
    inline RuntimeException(const wstring& wstr) : str("�����쳣\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};