
#include <iostream>
#include <dlfcn.h>
#include <cstdlib>
#include <linux/input.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <list>
#include "structs.h"

const uint16_t VENDOR  = 0x0547;
const uint16_t PRODUCT = 0x1002;

const uint8_t STATE_CAB_PLAYER_1 = 1;
const uint8_t STATE_CAB_PLAYER_2 = 3;
const uint8_t STATE_PLAYER_1 = 0;
const uint8_t STATE_PLAYER_2 = 2;

const uint8_t PAD_LU = 0x01;
const uint8_t PAD_RU = 0x02;
const uint8_t PAD_CN = 0x04;
const uint8_t PAD_LD = 0x08;
const uint8_t PAD_RD = 0x10;

const uint8_t CAB_SERVICE = 0x40;
const uint8_t CAB_TEST = 0x02;
const uint8_t CAB_COIN = 0x04;

uint8_t IOSTATE[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

struct piu_bind {
    const char* key;
    uint8_t state;
    uint8_t bit;
};

const std::list<piu_bind> binds = {
    { "q",  STATE_PLAYER_1, PAD_LU },
    { "e",  STATE_PLAYER_1, PAD_RU },
    { "s",  STATE_PLAYER_1, PAD_CN },
    { "z",  STATE_PLAYER_1, PAD_LD },
    { "c",  STATE_PLAYER_1, PAD_RD },
    { "F5", STATE_CAB_PLAYER_1, CAB_COIN },

    { "KP_7",  STATE_PLAYER_2, PAD_LU },
    { "KP_9",  STATE_PLAYER_2, PAD_RU },
    { "KP_5",  STATE_PLAYER_2, PAD_CN },
    { "KP_1",  STATE_PLAYER_2, PAD_LD },
    { "KP_3",  STATE_PLAYER_2, PAD_RD },
    { "F6",    STATE_CAB_PLAYER_2, CAB_COIN },

    { "F1", STATE_CAB_PLAYER_1, CAB_TEST },
    { "F2", STATE_CAB_PLAYER_1, CAB_SERVICE },
};

bool is_real_pad_connected = false;
bool was_emulated_device_added = false;

void add_emulated_device() {
    if (!usb_busses) {
        std::cerr << "usb_busses was NULL" << std::endl;
        return;
    }

    for (usb_bus* bus = usb_busses; bus; bus = bus->next) {
        for (usb_device* device = bus->devices; device; device = device->next) {
            if (device->descriptor.idVendor == VENDOR && device->descriptor.idProduct == PRODUCT) {
                is_real_pad_connected = true;
                std::cout << "Real dance pad detected. Not creating emulated device." << std::endl;
                return;
            }
        }
    }

    usb_device* emu_device = new usb_device;
    emu_device->config = new usb_config_descriptor;
    emu_device->config->interface = new usb_interface;
    emu_device->descriptor.idVendor = VENDOR;
    emu_device->descriptor.idProduct = PRODUCT;

    LIST_ADD(usb_busses->devices, emu_device);

    was_emulated_device_added = true;
    std::cout << "Added emulated device." << std::endl;
}

extern "C" usb_dev_handle* usb_open(usb_device* dev) {
    static auto o_usb_open = reinterpret_cast<decltype(&usb_open)>(dlsym(RTLD_NEXT, "usb_open"));

    usb_dev_handle* ret = nullptr;
    if (is_real_pad_connected) {
        ret = o_usb_open(dev);
    } else {
        ret = new usb_dev_handle;
    }

    return ret;
}


extern "C" int usb_claim_interface(usb_dev_handle *dev, int interface) {
    static auto o_usb_claim_interface = reinterpret_cast<decltype(&usb_claim_interface)>(dlsym(RTLD_NEXT, "usb_claim_interface"));

    if (!o_usb_claim_interface) {
        std::cerr << "o_usb_claim_interface was NULL" << std::endl;
        std::exit(1);
    }

    int ret = 0;
    if (is_real_pad_connected) {
        ret = o_usb_claim_interface(dev, interface);
    }

    return ret;
}

extern "C" int usb_find_busses() {
    static auto o_usb_find_busses = reinterpret_cast<decltype(&usb_find_busses)>(dlsym(RTLD_NEXT, "usb_find_busses"));

    if (!o_usb_find_busses) {
        std::cerr << "o_usb_find_busses was NULL" << std::endl;
        std::exit(1);
    }

    int ret = 0;
    if (!was_emulated_device_added || is_real_pad_connected) {
        ret = o_usb_find_busses();
    }

    return ret;
}

extern "C" int usb_find_devices() {
    static auto o_usb_find_devices = reinterpret_cast<decltype(&usb_find_devices)>(dlsym(RTLD_NEXT, "usb_find_devices"));

    if (!o_usb_find_devices) {
        std::cerr << "o_usb_find_busses was NULL" << std::endl;
        std::exit(1);
    }

    int ret = 0;
    if (!was_emulated_device_added) {
        ret = o_usb_find_devices();
        add_emulated_device();

        if (!is_real_pad_connected) {
            ret = 1;
        }
    }

    return ret;
}

extern "C" int usb_set_configuration(usb_dev_handle* dev, int configuration) {
    static auto o_usb_set_configuration = reinterpret_cast<decltype(&usb_set_configuration)>(dlsym(RTLD_NEXT, "usb_set_configuration"));

    if (!o_usb_set_configuration) {
        std::cerr << "o_usb_set_configuration was NULL" << std::endl;
        std::exit(1);
    }

    int ret = 0;
    if (is_real_pad_connected) {
        ret = o_usb_set_configuration(dev, configuration);
    }

    return ret;
}

extern "C" int usb_control_msg(usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout) {
    static auto o_usb_control_msg = reinterpret_cast<decltype(&usb_control_msg)>(dlsym(RTLD_NEXT, "usb_control_msg"));

    if (!o_usb_control_msg) {
        std::cerr << "o_usb_control_msg was NULL" << std::endl;
        std::exit(1);
    }

    int ret = 8;
    if (is_real_pad_connected) {
        ret = o_usb_control_msg(dev, requesttype, request, value, index, bytes, size, timeout);
    }

    if (requesttype == 0xC0) {
        for (int i = 0; i < sizeof(IOSTATE); i++) {
            is_real_pad_connected ? bytes[i] &= IOSTATE[i] : bytes[i] = IOSTATE[i];
        }
    }

    return ret;
}

extern "C" int XNextEvent(Display* display, XEvent* event_return) {
    static auto o_x_next_event = reinterpret_cast<decltype(&XNextEvent)>(dlsym(RTLD_NEXT, "XNextEvent"));

    if (!o_x_next_event) {
        std::cerr << "o_x_next_event was NULL" << std::endl;
        std::exit(1);
    }

    int ret = o_x_next_event(display, event_return);

    if (event_return->type <= 3)
        for (const auto& bind : binds)
            if (event_return->xkey.keycode == XKeysymToKeycode(display, XStringToKeysym(bind.key)))
                IOSTATE[bind.state] ^= bind.bit;

    return ret;
}