/*
 * 本文件是用于编码规则之间的转换，以及编码规则和字符集之间的转换。
 * 由于计算机存储常用ANSI(GBK)的编码，而网络传输常用UTF-8的编码。
 * 因此，两者的信息交互中，编码的转换必不可少。
 * 其中
 * Unicode 是「字符集」
 * UTF-8和GBK 是「编码规则」
 * 什么是字符集和编码规则：
 * 字符集：为每一个「字符」分配一个唯一的 ID（学名为码位 / 码点 / Code Point）
 * 编码规则：将「码位」转换为字节序列的规则（编码/解码 可以理解为 加密/解密 的过程）
 * 
*/


//C++ Operators
//  Operators specify an evaluation to be performed on one of the following:
//    One operand (unary operator)
//    Two operands (binary operator)
//    Three operands (ternary operator)
//  The C++ language includes all C operators and adds several new operators.
//  Table 1.1 lists the operators available in Microsoft C++.
//  Operators follow a strict precedence which defines the evaluation order of
//expressions containing these operators.  Operators associate with either the
//expression on their left or the expression on their right;    this is called
//“associativity.” Operators in the same group have equal precedence and are
//evaluated left to right in an expression unless explicitly forced by a pair of
//parentheses, ( ).
//  Table 1.1 shows the precedence and associativity of C++ operators
//  (from highest to lowest precedence).
//
//Table 1.1   C++ Operator Precedence and Associativity
// The highest precedence level is at the top of the table.
//+------------------+-----------------------------------------+---------------+
//| Operator         | Name or Meaning                         | Associativity |
//+------------------+-----------------------------------------+---------------+
//| ::               | Scope resolution                        | None          |
//| ::               | Global                                  | None          |
//| [ ]              | Array subscript                         | Left to right |
//| ( )              | Function call                           | Left to right |
//| ( )              | Conversion                              | None          |
//| .                | Member selection (object)               | Left to right |
//| ->               | Member selection (pointer)              | Left to right |
//| ++               | Postfix increment                       | None          |
//| --               | Postfix decrement                       | None          |
//| new              | Allocate object                         | None          |
//| delete           | Deallocate object                       | None          |
//| delete[ ]        | Deallocate object                       | None          |
//| ++               | Prefix increment                        | None          |
//| --               | Prefix decrement                        | None          |
//| *                | Dereference                             | None          |
//| &                | Address-of                              | None          |
//| +                | Unary plus                              | None          |
//| -                | Arithmetic negation (unary)             | None          |
//| !                | Logical NOT                             | None          |
//| ~                | Bitwise complement                      | None          |
//| sizeof           | Size of object                          | None          |
//| sizeof ( )       | Size of type                            | None          |
//| typeid( )        | type name                               | None          |
//| (type)           | Type cast (conversion)                  | Right to left |
//| const_cast       | Type cast (conversion)                  | None          |
//| dynamic_cast     | Type cast (conversion)                  | None          |
//| reinterpret_cast | Type cast (conversion)                  | None          |
//| static_cast      | Type cast (conversion)                  | None          |
//| .*               | Apply pointer to class member (objects) | Left to right |
//| ->*              | Dereference pointer to class member     | Left to right |
//| *                | Multiplication                          | Left to right |
//| /                | Division                                | Left to right |
//| %                | Remainder (modulus)                     | Left to right |
//| +                | Addition                                | Left to right |
//| -                | Subtraction                             | Left to right |
//| <<               | Left shift                              | Left to right |
//| >>               | Right shift                             | Left to right |
//| <                | Less than                               | Left to right |
//| >                | Greater than                            | Left to right |
//| <=               | Less than or equal to                   | Left to right |
//| >=               | Greater than or equal to                | Left to right |
//| ==               | Equality                                | Left to right |
//| !=               | Inequality                              | Left to right |
//| &                | Bitwise AND                             | Left to right |
//| ^                | Bitwise exclusive OR                    | Left to right |
//| |                | Bitwise OR                              | Left to right |
//| &&               | Logical AND                             | Left to right |
//| ||               | Logical OR                              | Left to right |
//| e1?e2:e3         | Conditional                             | Right to left |
//| =                | Assignment                              | Right to left |
//| *=               | Multiplication assignment               | Right to left |
//| /=               | Division assignment                     | Right to left |
//| %=               | Modulus assignment                      | Right to left |
//| +=               | Addition assignment                     | Right to left |
//| -=               | Subtraction assignment                  | Right to left |
//| <<=              | Left-shift assignment                   | Right to left |
//| >>=              | Right-shift assignment                  | Right to left |
//| &=               | Bitwise AND assignment                  | Right to left |
//| |=               | Bitwise inclusive OR assignment         | Right to left |
//| ^=               | Bitwise exclusive OR assignment         | Right to left |
//| ,                | Comma                                   | Left to right |
//+------------------+-----------------------------------------+---------------+

#include "urlencode.h"


/*
parameter:
 pcUtf_8:	pcUtf_8 string 
 ulNum:		strlen(pcUtf_8) or less
 pcUrl:		pcUrl string
 
return value:
 strlen(pcUtf_8)
*/
uint32_t ulURL_Encode(const char *pcUtf_8, uint32_t ulNum, char *pcUrl)
{
	uint32_t i = 0;
	uint32_t j = 0;
	const char cHexCode[16] = {"0123456789ABCDEF"};
	
	for (i=0,j=0; i<ulNum; i++,j+=3)
	{
		pcUrl[j+0] = '%';
		pcUrl[j+1] = cHexCode[(uint8_t)pcUtf_8[i] >> 0x04];
		pcUrl[j+2] = cHexCode[(uint8_t)pcUtf_8[i]  & 0x0F];
	}
	return j+3;
}


/*
parameter:
 pcUincode:	pcUincode string 
 ulNum:		strlen(pcUincosde) or less
 pcUtf_8:	pcUtf_8 string
 
return value:
 strlen(pcUtf_8)
*/
uint32_t ulUnicodetoUtf_8(const char *pcUincode, uint32_t ulNum, char *pcUtf_8)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint16_t cTemp;
	
	for (i=0,j=0; i<ulNum; i+=2,j+=3)
	{
		cTemp = ((uint8_t)pcUincode[i] << 8) + (uint8_t)pcUincode[i+1];
		pcUtf_8[j+0] = (char)((cTemp >> 0x0C & 0x3F) | 0xE0);
		pcUtf_8[j+1] = (char)((cTemp >> 0x06 & 0x3F) | 0x80);
		pcUtf_8[j+2] = (char)((cTemp >> 0x00 & 0x3F) | 0x80);
	}
	return j+3;
}


/*
parameter:
 pcUtf_8:	pcUtf_8 string 
 ulNum:		strlen(pcUtf_8) or less
 pcUincode:	pcUincode string
 
return value:
 strlen(pcUincode)
*/
uint32_t ulUtf_8toUnicode(const char *pcUtf_8, uint32_t ulNum, char *pcUincode)
{
	uint32_t i = 0;
	uint32_t j = 0;
	
	for (i=0,j=0; j<ulNum; i+=3,j+=2)
	{
		pcUincode[j+0] = (char)(( ((uint8_t)pcUtf_8[i+0] & 0x0F) << 0x04) + \
								( ((uint8_t)pcUtf_8[i+1] & 0x3C) >> 0x02));
		pcUincode[j+1] = (char)(( ((uint8_t)pcUtf_8[i+1] & 0x03) << 0x06) + \
								( ((uint8_t)pcUtf_8[i+2] & 0x3F) >> 0x00));
	}
	return j+2;
}

//UTF和GBK的转换可以用ffunicode.c
#include "ff.h"

/*
parameter:
 pcUincode:	pcUincode string 
 ulNum:		strlen(pcUincode) or less
 pcUincode:	pcUincode string
 
return value:
 strlen(pcUincode)
*/
uint32_t UnicodetoGbk(const char *pcUincode, uint32_t ulNum, char *pcGbk)
{
	uint16_t i = 0;
	uint16_t cTemp = 0;
	
	for (i=0; i<ulNum; i+=2)
	{
		cTemp = ((uint8_t)pcUincode[i] << 0x08) + (uint8_t)pcUincode[i+1];
		cTemp = ff_uni2oem(cTemp, FF_CODE_PAGE);
		pcGbk[i+0] = (char)(cTemp >> 0x08);
		pcGbk[i+1] = (char)(cTemp  & 0xFF);
	}
	return i+2;
}


/*
parameter:
 pcGbk:		pcGbk string 
 ulNum:		strlen(pcGbk) or less
 pcUincode:	pcUincode string
 
return value:
 strlen(pcUincode)
*/
uint32_t GbktoUnicode(const char *pcGbk, uint32_t ulNum, char *pcUincode)
{
	uint32_t i = 0;
	uint16_t cTemp = 0;
	
	for (i=0; i<ulNum; i+=2)
	{
		cTemp = ((uint8_t)pcGbk[i] << 0x08) + (uint8_t)pcGbk[i+1];
		cTemp = ff_oem2uni(cTemp, FF_CODE_PAGE);
		pcUincode[i+0] = (char)(cTemp >> 0x08);
		pcUincode[i+1] = (char)(cTemp  & 0xFF);
	}
	return i+2;
}
