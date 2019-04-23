#include <errno.h> 
#include <signal.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <libusb-1.0/libusb.h> 
 
#define VENDOR_ID 0x1a86 
 
#define PRODUCT_ID 0xe010

#define GPIO_NUM    4

enum {
    GPIO_LOW,
    GPIO_HIGH
};

enum {
    GPIO_1=0,
    GPIO_2,
    GPIO_3,
    GPIO_4,
};

#define CH9328_SET_DIRECTION          0xC0
#define CH9328_SET_VALUE              0xB0

 
// HID Class-Specific Requests values. See section 7.2 of the HID specifications 
#define HID_GET_REPORT                0x01 
#define HID_GET_IDLE                  0x02 
#define HID_GET_PROTOCOL              0x03 
#define HID_SET_REPORT                0x09 
#define HID_SET_IDLE                  0x0A 
#define HID_SET_PROTOCOL              0x0B 
#define HID_REPORT_TYPE_INPUT         0x01 
#define HID_REPORT_TYPE_OUTPUT        0x02 
#define HID_REPORT_TYPE_FEATURE       0x03 
 
 
#define CTRL_IN		LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE 
#define CTRL_OUT	LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE 
 
 
const static int PACKET_CTRL_LEN=2;  
 
const static int PACKET_INT_LEN=2; 
const static int INTERFACE=0; 
const static int ENDPOINT_INT_IN=0x81; /* endpoint 0x81 address for IN */ 
const static int ENDPOINT_INT_OUT=0x01; /* endpoint 1 address for OUT */ 
const static int TIMEOUT=5000; /* timeout in ms */ 
 
 
void bad(const char *why) 
{ 
    fprintf(stderr,"Fatal error> %s\n",why); 
    exit(17); 
} 
 
 
static struct libusb_device_handle *devh = NULL; 

static int find_ch9328_hidusb(void) 
{ 
    devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID); 
    return devh ? 0 : -EIO; 
}

static int test_set_gpio_value(uint8_t value) 
{ 
    int r,i; 
    char answer[8]; 
    char question[8];
    uint8_t gpio_status = 0;
    
    question[0]=CH9328_SET_VALUE;
    question[1]=value; 
    r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_OUTPUT<<8)|0x00, 0, question, 8,TIMEOUT); 
    if (r < 0) { 
        fprintf(stderr, "Control Out error %d\n", r); 
        return r; 
    } 
 
    return 0; 
}

static int test_set_gpio_direction(uint8_t value) 
{ 
    int r,i; 
    char answer[8]; 
    char question[8];
    uint8_t gpio_status = 0;
    question[0]=CH9328_SET_DIRECTION;
    question[1]=value; 
    
    r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_OUTPUT<<8)|0x00, 0, question, 8,TIMEOUT); 
    if (r < 0) { 
        fprintf(stderr, "Control Out error %d\n", r); 
        return r; 
    }
    
    printf("\n"); 
 
    return 0; 
}
 
static int test_get_gpio_in(void) 
{ 
    int r,i; 
    char answer[PACKET_CTRL_LEN]; 
    char question[PACKET_CTRL_LEN];
    uint8_t gpio_status = 0;
    question[0]=0x20;
    question[1]=0x0; 
    r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00, 0,question, PACKET_CTRL_LEN,TIMEOUT); 
    if (r < 0) { 
        fprintf(stderr, "Control Out error %d\n", r); 
        return r; 
    } 

    r = libusb_control_transfer(devh,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,0, answer,PACKET_CTRL_LEN, TIMEOUT); 
    if (r < 0) { 
        fprintf(stderr, "Control IN error %d\n", r); 
        return r; 
    }
    //printf("%02x, %02x; \n",question[0],answer[0]);
    gpio_status = answer[0] & 0xf;
    for(i = 0;i < GPIO_NUM; i++) {
        printf("GPIO[%d] ", i+1);
        if (gpio_status & 1 << i)
            printf("In = [%d]\n", GPIO_HIGH);
        else
            printf("In = [%d]\n", GPIO_LOW);
    } 
    printf("\n"); 
 
    return 0; 
}

static int test_gpio_output(void) 
{ 
    uint8_t test_gpio = GPIO_4, test_output = GPIO_HIGH;
    uint8_t value = 0;
    
    switch (test_gpio)
    {
        case GPIO_1:
            value = test_output << GPIO_1;
            break;
        case GPIO_2:
            value = test_output << GPIO_2;
            break;
        case GPIO_3:
            value = test_output << GPIO_3;
            break;
        case GPIO_4:
            value = test_output << GPIO_4;
            break;
        default:
            break;
    }
    //value = 0; set direction to INPUT
    test_set_gpio_direction(value);
    test_set_gpio_value(value);
 
    return 0; 
}
 
int main(void) 
{ 
    int r = 1; 
 
    r = libusb_init(NULL); 
    if (r < 0) { 
        fprintf(stderr, "Failed to initialise libusb\n"); 
        exit(1); 
    } 
 
    r = find_ch9328_hidusb(); 
    if (r < 0) { 
        fprintf(stderr, "Could not find/open CH9328 Generic HID device\n"); 
        goto out; 
    } 
    printf("Successfully find the CH9328 Generic HID device\n"); 
 
#ifdef LINUX
    if(libusb_kernel_driver_active(devh, 0) == 1)
	{
		libusb_detach_kernel_driver(devh, 0);
        printf("Successfully libusb_detach_kernel_driver\n");
	}
#endif

#if 0 
    r = libusb_set_configuration(devh, 1); 
    if (r < 0) { 
        fprintf(stderr, "libusb_set_configuration error %d\n", r); 
        goto out; 
    }
    printf("Successfully set usb configuration 1\n");
#endif
    r = libusb_claim_interface(devh, 0); 
    if (r < 0) { 
        fprintf(stderr, "libusb_claim_interface error %d\n", r); 
        goto out; 
    } 
    printf("Successfully claimed interface\n"); 
 
    printf("test_get_gpio_in\n"); 
    //test_gpio_output();
    test_get_gpio_in(); 
 
    libusb_release_interface(devh, 0); 
out: 
 //	libusb_reset_device(devh); 
    libusb_close(devh); 
    libusb_exit(NULL); 
    return r >= 0 ? r : -r; 
} 
 
