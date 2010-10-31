/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

/*
A modified version of uvudec main.cpp with the symbols rearranged
Allows simulation of running the program without going through system() mess
*/

//We don't want to launch from main
//we will hook into uvmain, which is called from main
#define main		uvudec_main_disabled
#define uvmain		uvudec_uvmain

#include "../uvudec/main.cpp"
