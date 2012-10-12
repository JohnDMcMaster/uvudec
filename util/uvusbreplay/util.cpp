#include <string>
#include "util.h"

bool g_util_halt_error = false;

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

