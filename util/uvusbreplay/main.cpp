/*
Wanted to do this in python but couldn't get python bindings to work even after trying newest version of libpcap
Expect the Lua bindings are out of date and I don't feel like messing with them
*/

#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "uvd/util/error.h"
#include "uvd/util/util.h"
#include <limits.h>
     
typedef enum {
	//Linux kernel
	OUTPUT_LINUX,
	//libusb
	OUTPUT_LIBUSB,
} output_target_t;
output_target_t g_output_target = OUTPUT_LINUX;

unsigned int g_min_packet = 0;
unsigned int g_cur_packet = 0;
unsigned int g_max_packet = UINT_MAX;


void loop_cb(u_char *args, const struct pcap_pkthdr *header,
    const u_char *packet) {
    	
	++g_cur_packet;
	if (g_cur_packet < g_min_packet || g_cur_packet > g_max_packet) {
		//printf("Skipping packet %d\n", g_cur_packet);
		return;
	}
	
	printf("caplen: %d, len: %d\n",
			header->caplen, header->len );
	if (header->caplen != header->len) {
		printf("malformed packet\n");
		return;
	}
	//caplen is actual length, len is reported
}

void usage() {
	printf("uvusbreplay [options] <input .cap>\n");
	printf("Options:\n");
	printf("-r <min-max>: use packet range, default all, 1 indexed (since Wireshark does), inclusive\n");
	printf("-k: format output for Linux kernel (default)\n");
	printf("-l: format output for libusb\n");
}

int main(int argc, char *argv[])
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *p = NULL;
	
	opterr = 0;

	while (true) {
		int c = getopt(argc, argv, "r:klh?");
		
		if (c == -1) {
			break;
		}
		
		switch (c)
		{
			case 'r':
			{
				if (UV_FAILED(parseNumericRangeString(optarg, &g_min_packet, &g_max_packet))) {
					printf("Invalid range string %s\n", optarg);
					usage();
					exit(1);
				}
				break;
			}
			
			case 'k':
				g_output_target = OUTPUT_LINUX;
				break;
			
			case 'l':
				g_output_target = OUTPUT_LIBUSB;
				break;
			
			case 'h':
			case '?':
				usage();
				exit(1);
				break;
			
			default:
				printf("Unknown argument %c\n", c);
				usage();
				exit(1);
		}
	}
	
	std::string fileName = "in.cap";
    //"/home/mcmaster/document/external/uvscopetek/captures/twain_image/wireshark/1/640x320_wireshark.cap"
     
	unsigned int raws = 0;
	for (int index = optind; index < argc; index++) {
		if (raws == 0) {
			fileName = argv[index];
		} else {
			printf("Too many args\n");
			usage();
			exit(1);
		}
		++raws;
	}
	
	p = pcap_open_offline(fileName.c_str(), errbuf);
	
	pcap_loop(p, -1, loop_cb, NULL);
	
	return 0;
}

