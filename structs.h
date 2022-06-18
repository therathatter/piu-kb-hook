#pragma once
#include <limits.h>
#include <cstdint>

// Taken from https://github.com/hjelmn/libusb-compat-0.1/blob/master/libusb/usb.h

#define LIST_ADD(begin, ent)       \
    do                             \
    {                              \
        if (begin)                 \
        {                          \
            ent->next = begin;     \
            ent->next->prev = ent; \
        }                          \
        else                       \
            ent->next = NULL;      \
        ent->prev = NULL;          \
        begin = ent;               \
    } while (0)


typedef struct libusb_device_handle libusb_device_handle;

struct usb_dev_handle
{
    libusb_device_handle *handle;
    struct usb_device *device;

    /* libusb-0.1 is buggy w.r.t. interface claiming. it allows you to claim
	 * multiple interfaces but only tracks the most recently claimed one,
	 * which is used for usb_set_altinterface(). we clone the buggy behaviour
	 * here. */
    int last_claimed_interface;
};

struct usb_bus
{
    struct usb_bus *next, *prev;

    char dirname[PATH_MAX + 1];

    struct usb_device *devices;
    uint32_t location;

    struct usb_device *root_dev;
};


// ripped from IDA
struct usb_endpoint_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
  uint8_t bRefresh;
  uint8_t bSynchAddress;
  uint8_t* xtra;
  int xtralen;
};

// also ripped from IDA
struct usb_interface_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
  usb_endpoint_descriptor *endpoint;
  uint8_t * xtra;
  int xtralen;
};

struct usb_interface
{
    usb_interface_descriptor *altsetting;
    int num_altsetting;
};

struct usb_config_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t MaxPower;

    usb_interface *interface;

    unsigned char *extra; /* Extra descriptors */
    int extralen;
};

/* Device descriptor */
struct usb_device_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
};

struct usb_device
{
    usb_device *next, *prev;

    char filename[PATH_MAX + 1];

    usb_bus *bus;

    usb_device_descriptor descriptor;
    usb_config_descriptor *config;

    void *dev; /* Darwin support */

    uint8_t devnum;

    unsigned char num_children;
    struct usb_device **children;
};

struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;

extern "C" struct usb_bus *usb_busses;

