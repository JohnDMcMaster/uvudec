/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

Some code originally from
http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format
*/

std::string hexstr(const char *in, int sz)
{
	std::string ret;
	for (int i = 0; i < sz; ++i) {
		char buff[3];
		sprintf(buff, "%.2X", in[i]);
		ret += buff;
	}
	return ret;
}

std::string safestr(const char *in, int sz)
{
	std::string ret;
	
	for (int i = 0; i < sz; ++i) {
		if (in[i] == 0)
			break;
		if (isprint(in[i]))
			ret += in[i];
		else
			ret += '.';
	}
	return ret;
}

void init(const char *in)
{
	FILE *file = NULL;
	struct stat astat;

	file = fopen(in, "rb");
	if (!file)
		err("file not found\n");

	if (stat(in, &astat))
		err("no size\n");
	
	g_file_size = astat.st_size;
	g_file_contents = (char *)malloc(g_file_size);
	
	if (!g_file_contents)
		err("alloc fail\n");

	if (fread(g_file_contents, 1, g_file_size, file) != g_file_size)
		err("bad file read\n");
	g_cur_ptr = g_file_contents;

	fclose(file);
}

void decompress()
{
	err("ZIP decompression not supported\n");
	exit(1);
}

void advance(int bytes)
{
	g_file_pos += bytes;
	g_cur_ptr += bytes;
}

int read_byte()
{
	uint8_t ret = *g_cur_ptr;
	advance(1);
	return ret;
}

int read16()
{
	return (read_byte() << 8)
		+ read_byte();
}

int bitshift_read()
{
	uint32_t first = read_byte();
	
	if ( first & 0x80)
		return ((first & 0x7F) << 8) + read_byte();
	return first;
}

int read_relocation_bitmask()
{
	uint32_t first;
	uint32_t lower;
	uint32_t upper;

	first = read_byte();
	
	if ((first & 0x80) != 0x80)
		return first;
	
	if ((first & 0xC0) != 0xC0)
		return ((first & 0x7F) << 8) + read_byte();
	
	if ((first & 0xE0) != 0xE0) {
		upper = ((first & 0xFF3F) << 8) + read_byte();
		lower = read16();
	} else {
		upper = read16();
		lower = read16();
	}
	uint32_t ret = lower + (upper << 16);
	return ret;
}

void dump_tree()
{
	uint16_t n_internal_nodes = bitshift_read();

	//Internal node
	if (n_internal_nodes) {
		uint32_t relocation_bitmask;

		for (int i = 0; i < n_internal_nodes; ++i) {
			uint32_t n_node_bytes;
			uint32_t cur_relocation_bitmask;
	
			n_node_bytes = read_byte();
			//Only allowed 32 bytes
			if (n_node_bytes > 0x20u)
				err("Too many bytes\n");

			cur_relocation_bitmask = 1 << (n_node_bytes - 1);

			if (n_node_bytes >= 0x10)
				relocation_bitmask = read_relocation_bitmask();
			else
				relocation_bitmask = bitshift_read();

			//Relocations don't appear until the end
			printf_indented("");
			for (uint32_t j = 0; j < n_node_bytes; ++j) {
				if ( cur_relocation_bitmask & relocation_bitmask)
					printf("..");
				else
					printf("%.2X", read_byte());
				cur_relocation_bitmask >>= 1;
			}
			printf(":\n");
			inc_indent();
			dump_tree();
			dec_indent();
		}
	//Leaf node
	} else {
		uint32_t read_flags;
		uint32_t func_index = 0;
		do {
			uint32_t tree_block_len = read_byte();
			uint32_t a_crc16 = read16();
			do {
				uint32_t total_len;
				uint32_t ref_cur_offset = 0;
								
				total_len = bitshift_read();
				printf_indented("%d. tree_block_len:0x%.2X a_crc16:0x%.4X total_len:0x%.4X", func_index, tree_block_len, a_crc16, total_len);
				++func_index;
			
				do {
					std::string name;
					uint32_t delta = 0;
					bool has_negative;
					
					delta = bitshift_read();
				
					read_flags = read_byte();
					//whys neg ref useful?
					has_negative = read_flags < 0x20;
					
					for (int i = 0; ; ++i) {
						if (i >= 1024)
							err("reference length exceeded\n");
					
						if ( read_flags < 0x20)
							read_flags = read_byte();
						if ( read_flags < 0x20)
							break;
				
						name += (char)read_flags;
						read_flags = 0;
					}
					ref_cur_offset += delta;
					if (ref_cur_offset == 0)
						printf(" ");
					printf(" %.4X:%s", ref_cur_offset, name.c_str());					
				} while (read_flags & 1);
				
				if (read_flags & 2) {
					uint32_t first;
					uint32_t second;
				
					first = bitshift_read();
					second = read_byte();
					printf(" (0x%.4X: 0x%.2X)", first, second);
				}
				
				//Symbol linked references
				if (read_flags & 4) {
					uint32_t a_offset;
					std::string ref_name;
					uint32_t ref_name_len;
							
					a_offset = bitshift_read();
					ref_name_len = read_byte();
					if (!ref_name_len)
						ref_name_len = bitshift_read();
					
					ref_name = std::string(g_cur_ptr, ref_name_len);
					//If last char is 0, we have a special flag set
					if (g_cur_ptr[ref_name_len - 1] == 0)
						a_offset = -a_offset;
					advance(ref_name_len);
				}
				printf("\n");
			} while (read_flags & 0x08);
		} while (read_flags & 0x10);
	}
}

int main(int argc, char **argv)
{
	struct sig_header_t *header = NULL;

	if (argc < 2)
		err("usage: sigread [signature file]\n");
	init(argv[1]);
	
	printf("File size: 0x%.8X (%d)\n", g_file_size, g_file_size);
	
	header = (struct sig_header_t *)g_file_contents;
	advance(sizeof(struct sig_header_t));
	
	printf("magic: %s\n", hexstr(header->magic, sizeof(header->magic)).c_str());
	if (memcmp(header, "IDASGN", 6))
		err("magic fail\n");
	
	printf("version: %d\n", header->version);
	/*
	if (header->version == 5)
		numberModules = oldNumberModules;
	Maybe some differences when using ver 6
	Ver 5 issues apply as well
	*/
	if (header->version != 7)
		err("version mismatch\n");
		
	printf("last (n_module) offest: 0x%.8X\n", offsetof(struct sig_header_t, n_modules));
	if (sizeof(struct sig_header_t) != 0x29)
		err("sig_header_t wrong size: 0x%.8X\n", sizeof(struct sig_header_t));
		
	printf("processor: %s (0x%.2X)\n", IDASigArchToString(header->processor).c_str(), header->processor);
	printf("file_types: %s (0x%.8X)\n", IDASigFileToString(header->file_types).c_str(), header->file_types);
	printf("OS_types: %s (0x%.4X)\n", IDASigOSToString(header->OS_types).c_str(), header->OS_types);
	printf("app_types: %s (0x%.4X)\n", IDASigApplicationToString(header->app_types).c_str(), header->app_types);
	printf("feature_flags: %s (0x%.2X)\n", IDASigFeaturesToString(header->feature_flags).c_str(), header->feature_flags);
	printf("unknown (pad): 0x%.2X\n", header->pad);
	printf("old_number_modules: 0x%.4X\n", header->old_number_modules);
	printf("crc16: 0x%.4X\n", header->crc16);	
	//Make sure its null terminated
	printf("ctype: %s\n", safestr(header->ctype, sizeof(header->ctype)).c_str());	
	printf("library_name_sz: 0x%.2X\n", header->library_name_sz);	
	printf("alt_ctype_crc: 0x%.4X\n", header->alt_ctype_crc);	
	printf("n_modules: 0x%.8X (%d)\n", header->n_modules, header->n_modules);

	//Name is immediatly after header
	char library_name[256];
	memcpy(&library_name[0], g_cur_ptr, header->library_name_sz);
	library_name[header->library_name_sz] = 0;
	advance(header->library_name_sz);
	printf("library name: %s\n", library_name);
		
	if (header->feature_flags & IDASIG__FEATURE__COMPRESSED)
		decompress();
	
	dump_tree();
		
	return 0;
}

