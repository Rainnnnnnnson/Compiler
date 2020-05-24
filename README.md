# Compiler
 
环境 Win10 VS2019(需要googleTest) <br>
大约 7000 行<br>



流程  <br>
词法分析 语法分析 语义分析 生成代码 虚拟机执行<br>

关键字 <br>
null  false  true  var  if  else  while  function  array  object  break  continue  return  <br>

符号  <br>
!   +   -   *   /   %   <   <=   >   >=   !=   ==   ( )   [ ]   { }   .   ;  <br>

注释        <br>
//单行注释  <br>
/*           <br>
    多行注释 <br>
*/            <br>

运算符优先级 <br>
1 || && <br>
2 != == <br>
3 < <= > >= <br>
4 + - <br>
5 * / % <br>

语法
```
var a = null;  <br>
var b = false; <br>
var c = true;  <br>
var d = 1;  // int   <br>
var e = 1.2 // float <br>
var f = array[5];    <br>
var g = function(p1, p2){    //不可递归  
    return 0;                            
};                                      
function h(p1, p2){          //可以递归 
    while(true){                        
        if(p1){                          
            continue;                  
        } else if(p2) {                  
            break;                     
        } else {                        
            return null;                
        }
    }                                    
    //默认return null;                  
}                                     
var i = object;                          
i.i = 1;                                
i.o = 2;                                 
```
更多请观看 单元测试 和 demo文件
