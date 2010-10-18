/*
Inspired by Red Plait`s pattern maker
	See util/rpat for original program (w/ my fixes)
Copyrightish 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifdef UVD_FLIRT_PATTERN_BFD

#include <bfd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uvd/init/config.h"
#include "uvd/hash/crc.h"
#include "uvd/util/debug.h"
#include "uvd/flirt/flirt.h"
#include "uvdbfd/flirt/flirt.h"
#include "uvdbfd/flirt/core.h"
#include "uvd/util/string_writer.h"
#include "uvd/flirt/flirt.h"

/*
UVDFLIRTPatternGeneratorBFD
*/

UVDFLIRTPatternGeneratorBFD::UVDFLIRTPatternGeneratorBFD()
{
}

UVDFLIRTPatternGeneratorBFD::~UVDFLIRTPatternGeneratorBFD()
{
	deinit();
}

uv_err_t UVDFLIRTPatternGeneratorBFD::init()
{
	std::string defaultTarget = "i686-pc-linux-gnu";
	printf_flirt_debug("bfd init\n");
	bfd_init();
	if( !bfd_set_default_target(defaultTarget.c_str()) )
	{
		printf_error("can't set BFD default target to `%s': %s", defaultTarget.c_str(), bfd_errmsg(bfd_get_error()));
		return UV_DEBUG(UV_ERR_GENERAL);
	}
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGeneratorBFD::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGeneratorBFD::canGenerate(const std::string &file)
{
	uv_err_t rc = UV_ERR_GENERAL;
	bfd *abfd = NULL;

	//If its a junk file, don't think we get a good handle
	abfd = bfd_openr(file.c_str(), "default");
	if( abfd == NULL )
	{
		printf_error("Could not open file <%s>\n", file.c_str());
		return UV_ERR_GENERAL;
	}

	//Needs to be an object...maybe an archive?	
	if( bfd_check_format(abfd, bfd_object) == TRUE
			|| bfd_check_format(abfd, bfd_archive) == TRUE )
	{
		rc = UV_ERR_OK;
	}
	else
	{
		rc = UV_ERR_GENERAL;
	}
	bfd_close(abfd);
printf_debug("rc %d\n", rc);

	return rc;
}

uv_err_t UVDFLIRTPatternGeneratorBFD::generateByBFD(bfd *abfd, std::string &output)
{
	UVDBFDPatCore generator;
	
	printf_flirt_debug("generating for bfd %s\n", bfd_get_filename(abfd));
	
	uv_assert_err_ret(generator.init(abfd));
	uv_assert_err_ret(generator.generate());
	output += generator.m_writer.m_buffer;
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGeneratorBFD::generateByFile(const std::string &fileName, std::string &output)
{
	/*
	TODO: move recursion to recursive by BFD
	*/
	bfd *abfd = NULL;

	//printf("head bfd generation for filename %s\n", fileName.c_str());
	//fflush(stdout);
	printf_flirt_debug("head bfd generation for filename %s\n", fileName.c_str());
	abfd = bfd_openr(fileName.c_str(), "default");
	uv_assert_ret(abfd);
	//If we get an archive, we must recurse
	if( bfd_check_format(abfd, bfd_archive) == TRUE )
	{
		bfd *lastArbfd = NULL;
		bfd *arbfd = NULL;

		//Recursive for each file in the archive
		for(;; )
		{
			//What might have set an error?
			bfd_set_error(bfd_error_no_error);

			//If arbfd is NULL, indicates begin() on the linked list
			arbfd = bfd_openr_next_archived_file(abfd, arbfd);
			//We advanced, trash old if we are still hanging onto it
			if( lastArbfd )
			{
				bfd_close(lastArbfd);
			}
			
			//and end()?
			if( arbfd == NULL )
			{
				uv_assert_ret(bfd_get_error() == bfd_error_no_more_archived_files);
				break;
			}
			uv_assert_err_ret(generateByBFD(arbfd, output));
			lastArbfd = arbfd;
		}
	}
	//Otherwise, process
	else
	{
		uv_assert_err_ret(generateByBFD(abfd, output));
	}
	bfd_close(abfd);
	
	return UV_ERR_OK;
}

uv_err_t UVDFLIRTPatternGeneratorBFD::saveToStringCore(const std::string &inputFile, std::string &output)
{
	uv_assert_err_ret(generateByFile(inputFile, output));
	return UV_ERR_OK;
}
	
uv_err_t UVDFLIRTPatternGeneratorBFD::getPatternGenerator(UVDFLIRTPatternGeneratorBFD **generatorOut)
{
	UVDFLIRTPatternGeneratorBFD *generator = NULL;
	
	generator = new UVDFLIRTPatternGeneratorBFD();
	
	uv_assert_err_ret(generator->init());
	
	*generatorOut = generator;
	return UV_ERR_OK;
}

#endif

