#pragma once
#include<string>
using std::string;
using std::wstring;
using std::exception;

inline string MessageHead(int line) {
    return "错误行号 : " + std::to_string(line) + "\n";
}

inline string WstringToString(const wstring& wstr) {
    size_t count = 0;
    char arr[1024];
    wcstombs_s(&count, arr, 1024, wstr.c_str(), wstr.size());
    auto s = string(arr, count);
    return arr;
}

/*
    代码写错了需要DEBUG
*/
class CompilerError : public exception {
public:
    inline CompilerError() : str("编译器错误\n") {}
    inline CompilerError(const string& str) : str("编译器错误\n" + str) {}
    inline CompilerError(const wstring& wstr) : str("编译器错误\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    解析字符串出现异常使用
*/
class ParseException : public exception {
public:
    inline ParseException() : str("解析异常\n") {}
    inline ParseException(const string& str) : str("解析异常\n" + str) {}
    inline ParseException(const wstring& wstr) : str("解析异常\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    编译生成代码出现异常使用
*/
class CompileException : public exception {
public:
    inline CompileException() : str("编译异常\n") {}
    inline CompileException(const string& str) : str("编译异常\n" + str) {}
    inline CompileException(const wstring& wstr) : str("编译异常\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    配置虚拟机参数错误出现异常使用
*/
class ConfigurationException : public exception {
public:
    inline ConfigurationException() : str("配置异常\n") {}
    inline ConfigurationException(const string& str) : str("配置异常\n" + str) {}
    inline ConfigurationException(const wstring& wstr) : str("配置异常\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};

/*
    运行VM出现异常时候使用
*/
class RuntimeException : public exception {
public:
    inline RuntimeException() : str("运行异常\n") {}
    inline RuntimeException(const string& str) : str("运行异常\n" + str) {}
    inline RuntimeException(const wstring& wstr) : str("运行异常\n" + WstringToString(wstr)) {}
    inline virtual char const* what() const {
        return str.c_str();
    }
private:
    string str;
};