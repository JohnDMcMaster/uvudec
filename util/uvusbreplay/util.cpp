#include <string>
#include "linux/usb/ch9.h"

bool g_util_halt_error = false;

#define URB_IS_IN( _urb) (((_urb)->endpoint & USB_DIR_IN) == USB_DIR_IN)

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
	//not sure what these are...not labeled in wireshark either
	uint8_t pad[24];
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
		if (g_util_halt_error) {
			exit(1);
		}
		return "";
	}
}

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
			if (g_util_halt_error) {
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
		if (g_util_halt_error) {
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
		if (g_util_halt_error) {
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
		if (g_util_halt_error) {
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
		if (g_util_halt_error) {
			exit(1);
		}
		return "";
	}
}

void print_urb(usb_urb_t *urb) {
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

