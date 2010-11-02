/* uvudec.i */
%module uvudec
%{
/* Put header files here or function declarations like below */
//extern double My_variable;
//extern int fact(int n);
//extern int my_mod(int x, int y);
//extern char *get_time();

//uv_err_t UVDInit();
extern int UVDInit();
//uv_err_t UVDDeinit();
extern int UVDDeinit();
%}

//extern double My_variable;
//extern int fact(int n);
//extern int my_mod(int x, int y);
//extern char *get_time();

extern int UVDInit();
extern int UVDDeinit();

