#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <sys/stat.h>
#include "partial.h"
#include "iBSS.n72ap.h"
#include "iBSS.n90ap.h"
#include "libflursch.h"

int main(int argc, char *argv[]) {
    
    libflursch_init();
    
    if (file_exists("images") < 0) {
        if (mkdir("images", 0700) < 0) {
            return EXIT_FAILURE;
        }
    }
    
    if (file_exists("images/iBSS.n72ap.RELEASE.dfu") < 0) {
        dprintf("File not found, downloading file from Apple.");
        if(download_file_from_zip("http://appldnld.apple.com/iPhone4/061-9855.20101122.Lrft6/iPod2,1_4.2.1_8C148_Restore.ipsw", "Firmware/dfu/iBSS.n72ap.RELEASE.dfu","images/iBSS.n72ap.RELEASE.dfu", NULL) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }
    
    if (inject() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    dprintf("Uploading %s.","images/iBSS.n72ap.RELEASE.dfu.");
    if(send_file("images/iBSS.n72ap.RELEASE.dfu", kDFUMode) != EXIT_SUCCESS) {
        dprintf("Failed to upload %s.", "images/iBSS.n72ap.RELEASE.dfu.");
        return -1;
    }
/*
    
    if (file_exists("images/iBSS.n90ap.RELEASE.dfu") < 0) {
        dprintf("File not found, downloading file from Apple.");
        if(download_file_from_zip("http://appldnld.apple.com/iPhone4/061-9858.20101122.Er456/iPhone3,1_4.2.1_8C148_Restore.ipsw", "Firmware/dfu/iBSS.n90ap.RELEASE.dfu","images/iBSS.n90ap.RELEASE.dfu", NULL) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }
    
    if (inject() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    
    dprintf("Uploading %s.","images/iBSS.n90ap.RELEASE.dfu.");
    if(send_file("images/iBSS.n90ap.RELEASE.dfu", kDFUMode) != EXIT_SUCCESS) {
        dprintf("Failed to upload %s.", "images/iBSS.n90ap.RELEASE.dfu.");
        return -1;
    }
*/
    dprintf("Waiting 5 seconds...");
    sleep(5);
    if (connect2(0x1281,10) != EXIT_SUCCESS) {
        dprintf("Cannot connect to device.");
        return EXIT_FAILURE;
    }
    
    if (send_buffer((unsigned char *)iBSS_n72ap, sizeof(iBSS_n72ap), RecoveryMode) != EXIT_SUCCESS) {
        dprintf("Failed to send payload.");
        return EXIT_FAILURE;
    }
    
    if(send_cmd("go") != EXIT_SUCCESS) {
        dprintf("Failed to send command.");
        return EXIT_FAILURE;
    }
    
    libflursch_exit();
    
    return EXIT_SUCCESS;
}
