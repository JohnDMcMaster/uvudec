/*
Wanted to do this in python but couldn't get python bindings to work even after trying newest version of libpcap
Expect the Lua bindings are out of date and I don't feel like messing with them
Python is good for dev, but SIGSEGV in Python is ugly...
*/

#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "uvd/util/error.h"
#include "uvd/util/util.h"
#include <limits.h>

#include "linux/usb/ch9.h"
     
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
bool g_error = false;
bool g_halt_error = true;
bool g_verbose = false;

#define dbg(...) do { \
	if (g_verbose) { printf(__VA_ARGS__); } \
} while(0)

//typedef uint8_t usb_urb_id_t[8];

#define URB_SUBMIT        'S'
#define URB_COMPLETE      'C'
#define URB_ERROR         'E'
typedef uint8_t urb_type_t;

#define URB_ISOCHRONOUS   0x0
#define URB_INTERRUPT     0x1
#define URB_CONTROL       0x2
#define URB_BULK          0x3
typedef uint8_t urb_transfer_t;

typedef struct {
	uint64_t id;
	urb_type_t type;
	urb_transfer_t transfer_type;
	uint8_t endpoint;
	uint8_t device;
	uint16_t bus_id;
	uint8_t setup_request;
	uint8_t data;
	uint64_t sec;
	uint32_t usec;
	uint32_t status;
	uint32_t length;
	uint32_t data_length;
} usb_urb_t;

std::string transfer_type_str( unsigned int tt ) {
	switch (tt) {
	case URB_ISOCHRONOUS:
		return "URB_ISOCHRONOUS";
	case URB_INTERRUPT:
		return "URB_INTERRUPT";
	case URB_CONTROL:
		return "URB_CONTROL";
	case URB_BULK:
		return "URB_BULK";
	default:
		printf("WTF?\n");
		if (g_halt_error) {
			exit(1);
		}
		return "";
	}
}

//Output printf
#define oprintf		printf

std::string get_request_str(unsigned int bRequestType, unsigned int bRequest) {
	switch (bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		switch (bRequest) {
		case USB_REQ_GET_STATUS:
			return "USB_REQ_GET_STATUS";
		case USB_REQ_CLEAR_FEATURE:
			return "USB_REQ_CLEAR_FEATURE";
		case USB_REQ_SET_FEATURE:
			return "USB_REQ_SET_FEATURE";
		case USB_REQ_SET_ADDRESS:
			return "USB_REQ_SET_ADDRESS";
		case USB_REQ_GET_DESCRIPTOR:
			return "USB_REQ_GET_DESCRIPTOR";
		case USB_REQ_SET_DESCRIPTOR:
			return "USB_REQ_SET_DESCRIPTOR";
		case USB_REQ_GET_CONFIGURATION:
			return "USB_REQ_GET_CONFIGURATION";
		case USB_REQ_SET_CONFIGURATION:
			return "USB_REQ_SET_CONFIGURATION";
		case USB_REQ_GET_INTERFACE:
			return "USB_REQ_GET_INTERFACE";
		case USB_REQ_SET_INTERFACE:
			return "USB_REQ_SET_INTERFACE";
		case USB_REQ_SYNCH_FRAME:
			return "USB_REQ_SYNCH_FRAME";	
		default:
			printf("WTF?\n");
			if (g_halt_error) {
				exit(1);
			}
			return "";
		};
			
	//TODO: consider decoding class, although really its not as of much interest for replaying
	//since it should be well defined
	case USB_TYPE_CLASS:
	case USB_TYPE_VENDOR:
	case USB_TYPE_RESERVED:
		return UVDSprintf("0x%02X", bRequest);
	
	default:
		printf("WTF?\n");
		if (g_halt_error) {
			exit(1);
		}
		return "";
	}
}

std::string get_request_type_str(unsigned int bRequestType) {
	std::string ret = "";
	
	if ((bRequestType & USB_DIR_IN) == USB_DIR_IN) {
		ret += "USB_DIR_IN";
	} else {
		ret += "USB_DIR_OUT";
	}
	
	switch (bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		ret += " | USB_TYPE_STANDARD"; 
		break;
	case USB_TYPE_CLASS:
		ret += " | USB_TYPE_CLASS"; 
		break;
	case USB_TYPE_VENDOR:
		ret += " | USB_TYPE_VENDOR"; 
		break;
	case USB_TYPE_RESERVED:
		ret += " | USB_TYPE_RESERVED"; 
		break;
	default:
		printf("WTF?\n");
		if (g_halt_error) {
			exit(1);
		}
		return "";
	}
	
	switch (bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		ret += " | USB_RECIP_DEVICE";
		break;
	case USB_RECIP_INTERFACE:
		ret += " | USB_RECIP_INTERFACE";
		break;
	case USB_RECIP_ENDPOINT:
		ret += " | USB_RECIP_ENDPOINT";
		break;
	case USB_RECIP_OTHER:
		ret += " | USB_RECIP_OTHER";
		break;
	case USB_RECIP_PORT:
		ret += " | USB_RECIP_PORT";
		break;
	case USB_RECIP_RPIPE:
		ret += " | USB_RECIP_RPIPE";
		break;
	default:
		printf("WTF?\n");
		if (g_halt_error) {
			exit(1);
		}
		return "";
	}

	return ret;
}

std::string urb_type_str(unsigned int t) {
	switch (t) {
	case URB_SUBMIT:
		return "URB_SUBMIT";
	case URB_COMPLETE:
		return "URB_COMPLETE";
	case URB_ERROR:
		return "URB_ERROR";
	default:
		printf("WTF?\n");
		if (g_halt_error) {
			exit(1);
		}
		return "";
	}
}

//When we get an IN request we may process packets in between
class PendingRX {
public:
	usb_urb_t m_urb;
	usb_ctrlrequest m_ctrl;
	unsigned int packet_number;
};

//Pending control requests
std::map<uint64_t, PendingRX> g_pending;

void loop_cb(u_char *args, const struct pcap_pkthdr *header,
    const u_char *packet) {
    unsigned int len = 0;
	usb_urb_t *urb = NULL;
	//Skip over the header data
	struct usb_ctrlrequest *ctrl = NULL;
    	
	++g_cur_packet;
	if (g_cur_packet < g_min_packet || g_cur_packet > g_max_packet) {
		//printf("Skipping packet %d\n", g_cur_packet);
		return;
	}
	
	if (header->caplen != header->len) {
		printf("packet %d: malformed, caplen %d != len %d\n",
			g_cur_packet, header->caplen, header->len );
		g_error = true;
		return;
	}
	len = header->len;
	//caplen is actual length, len is reported
	
	urb = (usb_urb_t *)packet;
	if (g_verbose) {
		printf("Packet %d\n", g_cur_packet);
		printf("\tid: 0x%016llX\n", urb->id);
		printf("\ttype: %s (%c / 0x%02X)\n", urb_type_str(urb->type).c_str(), urb->type, urb->type);
		printf("\ttransfer_type: %s (0x%02X)\n",
				transfer_type_str(urb->transfer_type).c_str(), urb->transfer_type );
		printf("\tendpoint: 0x%02X\n", urb->endpoint);
		printf("\tdevice: 0x%02X\n", urb->device);
		printf("\tbus_id: 0x%04X\n", urb->bus_id);
		printf("\tsetup_request: 0x%02X\n", urb->setup_request);
		printf("\tdata: 0x%02X\n", urb->data);
		//printf("\tsec: 0x%016llX\n", urb->sec);
		printf("\tusec: 0x%08X\n", urb->usec);
		printf("\tstatus: 0x%08X\n", urb->status);
		printf("\tlength: 0x%08X\n", urb->length);
		printf("\tdata_length: 0x%08X\n", urb->data_length);
	}
	
	if (len < sizeof(*ctrl)) {
		printf("packet %d: got %d instead of min header length %d\n",
				g_cur_packet, len, sizeof(*ctrl));
		g_error = true;
		return;
	}
	if (urb->transfer_type & URB_CONTROL) {
		ctrl = (struct usb_ctrlrequest *)&urb[1];
		if (g_verbose) {
			printf("Packet %d (control info)\n", g_cur_packet);
			printf("\tbRequestType: %s (0x%02X)\n", get_request_type_str(ctrl->bRequestType).c_str(), ctrl->bRequestType);
			printf("\tbRequest: %s (0x%02X)\n", get_request_str( ctrl->bRequestType, ctrl->bRequest ).c_str(), ctrl->bRequest);
			printf("\twValue: 0x%04X\n", ctrl->wValue);
			printf("\twIndex: 0x%04X\n", ctrl->wIndex);
			printf("\twLength: 0x%04X\n", ctrl->wLength);
		}
	
		if ((ctrl->bRequestType & USB_DIR_IN) == USB_DIR_IN) {
			dbg("%d: IN\n", g_cur_packet);
		} else {
			dbg("%d: OUT\n", g_cur_packet);
		}
		
		if (urb->type == URB_ERROR) {
			printf("oh noes!\n");
			if (g_halt_error) {
				exit(1);
			}
		} else if (urb->type == URB_SUBMIT) {
			PendingRX pending;
	
			pending.m_urb = *urb;
			pending.m_ctrl = *ctrl;
			pending.packet_number = g_cur_packet;
			g_pending[urb->id] = pending;
			
			//Wait for the return
			return;
		} else if(urb->type != URB_COMPLETE) {
			printf("WTF?\n");
			if (g_halt_error) {
				exit(1);
			}
		}
		
		//Find the matching submit request
		PendingRX submit;
		if (g_pending.find(urb->id) == g_pending.end()) {
			printf("WTF?\n");
			if (g_halt_error) {
				exit(1);
			}
		}
		submit = g_pending[urb->id];
		//Done with it, get rid of it
		g_pending.erase(g_pending.find(urb->id));
		if (!g_pending.empty()) {
			printf("WARNING: out of order traffic, not too much thought put into that\n");
		}
		
		std::string device_str = "dev->udev";
		std::string pipe_str;
		unsigned int timeout = 500;
		if (ctrl->bRequestType & USB_DIR_IN) {
			pipe_str = UVDSprintf( "usb_rcvctrlpipe(%s, 0)", device_str.c_str() );
		} else {
			pipe_str = UVDSprintf( "usb_sndctrlpipe(%s, 0)", device_str.c_str() );
		}
	
		oprintf("//Generated from packet %d / %d\n", submit.packet_number, g_cur_packet);
		
		unsigned int data_size = 0;
		std::string data_str = "NULL";
		if (ctrl->bRequestType & USB_DIR_OUT) {
			//Verify data
			if (ctrl->wLength) {
				printf("FIXME: add out data support\n");
				if (g_halt_error) {
					exit(1);
				}
			}		
		}
		oprintf("usb_control_msg(%s, %s, %s, %s, 0x%04X, 0x%04X, %s, %u, %u);\n",
				device_str.c_str(), pipe_str.c_str(),
				//get_request_str(ctrl->bRequest).c_str(),
				get_request_str( ctrl->bRequestType, ctrl->bRequest ).c_str(),
				get_request_type_str(ctrl->bRequestType).c_str(),
				ctrl->wValue, ctrl->wIndex,
				data_str.c_str(), data_size,
				timeout );
				
		if (ctrl->bRequestType & USB_DIR_IN) {
			//Verify data
			if (ctrl->wLength) {
				printf("FIXME: add in data support\n");
				if (g_halt_error) {
					exit(1);
				}
			}		
		}
	}
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
		int c = getopt(argc, argv, "r:klh?v");
		
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
			
			case 'v':
				g_verbose = true;
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
	
	dbg("parsing from range %u to %u\n", g_min_packet, g_max_packet);
	p = pcap_open_offline(fileName.c_str(), errbuf);
	pcap_loop(p, -1, loop_cb, NULL);
	
	if (!g_pending.empty()) {
		printf("WARNING: %d pending requests\n", g_pending.size());
	}
	
	return 0;
}

