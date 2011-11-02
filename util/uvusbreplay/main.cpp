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

#define URB_IS_IN( _urb) (((_urb)->endpoint & USB_DIR_IN) == USB_DIR_IN)

#define VERSION_STR		"0.1"

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
//#define URB_TYPE_MASK     0x3
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
} __attribute__((packed)) usb_urb_t;

//TODO; figure out what this actually is
typedef struct {
	uint8_t raw[24];
} __attribute__((packed)) control_rx_t;

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
		printf("WTF? %d\n", __LINE__);
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
			printf("WTF? %d\n", __LINE__);
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
		printf("WTF? %d\n", __LINE__);
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
		printf("WTF? %d\n", __LINE__);
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
		printf("WTF? %d\n", __LINE__);
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
		printf("WTF? %d\n", __LINE__);
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

bool keep_packet( const PendingRX &in ) {
	//grr forgot I had this on
	//return (in.m_urb.transfer_type & URB_CONTROL) && (in.m_ctrl.bRequestType & USB_DIR_IN);
	return true;
}

typedef struct {
	unsigned req_in;
	unsigned req_in_last;
	unsigned in;
	unsigned in_last;
	
	unsigned req_out;
	unsigned req_out_last;
	unsigned out;
	unsigned out_last;
} payload_bytes_type_t;
typedef struct {
	payload_bytes_type_t ctrl;
	payload_bytes_type_t bulk;
} payload_bytes_t;
payload_bytes_t g_payload_bytes;

void update_delta( payload_bytes_type_t *in ) {
	in->req_in_last = in->req_in;
	in->in_last = in->in;

	in->req_out_last = in->req_out;
	in->out_last = in->out;
}

void loop_cb(u_char *args, const struct pcap_pkthdr *header,
    const u_char *packet) {
    uint8_t *dat_cur = 0;
    unsigned int len = 0;
	usb_urb_t *urb = NULL;
	size_t remaining_bytes = 0;
    	
	++g_cur_packet;
	if (g_cur_packet < g_min_packet || g_cur_packet > g_max_packet) {
		printf("//Skipping packet %d\n", g_cur_packet);
		return;
	}
	
	if (header->caplen != header->len) {
		printf("packet %d: malformed, caplen %d != len %d\n",
			g_cur_packet, header->caplen, header->len );
		g_error = true;
		return;
	}
	len = header->len;
	remaining_bytes = len;
	dat_cur = (uint8_t *)packet;
	dbg("PACKET %u: length %u\n", g_cur_packet, len);
	//caplen is actual length, len is reported
	
	urb = (usb_urb_t *)dat_cur;
	remaining_bytes -= sizeof(*urb);
	dat_cur += sizeof(*urb);
	if (g_verbose) {
		printf("Packet %d (header size: %u)\n", g_cur_packet, sizeof(*urb));
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
	
	
	if (urb->type == URB_ERROR) {
		printf("oh noes!\n");
		if (g_halt_error) {
			exit(1);
		}
	}
	PendingRX pending;
	pending.m_urb = *urb;

	//Find the matching submit request
	PendingRX submit;
	if( urb->type == URB_COMPLETE) {
		if (g_pending.find(urb->id) == g_pending.end()) {
			printf("WTF? %d\n", __LINE__);
			if (g_halt_error) {
				exit(1);
			}
		}
		submit = g_pending[urb->id];
		//Done with it, get rid of it
		g_pending.erase(g_pending.find(urb->id));
		if (!g_pending.empty()) {
			//printf("WARNING: out of order traffic packet around %u, not too much thought put into that\n", g_cur_packet);
		}
	}
	
	if (urb->transfer_type == URB_CONTROL) {
		if (urb->type == URB_SUBMIT) {
			PendingRX pending;
			struct usb_ctrlrequest *ctrl = NULL;

			if (len < sizeof(*ctrl)) {
				printf("packet %d: got %d instead of min header length %d\n",
						g_cur_packet, len, sizeof(*ctrl));
				g_error = true;
				return;
			}
		
			ctrl = (struct usb_ctrlrequest *)&urb[1];
			remaining_bytes -=  sizeof(*ctrl);
			dat_cur += sizeof(*ctrl);
		
			if ((ctrl->bRequestType & USB_DIR_IN) == USB_DIR_IN) {
				dbg("%d: IN\n", g_cur_packet);
			} else {
				dbg("%d: OUT\n", g_cur_packet);
			}
				
			pending.m_ctrl = *ctrl;
			pending.packet_number = g_cur_packet;
			g_pending[urb->id] = pending;
			
			if (g_verbose) {
				printf("Packet %d control submit (control info size %u)\n", g_cur_packet, sizeof(*ctrl));
				printf("\tbRequestType: %s (0x%02X)\n", get_request_type_str(ctrl->bRequestType).c_str(), ctrl->bRequestType);
				printf("\tbRequest: %s (0x%02X)\n", get_request_str( ctrl->bRequestType, ctrl->bRequest ).c_str(), ctrl->bRequest);
				printf("\twValue: 0x%04X\n", ctrl->wValue);
				printf("\twIndex: 0x%04X\n", ctrl->wIndex);
				printf("\twLength: 0x%04X\n", ctrl->wLength);
			}
	
			//Wait for the return
			return;
		} else if(urb->type != URB_COMPLETE) {
			printf("WTF? %d\n", __LINE__);
			if (g_halt_error) {
				exit(1);
			}
		}
		
		if (false && keep_packet(submit)) {
			payload_bytes_type_t *bulk = &g_payload_bytes.bulk;
			//payload_bytes_type_t *ctrl = &g_payload_bytes.ctrl;
			
			printf("Transer statistics\n");
			printf("\tBulk\n");
			printf("\t\tIn: %u (delta %u), req: %u (delta %u)\n",
					bulk->in, bulk->in - bulk->in_last,
					bulk->req_in, bulk->req_in - bulk->req_in_last
					);
			update_delta( bulk );
			printf("\t\tOut: %u, req: %u\n", g_payload_bytes.bulk.out, g_payload_bytes.bulk.req_out);
			printf("\tControl\n");
			printf("\t\tIn: %u, req: %u\n", g_payload_bytes.ctrl.in, g_payload_bytes.ctrl.req_in);
			printf("\t\tOut: %u, req: %u\n", g_payload_bytes.ctrl.out, g_payload_bytes.ctrl.req_out);
		}
		//std::string device_str = "dev->udev";
		std::string device_str = "udev";
		std::string pipe_str;
		std::string timeout = "500";
		if (submit.m_ctrl.bRequestType & USB_DIR_IN) {
			pipe_str = UVDSprintf( "usb_rcvctrlpipe(%s, 0)", device_str.c_str() );
		} else {
			pipe_str = UVDSprintf( "usb_sndctrlpipe(%s, 0)", device_str.c_str() );
		}
	
		if (keep_packet(submit)) {
			oprintf("//Generated from packet %u/%u\n", submit.packet_number, g_cur_packet);
		}
		
		unsigned int data_size = 0;
		std::string data_str = "NULL";
		if (submit.m_ctrl.bRequestType & USB_DIR_OUT) {
			//Verify data
			if (submit.m_ctrl.wLength) {
				printf("FIXME: add out data support\n");
				if (g_halt_error) {
					exit(1);
				}
			}		
		} else {
			if (submit.m_ctrl.wLength) {
				data_str = "buff";
				data_size = submit.m_ctrl.wLength;
			}
		}
		if (keep_packet(submit)) {
			oprintf("n_rw = usb_control_msg(%s, %s, %s, %s, 0x%04X, 0x%04X, %s, %u, %s);\n",
					device_str.c_str(), pipe_str.c_str(),
					//get_request_str(submit.m_ctrl.bRequest).c_str(),
					get_request_str( submit.m_ctrl.bRequestType, submit.m_ctrl.bRequest ).c_str(),
					get_request_type_str(submit.m_ctrl.bRequestType).c_str(),
					submit.m_ctrl.wValue, submit.m_ctrl.wIndex,
					data_str.c_str(), data_size,
					timeout.c_str() );
		}
		if (submit.m_ctrl.bRequestType & USB_DIR_IN) {
			unsigned int payload_sz = submit.m_ctrl.wLength;
			
			//Take off the unknown struct
			if (remaining_bytes < sizeof(control_rx_t)) {
				printf("not enough data\n");
				if (g_halt_error) {
					exit(1);
				}
				return;
			}
			dat_cur += sizeof(control_rx_t);
			remaining_bytes -= sizeof(control_rx_t);
			
			//Verify we actually have enough
			if (remaining_bytes != payload_sz) {
				printf("expected remaining bytes %u to be the requested length %u\n",
						remaining_bytes, payload_sz );
				if (g_halt_error) {
					exit(1);
				}
				return;
			}
			std::string byte_str;
			uint8_t *payload = NULL;
			payload = dat_cur;
			std::string pad = "";
			for (unsigned int i = 0; i < payload_sz; ++i) {
				byte_str += pad;
				byte_str += UVDSprintf("0x%02X", payload[i]);
				pad = ", ";
			}
			if (keep_packet(submit)) {
				oprintf("if (validate_read((char[]){%s}, %u, buff, n_rw, \"packet %u/%u\") < 0)\n",
						byte_str.c_str(), payload_sz, 
						submit.packet_number, g_cur_packet );
				oprintf("\treturn 1;\n");
			}
			remaining_bytes -= submit.m_ctrl.wLength;
		} else {
			if (keep_packet(submit)) {
				oprintf("if (validate_write(%u, n_rw, \"packet %u/%u\") < 0)\n",
						data_size, submit.packet_number, g_cur_packet );
				oprintf("\treturn 1;\n");
			}
		}
	} else if (urb->transfer_type == URB_BULK) {
		//Don't care about this but populate for now
		g_pending[urb->id] = pending;
	
		if (URB_IS_IN(urb)) {
			switch (urb->type) {
			case URB_SUBMIT:
				g_payload_bytes.bulk.req_in += urb->length;
				break;
			case URB_COMPLETE:
				g_payload_bytes.bulk.in += urb->data_length;
				break;
			}		
		} else {	
			switch (urb->type) {
			case URB_SUBMIT:
				g_payload_bytes.bulk.req_out += urb->length;
				break;
			case URB_COMPLETE:
				g_payload_bytes.bulk.out += urb->data_length;
				break;
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
	

	oprintf("/*\n");
	oprintf("Generated by uvusbreplay %s\n", VERSION_STR);
	oprintf("uvusbreplay copyright 2011 John McMaster <JohnDMcMaster@gmail.com>\n");
	oprintf("Date: %s\n", UVDCurDateTime().c_str());
	oprintf("Source data: %s\n", fileName.c_str());
	oprintf("Source range: %u - %u\n", g_min_packet, g_max_packet);
	oprintf("*/\n");
	oprintf("int n_rw = 0;\n");
	oprintf("char buff[64];\n");
	
	dbg("parsing from range %u to %u\n", g_min_packet, g_max_packet);
	p = pcap_open_offline(fileName.c_str(), errbuf);
	if (p == NULL) {
		printf("failed to open %s\n", fileName.c_str());
		exit(1);
	}
	pcap_loop(p, -1, loop_cb, NULL);
	
	if (!g_pending.empty()) {
		printf("WARNING: %d pending requests\n", g_pending.size());
	}
	//Makes copy/pasting easier in some editors...
	oprintf("\n");
	
	return 0;
}

