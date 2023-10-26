#include<stdio.h>  

/*
这一句话是必须要的格式
stdio 表示系统文件库, 也可以声明其它的
.h  表示头文件,因为这些文件都是放在程序各文件的开头
#include 告诉预处理器将指定头文件的内容插入到预处理器命令的相应位  
导入头文件的预编译指令
<> 表示系统自带的库
也可以写成" " 表示用户自定义的库
如果写成" "并且自定义的库里面没有这个文件系统会自动查找自带的库,如果还是没有报错
*/

int main() // 程序的入口
{ //程序从这里开始运行
/*
int 表示数字格式,返回一个数字
main()主函数 表示程序的入口  一个程序有且只能有一个main函数的存在
*/
printf("hello C"); //打印一个hello C
return 0; //返回一个整数0,因为它是int类型,所以只能返回整数
}  //程序从这里结束




/*
gcc -v 检查环境

gcc hello.c
生成执行文件并命名
gcc hello.c -o haa.exe
生成编译环境信息
gcc .\01-hello.c -v


*/