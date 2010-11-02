/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UV_DISASM_TYPES_H
#define UV_DISASM_TYPES_H

/*
For short term, C interface will be broken
Eventually a C interface will be exposed again with at least basic capabilities
*/

#include <string>
#include <vector>
#include <map>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef TRUE
#define TRUE	1
#endif /* ifndef TRUE */
#ifndef FALSE
#define FALSE	0
#endif /* ifndef FALSE */

#define MAX_OPCODE_SIZE			4
/* Maximum length, in bytes, of a instruction (opcode, prefix, operands) */
#define MAX_INST_SIZE			16

typedef int32_t uv_err_t;

/*
Tristate type
*/
typedef int32_t uvd_tri_t;
#define UVD_TRI_UNKNOWN			-1
#define UVD_TRI_FALSE			0
#define UVD_TRI_TRUE			1

//A function with appropriete return type that takes no args
typedef uv_err_t (*uv_thunk_t)();

//An analyzed data address
//FIXME: do massive replaces to get this into code
typedef uint32_t uv_addr_t;
#define UVD_ADDR_MAX			UINT_MAX

/*
Instruction classes
The way these are used is still being developed
Primary concern to is discover funcs and primary flow control.

First
func existance, basic flow control
-Calls and their target address
-Return
-Jumps and their target address

Next
func arguments
-Push
-Pop

Next
Basic arithmitic
-Add
-subtract
-multiply
-divide
-bitwise shifts

Next
Global variables, const data
-Use func arguments
-Use artimetic on global addresses
-Note funcs called on global variables
	Ex: strlen identifies a string and an integer returned
*/

/* Invalid, this indicates error */
#define UV_DISASM_INST_INVAL	0x0000

/* Arithmetic */
/* Addition */
#define UV_DISASM_INST_ADD		0x0001
/* Subtraction */
#define UV_DISASM_INST_SUB		0x0002
/* Multiplication */
#define UV_DISASM_INST_MUL		0x0003
/* Division */
#define UV_DISASM_INST_DIV		0x0004
/* Bitwise shift left */
#define UV_DISASM_INST_BITL		0x0005
/* Bitwise shift right */
#define UV_DISASM_INST_BITR		0x0006
/* Increment value */
#define UV_DISASM_INST_INC		0x0007
/* Decrement value */
#define UV_DISASM_INST_DEC		0x0008
/* Modular arithmitec */
#define UV_DISASM_INST_MOD		0x0008
/* Bitwise AND */
#define UV_DISASM_INST_BAND		0x0009
/* Bitwise OR */
#define UV_DISASM_INST_BOR		0x000A
/* Bitwise NOT */
#define UV_DISASM_INST_BNOT		0x000B
/* XOR */
#define UV_DISASM_INST_XOR		0x000C
/* Logical AND */
#define UV_DISASM_INST_LAND		0x0020
/* Logical OR */
#define UV_DISASM_INST_LOR		0x0021
/* Logical NOT */
#define UV_DISASM_INST_LNOT		0x0022


/* Flow control */
/* Uncondition jump */
#define UV_DISASM_INST_JMP		0x0100
/* Jump conditionally */
#define UV_DISASM_INST_JC		0x0101
/* Function call */
#define UV_DISASM_INST_CALL		0x0102
/* Return from call */
#define UV_DISASM_INST_RET		0x0103

/* Data flow */
/* Move a value */
#define UV_DISASM_INST_MOV		0x0200

/* Misc */
/* Multibyte opcode, indicates need to remap onto second byte */
#define UV_DISASM_INST_MB		0xF000
/* Instruction prefix */
#define UV_DISASM_INST_PREFIX	0xF001
/* System halt */
#define UV_DISASM_INST_HLT		0xF010
/* Call software interrupt  */
#define UV_DISASM_INST_INT		0xF011

/*
Assebler style
Ultimatly, this project is suppose to feed into GCC, so AT&T will be supported
over Intel syntax.  However, reference manuals are Intel format...so yeah

Styles wanted:

Intel:
MOV EBP, ESP

Intel with opcode deconposition (numbers may not be accurate)
MOV(0x89) EBP(r=5), ESP(mod=3, r/m=4)

AT&T:
mov %ebp, %esp
With decomposition:
mov(0x89) %ebp(r=5), %esp(mod=3, r/m=4)
mov(0x12) %ebp(r=5), imm32=0x23456789

Proposal parameters:

instruction
dest location
dest location decomposition
source location
source location decomposition
extra: appended to end

*/

/*
Instructions always seem to follow something like this
<prefix> <base> <operand>, <operand>, <operand>
Order of operands is reversed for AT&T vs Intel

In theory, one could ignore the concept of prefixes and just write a bajillion instruction defs as mulitbyte instructions
*/


//const char *uv_disasm_data_str(int uv_disasm_data);

#if 0
struct uv_disasm_reg_shared_t
{
	char *name;
	char *desc;
	/* In bits */
	int size;
	/* If this is a memory mapped register, describes where and what type */
	unsigned int mem_addr;
	struct uv_disasm_mem_shared_t *mem; 
};
#endif

/*
/ *
Information needed to make the parsed data to the usage data
* /
struct uv_inst_parse_t
{
	/ *
	After the opcode, every field seems to always match to an operand
	Most fields have a matching operand such as an immediate 
	* /
	struct uv_inst_operand_shared_t *operand;
	int reserved;
};
*/


#if 0
/* Store symbols in a string keyed binary search tree */
struct uv_disasm_sym_map_t
{
	char *key;	
	struct uv_disasm_sym_t *value;
	struct uv_disasm_sym_map_t *left;
	struct uv_disasm_sym_map_t *right;
};
#endif

typedef int uvd_varient_t;
#define UVD_VARIENT_UNKNOWN			0
#define UVD_VARIENT_STRING			1
#define UVD_VARIENT_INT32			2
#define UVD_VARIENT_UINT32			3

class UVDVarient
{
public:
	UVDVarient();
	UVDVarient(std::string s);
	UVDVarient(int32_t i);
	UVDVarient(uint32_t i);

	uvd_varient_t getType() const;

	uv_err_t getString(std::string &s) const;
	void setString(const std::string &s);

	uv_err_t getI32(int32_t &i) const;
	void setI32(int32_t i);

	uv_err_t getUI32(uint32_t &i) const;
	void setUI32(uint32_t i);
	
	std::string toString() const;

private:
	uvd_varient_t m_type;
	std::string m_sVal;
	union
	{
		int32_t m_I32Val;
		uint32_t m_UI32Val;
	};
};

typedef std::map<std::string, UVDVarient> UVDVariableMap;

/*
UVDUint32RangePair
Originally for UVDUint32RangePriorityList and other memory range pairings
*/
class UVDUint32RangePair
{
public:
	UVDUint32RangePair();
	UVDUint32RangePair(uint32_t min, uint32_t max);

	//Returns 0 if max < min
	uint32_t size() const;
	bool contains(uint32_t val);

public:
	uint32_t m_min;
	uint32_t m_max;
};
//For general use
typedef UVDUint32RangePair UVDRangePair;

#endif /* ifndef UV_DISASM_TYPES_H */

#include "uvd/util/error.h"
