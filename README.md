# Compiler
 
环境 Win10 VS2019(需要googleTest) 
大约 7000 行

流程
词法分析 语法分析 语义分析 生成代码 虚拟机执行

关键字 
null  false  true  var  if  else  while  function  array  object  break  continue  return

符号
!   +   -   *   /   %   <   <=   >   >=   !=   ==   ( )   [ ]   { }   .   ;  

注释
//单行注释
/*
    多行注释
*/

运算符优先级 <br>
1 || && <br>
2 != == <br>
3 < <= > >= <br>
4 + - <br>
5 * / % <br>

语法
var a = null;  <br>
var b = false; <br>
var c = true;  <br>
var d = 1;  // int   <br>
var e = 1.2 // float <br>
var f = array[5];    <br>
var g = function(p1, p2){    //不可递归  <br>
    return 0;                            <br>
};                                       <br>
function h(p1, p2){          //可以递归  <br>
    while(true){                         <br>
        if(p1){                          <br>
            continue;                    <br>
        } else if(p2) {                  <br>
            break;                       <br>
        } else {                         <br>
            return null;                 <br>
        }
    }                                    <br>
    //默认return null;                   <br>
}                                        <br>
var i = object;                          <br>
i.i = 1;                                 <br>
i.o = 2;                                 <br>

更多请观看 单元测试 和 demo文件
