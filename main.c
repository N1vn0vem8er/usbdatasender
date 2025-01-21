#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <libusb-1.0/libusb.h>
#include <string.h>
#include <sysexits.h>
#include <sys/stat.h>

const char* VERSION = "1.0.0";
const char* NAME = "Usb Data Sender";
const char* BINARYNAME = "usbdatasender";

int detatchKernalDriver = 1;

int extendedLogging = 0;

int readFromFile = 0;

void printHelp()
{
    printf("Usage: %s [options] [args]\n", BINARYNAME);
    printf("Options:\n\t-h\tshow help\n\t-v\tshow version\n\t-d\textra debug information\n\t-l\tlist connected devices\n\t-d\tset detach kernel driver\n\t-c\tcontrol transfer\n\tusage:\n\t\t-c vendorId:productId interface request_type bRequest wValue wIndex wLength timeout data\n\t\tAll hex except tiemout in decimal\n\n\t-b\tbulk transfer\n\tusage:\n\t\t-b vendorId:productId interface endpoint length timeout data\n\t\tAll hex except tiemout in decimal\n\n\t-i\tinterrupt transfer\n\n\tusage:\n\t\t-i vendorId:productId interface endpoint length timeout data\n\t\tAll hex except tiemout in decimal\n\t\t-f\t\tread data from file\n");
}

int to_uint8_t(const char* in, uint8_t* out)
{
    char* endptr;
    if(strlen(in) > 1 && in[1] == 'x')
        *out = (uint8_t)strtol(in, &endptr, 0);
    else
        *out = (uint8_t)strtol(in, &endptr, 16);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint8_t", in);
        return 1;
    }
    return 0;
}
int to_uint16_t(const char* in, uint16_t* out)
{
    char* endptr;
    if(strlen(in) > 1 && in[1] == 'x')
        *out = (uint16_t)strtol(in, &endptr, 0);
    else
        *out = (uint16_t)strtol(in, &endptr, 16);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint16_t", in);
        return 1;
    }
    return 0;
}
int toInt(const char* in, int* out)
{
    char* endptr;
    if(strlen(in) > 1 && in[1] == 'x')
        *out = (uint16_t)strtol(in, &endptr, 0);
    else
        *out = (uint16_t)strtol(in, &endptr, 10);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint16_t", in);
        return 1;
    }
    return 0;
}
int to_uchar(const char* in, unsigned char* out)
{
    char* endptr;
    if(strlen(in) > 1 && in[1] == 'x')
        *out = (uint16_t)strtol(in, &endptr, 0);
    else
        *out = (uint16_t)strtol(in, &endptr, 16);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint16_t", in);
        return 1;
    }
    return 0;
}
int getVendorId(const char* in, uint16_t* out)
{
    char* ch = strchr(in, ':');
    char* endptr;
    int pos = ch - in;
    char* vId = malloc((pos + 1) * sizeof(char));
    memcpy(vId, in, pos);
    if(strlen(in) > 1 && vId[1] == 'x')
        *out = (uint16_t)strtol(vId, &endptr, 0);
    else
        *out = (uint16_t)strtol(vId, &endptr, 16);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint16_t", in);
        return 1;
    }
    return 0;
}
int getProductId(const char* in, uint16_t* out)
{
    char* ch = strchr(in, ':');
    char* endptr;
    ch++;
    int pos = strlen(ch);
    char* pId = malloc(4 * sizeof(char));
    memcpy(pId, ch, pos);
    if(strlen(in) > 1 && pId[1] == 'x')
        *out = (uint16_t)strtol(pId, &endptr, 0);
    else
        *out = (uint16_t)strtol(pId, &endptr, 16);
    if(in == endptr)
    {
        printf("ERROR: Coudn't parse char* %s to uint16_t", in);
        return 1;
    }
    return 0;
}

int dataFromAscii(const char* in, unsigned char** out)
{
    char* buffer = malloc(2*sizeof(char));
    if(strlen(in) % 2 == 0)
    {
        *out = malloc((strlen(in)/2)*sizeof(char));
        int iindex;
        if(strlen(in) > 1 && in[1] == 'x')
            iindex = 2;
        else
            iindex = 0;
        for(int i=0;i<strlen(in)/2;i++)
        {
            buffer[0] = in[iindex];
            buffer[1] = in[iindex+1];
            (*out)[i] = (unsigned char) strtol(buffer, NULL, 16);
            iindex+=2;
            free(buffer);
            buffer = malloc(2*sizeof(char));
        }
    }
    else
    {
        *out = malloc((strlen(in)/2 + 1)*sizeof(char));
        int iindex;
        if(strlen(in) > 1 && in[1] == 'x')
        {
            iindex = 3;
            buffer[0] = in[2];
            buffer[1] = '\0';
            (*out)[0] = (unsigned char) strtol(buffer, NULL, 16);
        }
        else
        {
            iindex = 1;
            buffer[0] = in[0];
            buffer[1] = '\0';
            (*out)[0] = (unsigned char) strtol(buffer, NULL, 16);
        }
        for(int i=1;i<strlen(in)/2 + 1;i++)
        {
            buffer[0] = in[iindex];
            buffer[1] = in[iindex+1];
            (*out)[i] = (unsigned char) strtol(buffer, NULL, 16);
            iindex+=2;
            free(buffer);
            buffer = malloc(2*sizeof(char));
        }
    }
    free(buffer);
    return 0;
}
int readDataFromFile(char* path, unsigned char** out)
{
    FILE* file;
    file = fopen(path, "rb");
    struct stat stats;
    stat(path, &stats);
    int size = stats.st_size;
    out = malloc(size);
    fread(out, size, 1, file);
    fclose(file);
    return 0;
}

int controlTransfer(char* vpId, char* iF, char* type, char* bR, char* wV, char* wI, char* wL, char* tO, char* d)
{
    if(strlen(vpId) > 9)
    {
        printf("ERROR: %s is not a valid vendorId:productId", vpId);
        return EX_DATAERR;
    }
    int err;
    uint8_t request_type;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    unsigned char* data = NULL;
    uint16_t wLength;
    unsigned int timeOut = atoi(tO);
    int detatchKernalDriver = 0;
    uint16_t productId;
    uint16_t vendorId;
    int interface;
    if(extendedLogging)
        puts("Parsing data...");
    if(to_uint8_t(type, &request_type))
        return EX_DATAERR;
    if(to_uint8_t(bR, &bRequest))
        return EX_DATAERR;
    if(to_uint16_t(wV, &wValue))
        return EX_DATAERR;
    if(to_uint16_t(wI, &wIndex))
        return EX_DATAERR;
    if(to_uint16_t(wL, &wLength))
        return EX_DATAERR;
    if(dataFromAscii(d, &data))
        return EX_DATAERR;
    if(getProductId(vpId, &productId))
        return EX_DATAERR;
    if(getVendorId(vpId, &vendorId))
        return EX_DATAERR;
    if(toInt(iF, &interface))
        return EX_DATAERR;
    if(extendedLogging)
        puts("Done");

    printf("Preparing to send control transfer with the following settings:\nVendor: %u\nProduct: %u\nInterface: %u\nRequest Type: %u\nbRequest: %u\nwValue: %u\nwIndex: %u\nwLength: %u\n\nTimeout after: %u\n", vendorId, productId, interface, request_type, bRequest, wValue, wIndex, wLength, timeOut);

    libusb_context* context = NULL;
    if(extendedLogging)
        puts("Initializing libusb");
    libusb_init(&context);

    libusb_device** list;
    libusb_device* device = NULL;

    int listLength = libusb_get_device_list(NULL, &list);
    if(extendedLogging)
        puts("Searching for device");
    for(int i=0;i<listLength;i++)
    {
        libusb_device* dev = list[i];
        struct libusb_device_descriptor desc = {0};
        libusb_get_device_descriptor(dev, &desc);
        if(desc.idVendor == vendorId && desc.idProduct == productId)
        {
            device = dev;
            break;
        }
    }
    libusb_free_device_list(list, 1);
    if(device == NULL)
    {
        puts("ERROR: Device not found");
        libusb_exit(context);
        return EX_CONFIG;
    }
    if(extendedLogging)
    {
        puts("Device found");
        puts("Opening device");
    }
    libusb_device_handle* handle;
    err = libusb_open(device, &handle);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    err = libusb_set_auto_detach_kernel_driver(handle, detatchKernalDriver);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Claiming interface %u", interface);
    err = libusb_claim_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Sending data");
    err = libusb_control_transfer(handle, request_type, bRequest, wValue, wIndex, data, wLength, timeOut);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Releasing interface %u", interface);
    err = libusb_release_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Closing device");
    libusb_close(handle);
    if(extendedLogging)
        puts("Done");
    if(extendedLogging)
        puts("Exiting libusb");
    libusb_exit(context);
    if(extendedLogging)
        puts("Done");
    puts("Data after");

    int arrayLength = 0;
    if(strlen(d) % 2 == 0)
        arrayLength = strlen(d)/2;
    else
        arrayLength = strlen(d)/2 + 1;
    for(int i=0; i<arrayLength; i++)
    {
        printf("%x", data[i]);
    }
    free(data);
    return 0;
}
int bulkTransfer(char* vpId, char* iF, char* eP, char* l, char* tO, char* d)
{
    int err;
    uint16_t vendorId;
    uint16_t productId;
    int interface;
    unsigned char endpoint;
    unsigned char* data;
    int length;
    int transferred = 0;
    unsigned int timeout;
    if(extendedLogging)
        puts("Parsing data...");
    if(getVendorId(vpId, &vendorId))
        return EX_DATAERR;
    if(getProductId(vpId, &productId))
        return EX_DATAERR;
    if(toInt(iF, &interface))
        return EX_DATAERR;
    if(dataFromAscii(d, &data))
        return EX_DATAERR;
    if(toInt(l, &length))
        return EX_DATAERR;
    if(to_uchar(eP, &endpoint))
        return EX_DATAERR;
    timeout = atoi(tO);
    if(extendedLogging)
        puts("Done");
    printf("Preparing to send bulk transfer with the following settings:\nVendor: %u\nProduct: %u\nInterface: %u\nEndpoint: %u\n\nTimeout after %u\n", vendorId, productId, interface, endpoint, timeout);

    libusb_context* context = NULL;
    if(extendedLogging)
        puts("Initializing libusb");
    libusb_init(&context);

    libusb_device** list;
    libusb_device* device = NULL;

    int listLength = libusb_get_device_list(NULL, &list);
    if(extendedLogging)
        puts("Searching for device");
    for(int i=0;i<listLength;i++)
    {
        libusb_device* dev = list[i];
        struct libusb_device_descriptor desc = {0};
        libusb_get_device_descriptor(dev, &desc);
        if(desc.idVendor == vendorId && desc.idProduct == productId)
        {
            device = dev;
            break;
        }
    }
    libusb_free_device_list(list, 1);
    if(device == NULL)
    {
        puts("ERROR: Device not found");
        libusb_exit(context);
        return EX_CONFIG;
    }
    if(extendedLogging)
    {
        puts("Device found");
        puts("Opening device");
    }
    libusb_device_handle* handle;
    err = libusb_open(device, &handle);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    err = libusb_set_auto_detach_kernel_driver(handle, detatchKernalDriver);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Claiming interface %u", interface);
    err = libusb_claim_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Sending data");
    err = libusb_bulk_transfer(handle, endpoint, data, length, &transferred, timeout);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Releasing interface %u", interface);
    err = libusb_release_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Closing device");
    libusb_close(handle);
    if(extendedLogging)
        puts("Done");
    if(extendedLogging)
        puts("Exiting libusb");
    libusb_exit(context);
    if(extendedLogging)
        puts("Done");
    printf("Transferred: %u\n", transferred);
    puts("Data after");
    int arrayLength = 0;
    if(strlen(d) % 2 == 0)
        arrayLength = strlen(d)/2;
    else
        arrayLength = strlen(d)/2 + 1;
    for(int i=0; i<arrayLength; i++)
    {
        printf("%x", data[i]);
    }

    free(data);
    return 0;
}
int interruptTransfer(char* vpId, char* iF, char* eP, char* l, char* tO, char* d)
{
    int err;
    uint16_t vendorId;
    uint16_t productId;
    int interface;
    unsigned char endpoint;
    unsigned char* data;
    int length;
    int transferred = 0;
    unsigned int timeout;

    if(extendedLogging)
        puts("Parsing data...");
    if(getVendorId(vpId, &vendorId))
        return EX_DATAERR;
    if(getProductId(vpId, &productId))
        return EX_DATAERR;
    if(dataFromAscii(d, &data))
        return EX_DATAERR;
    if(toInt(iF, &interface))
        return EX_DATAERR;
    if(toInt(l, &length))
        return EX_DATAERR;
    if(to_uchar(eP, &endpoint))
        return EX_DATAERR;
    timeout = atoi(tO);
    if(extendedLogging)
        puts("Done");

    printf("Preparing to send interrupt transfer with the following settings:\nVendor: %u\nProduct: %u\nInterface: %u\nEndpoint: %u\n\nTimeout after %u\n", vendorId, productId, interface, endpoint, timeout);

    libusb_context* context = NULL;
    if(extendedLogging)
        puts("Initializing libusb");
    libusb_init(&context);

    libusb_device** list;
    libusb_device* device = NULL;

    int listLength = libusb_get_device_list(NULL, &list);
    if(extendedLogging)
        puts("Searching for device");
    for(int i=0;i<listLength;i++)
    {
        libusb_device* dev = list[i];
        struct libusb_device_descriptor desc = {0};
        libusb_get_device_descriptor(dev, &desc);
        if(desc.idVendor == vendorId && desc.idProduct == productId)
        {
            device = dev;
            break;
        }
    }
    libusb_free_device_list(list, 1);
    if(device == NULL)
    {
        puts("ERROR: Device not found");
        libusb_exit(context);
        return EX_CONFIG;
    }
    if(extendedLogging)
    {
        puts("Device found");
        puts("Opening device");
    }
    libusb_device_handle* handle;
    err = libusb_open(device, &handle);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    err = libusb_set_auto_detach_kernel_driver(handle, detatchKernalDriver);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Claiming interface %u", interface);
    err = libusb_claim_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Sending data");
    err = libusb_interrupt_transfer(handle, endpoint, data, length, &transferred, timeout);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        printf("Releasing interface %u", interface);
    err = libusb_release_interface(handle, interface);
    if(err)
    {
        puts(libusb_error_name(err));
    }
    if(extendedLogging)
        puts("Closing device");
    libusb_close(handle);
    if(extendedLogging)
        puts("Done");
    if(extendedLogging)
        puts("Exiting libusb");
    libusb_exit(context);
    if(extendedLogging)
        puts("Done");
    printf("Transferred: %u\n", transferred);
    puts("Data after");
    int arrayLength = 0;
    if(strlen(d) % 2 == 0)
        arrayLength = strlen(d)/2;
    else
        arrayLength = strlen(d)/2 + 1;
    for(int i=0; i<arrayLength; i++)
    {
        printf("%x", data[i]);
    }

    free(data);
    return 0;
}
int listDevices()
{
    libusb_context* context = NULL;
    if(extendedLogging)
        puts("Initializing libusb");
    libusb_init(&context);

    libusb_device** list;

    int listLength = libusb_get_device_list(NULL, &list);
    if(extendedLogging)
        puts("Searching for device");
    for(int i=0;i<listLength;i++)
    {
        libusb_device* dev = list[i];
        struct libusb_device_descriptor desc = {0};
        libusb_get_device_descriptor(dev, &desc);
        printf("%u:%u\n", desc.idVendor, desc.idProduct);
    }
    libusb_free_device_list(list, 1);
    libusb_exit(context);
    return 0;
}

int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"control_write", required_argument, 0, 'c'},
        {"list_devices", no_argument, 0, 'l'},
        {"bulk_transfer", required_argument, 0, 'b'},
        {"interrupt_transfer", required_argument, 0, 'i'},
        {"detach_kernel_driver", no_argument, 0, 'k'},
        {"logging", no_argument, 0, 'd'}
    };

    int c, option_index = 0;
    while((c=getopt_long(argc, argv, "hvlkdfc:b:i:", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'h':
                printHelp();
                break;
            case 'f':
                readFromFile = 1;
                break;
            case 'c':
                if(argc < 10){
                    puts("ERROR: Not enough arguments");
                    return EX_USAGE;
                }
                return controlTransfer(argv[optind-1], argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], argv[optind+4], argv[optind+5], argv[optind+6], argv[optind+7]);
                break;
            case 'v':
                printf("%s version %s\n", NAME, VERSION);
                break;
            case 'l':
                return listDevices();
                break;
            case 'b':
                if(argc < 7){
                    puts("ERROR: Not enough arguments");
                    return EX_USAGE;
                }
                return bulkTransfer(argv[optind-1], argv[optind], argv[optind+1], argv[optind+2], argv[optind+3], argv[optind+4]);
                break;
            case 'i':
                if(argc < 7){
                    puts("ERROR: Not enough arguments");
                    return EX_USAGE;
                }
                return interruptTransfer((argv[optind - 1]), argv[optind], argv[optind + 1], argv[optind+2], argv[optind+3], argv[optind+4]);
                break;
            case 'k':
                detatchKernalDriver = 0;
                break;
            case 'd':
                extendedLogging = 1;
                break;
        }
    }
    return 0;
}
