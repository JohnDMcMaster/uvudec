#include "uvd_decompiler.h"
#include "uvd_c_decompiler.h"
#include "uvd_compiler.h"

UVDDecompileNotes::UVDDecompileNotes()
{
	language = UVD_LANGUAGE_UNKNOWN;
}

uv_err_t UVDDecompileNotes::getOptimalLanguage(int &langauge)
{
	language = m_optimalLangauge;
}

UVDDecompiler::UVDDecompiler()
{
}

UVDDecompiler::~UVDDecompiler()
{
	deinit();
}

//Class specific init function
uv_err_t UVDDecompiler::init()
{
	return UV_ERR_OK;
}

uv_err_t UVDDecompiler::deinit()
{
	return UV_ERR_OK;
}

uv_err_t getDecompiler(UVDCompiler *compiler, UVDDecompiler **decompilerIn)
{
	UVDDecompiler *decompiler = NULL;
	
	uv_assert_ret(compiler);
	uv_assert_ret(decompilerIn);
	
	//Only one choice right now
	decompiler = new UVDCDecompiler();
	uv_assert_ret(decompiler);
	*decompilerIn = decompiler;
	
	return UV_ERR_OK;
}
