/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
Try a mix between short (unnamed) names and some later proper names

WARNING WARNING WARNING
This file seems to be parsed incorrectly by FLAIR?
5589E583EC10C745FC03000000EB138B45FC69C03D1C22008945FC8145FCF300 DA F9CA 0129 :0076 _aa3 :00B1 _aaa4 :00EC _caller ........C7042401000000E8........C7042401000000E8........C7042401000000E8........B8A77E0000C9C3
	FLAIR
	0x20 + 0xDA + (210 - 116)/2 = 0x20 + 0xDA + 47 = 297 = 0x129
5589E583EC10C745FC03000000EB138B45FC69C03D1C22008945FC8145FCF300 C6 B718 0115 :006C _aa3 :00A2 _aaa4 :00D8 _caller ........C7042401000000E8........C7042401000000E8........C7042401000000E8........B8A77E0000C9C3
	uvudec
Module size is reported as 0x0129, there are only 0x0115 bytes according to objdump
FLAIR is saying there are another 0x14 bytes
Maybe its time to check the raw binary data
According to independent verification by my uvelf tool (does not depend on binutils), FLAIR is wrong on the module size
Next question: whats so special at that offset to cause that?
Elf32_Shdr (section header) at index 1 (1):
  sh_name (section string table index):      0x0000001F (31)
    .text
  sh_type (type):                            0x00000001 (1)
    SHT_PROGBITS (program data)
  sh_addr (virtual addr at execution):       0x00000000 (0)
  sh_offset (file offset):                   0x00000034 (52)
  sh_size (size in bytes):                   0x00000115 (277)
  sh_entsize (entry size if sec holds tbl):  0x00000000 (0)
  sh_addralign (alignment):                  0x00000004 (4)
  sh_flags (flags):                          0x00000006 (6)
    SHF_ALLOC (occupies memory during execution)
    SHF_EXECINSTR (executable)
  sh_type (type) specific data...
    unknown
    sh_link (link to another section):         0x00000000 (0)
    sh_info (additional section info):         0x00000000 (0)
  String: <.text>
    Description: executable instructions
    Hexdump
      55 89 E5 83 EC 10 C7 45  FC 03 00 00 00 EB 13 8B  |U......E........|
      45 FC 69 C0 3D 1C 22 00  89 45 FC 81 45 FC F3 00  |E.i.=."..E..E...|
      00 00 81 7D FC CA D6 93  02 7E E4 8B 45 08 8B 55  |...}.....~..E..U|
      FC 8D 04 02 C9 C3 55 89  E5 83 EC 10 C7 45 FC 03  |......U......E..|
      00 00 00 EB 13 8B 45 FC  69 C0 3D 1C 22 00 89 45  |......E.i.=."..E|
      FC 81 45 FC F3 00 00 00  81 7D FC CB D6 93 02 7E  |..E......}.....~|
      E4 8B 45 08 8B 55 FC 8D  04 02 C9 C3 55 89 E5 83  |..E..U......U...|
      EC 10 C7 45 FC 03 00 00  00 EB 13 8B 45 FC 69 C0  |...E........E.i.|
      3D 1C 22 00 89 45 FC 81  45 FC F3 00 00 00 81 7D  |=."..E..E......}|
      FC CC D6 93 02 7E E4 8B  45 08 8B 55 FC 8D 04 02  |.....~..E..U....|
      C9 C3 55 89 E5 83 EC 10  C7 45 FC 03 00 00 00 EB  |..U......E......|
      13 8B 45 FC 69 C0 3D 1C  22 00 89 45 FC 81 45 FC  |..E.i.=."..E..E.|
      F3 00 00 00 81 7D FC CD  D6 93 02 7E E4 8B 45 08  |.....}.....~..E.|
      8B 55 FC 8D 04 02 C9 C3  55 89 E5 83 EC 04 C7 04  |.U......U.......|
      24 01 00 00 00 E8 FC FF  FF FF C7 04 24 01 00 00  |$...........$...|
      00 E8 FC FF FF FF C7 04  24 01 00 00 00 E8 FC FF  |........$.......|
      FF FF C7 04 24 01 00 00  00 E8 FC FF FF FF B8 A7  |....$...........|
      7E 00 00 C9 C3                                    |~....           |
*/

int a(int i)
{
	int j;

	for( j = 3; j < 43243211; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int a2(int i)
{
	int j;

	for( j = 3; j < 43243212; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int aa3(int i)
{
	int j;

	for( j = 3; j < 43243213; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int aaa4(int i)
{
	int j;

	for( j = 3; j < 43243214; j += 243)
	{
		j << 23;
		j *= 2235453;
	}
	return j + i;
}

int caller()
{
	a(1);
	a2(1);
	aa3(1);
	aaa4(1);
	return 32423;
}

