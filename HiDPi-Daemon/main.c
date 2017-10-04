//
//  main.c
//  hidpi-daemon
//
//  Created by Edward Pacman on 10/3/17.
//  Copyright © 2017 Edward Pacman. All rights reserved.
//
//  Build for Solve "topleft issues" on Hanckintosh after turning on the HiDPi and after first wake.

#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

#define LAUNCHD_PATH "/Library/LaunchDaemons/com.edward-p.hidpi.daemon.plist"
#define STOP_DAEMON_COMMAND "launchctl unload /Library/LaunchDaemons/com.edward-p.hidpi.daemon.plist" //for unload daemon after first wake.

char _defCommand[128] = "/Applications/RDM.app/Contents/MacOS/SetResX"; //Parameters will be added by genCommand()
char _hidCommand[128] = "/Applications/RDM.app/Contents/MacOS/SetResX";


io_connect_t  root_port; // a reference to the Root Power Domain IOService

struct res_mods {
    int available;
    char width[5];
    char hight[5];
} _defRes = {0, {}, {}}, _hidRes = {0, {}, {}};

void printHelp(void);

int option_o = 0;
int printPlist(void);
void genCommand(void);
int startDeamon(void);

void MySleepCallBack( void * refCon, io_service_t service, natural_t messageType, void * messageArgument )
{
    /*printf( "messageType %08lx, arg %08lx\n",
     (long unsigned int)messageType,
     (long unsigned int)messageArgument );*/
    switch ( messageType )
    {
            
        case kIOMessageCanSystemSleep:
            /* Idle sleep is about to kick in. This message will not be sent for forced sleep.
             Applications have a chance to prevent sleep by calling IOCancelPowerChange.
             Most applications should not prevent idle sleep.
             
             Power Management waits up to 30 seconds for you to either allow or deny idle
             sleep. If you don't acknowledge this power change by calling either
             IOAllowPowerChange or IOCancelPowerChange, the system will wait 30
             seconds then go to sleep.
             */
            
            //Uncomment to cancel idle sleep
            //IOCancelPowerChange( root_port, (long)messageArgument );
            // we will allow idle sleep
            IOAllowPowerChange( root_port, (long)messageArgument );
            break;
            
        case kIOMessageSystemWillSleep:
            /* The system WILL go to sleep. If you do not call IOAllowPowerChange or
             IOCancelPowerChange to acknowledge this message, sleep will be
             delayed by 30 seconds.
             
             NOTE: If you call IOCancelPowerChange to deny sleep it returns
             kIOReturnSuccess, however the system WILL still go to sleep.
             */
            IOAllowPowerChange( root_port, (long)messageArgument );
            break;
            
        case kIOMessageSystemWillPowerOn:
            //System has started the wake up process...
            break;
            
        case kIOMessageSystemHasPoweredOn:
            //System has finished waking up...
            system(_defCommand);
            system(_hidCommand);
            system(STOP_DAEMON_COMMAND); //Only need for first wake I guess.
            break;
            
        default:
            break;
            
    }
}

int startDeamon() {
    // notification port allocated by IORegisterForSystemPower
    IONotificationPortRef  notifyPortRef;
    
    // notifier object, used to deregister later
    io_object_t            notifierObject;
    // this parameter is passed to the callback
    void*                  refCon = NULL;
    
    // register to receive system sleep notifications
    
    root_port = IORegisterForSystemPower( refCon, &notifyPortRef, MySleepCallBack, &notifierObject );
    if ( root_port == 0 )
    {
        printf("IORegisterForSystemPower failed\n");
        return 1;
    }
    
    // add the notification port to the application runloop
    CFRunLoopAddSource( CFRunLoopGetCurrent(),
                       IONotificationPortGetRunLoopSource(notifyPortRef), kCFRunLoopCommonModes );
    
    /* Start the run loop to receive sleep notifications. Don't call CFRunLoopRun if this code
     is running on the main thread of a Cocoa or Carbon application. Cocoa and Carbon
     manage the main thread's run loop for you as part of their event handling
     mechanisms.
     */
    CFRunLoopRun();
    //Not reached, CFRunLoopRun doesn't return in this case.
    return 0;
}

int main( int argc, char *argv[] )
{
    char ag;
    char * token = NULL;
    if (argc == 1) {
        printHelp();
    }
    while ((ag = getopt(argc, argv, "d:r:oh")) != -1) { //deal with args
        switch (ag) {
            case 'd':
                if (strlen(optarg) > 9) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                token = strtok(optarg, "x");
                if (strlen(token) > 5 || token == NULL) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                else {
                    strcpy(_defRes.width, token);
                }
                token = strtok(NULL, "");
                if (strlen(token) > 5 || token == NULL) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                else {
                    strcpy(_defRes.hight, token);
                }
                _defRes.available = 1;
                break;
            case 'r':
                if (strlen(optarg) > 9) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                token = strtok(optarg, "x");
                if (strlen(token) > 5 || token == NULL) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                else {
                    strcpy(_hidRes.width, token);
                }
                token = strtok(NULL, "");
                if (strlen(token) > 5 || token == NULL) {
                    printf("Invalid Parameter: %s", optarg);
                    break;
                }
                else {
                    strcpy(_hidRes.hight, token);
                }
                _hidRes.available = 1;
                break;
            case 'o':
                option_o = 1;
                break;
            case 'h':
                printHelp();
                break;
            case '?':
                printHelp();
                break;
            default:
                printHelp();
                break;
        }
    }
    if (_hidRes.available && _defRes.available && option_o != 1) { //can launch daemon
        genCommand();
        startDeamon();
    }
    else if (_hidRes.available && _defRes.available && option_o == 1) { //export plist file
        if (printPlist() != 0) {
            return 1;
        }
    }
    return 0;
}

void genCommand() {
    strcat(_defCommand, " -w ");
    strcat(_defCommand, _defRes.width);
    strcat(_defCommand, " -h ");
    strcat(_defCommand, _defRes.hight);
    strcat(_defCommand, " -s 1.0");
    //printf("%s\n",_defCommand);
    
    strcat(_hidCommand, " -w ");
    strcat(_hidCommand, _hidRes.width);
    strcat(_hidCommand, " -h ");
    strcat(_hidCommand, _hidRes.hight);
    strcat(_hidCommand, " -s 2.0");
    //printf("%s\n",_hidCommand);
}

int printPlist() {
    FILE * f = NULL;
    f = fopen(LAUNCHD_PATH , "w+");
    if (f == NULL) {
        printf("Can't open file: %s", LAUNCHD_PATH);
        return 1;
    }
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\">\n");
    fprintf(f, "<dict>\n");
    fprintf(f, "    <key>Label</key>\n");
    fprintf(f, "    <string>com.edward-p.hidpi.daemon</string>\n");
    fprintf(f, "    <key>KeepAlive</key>\n");
    fprintf(f, "    <true/>\n");
    fprintf(f, "    <key>Program</key>\n");
    fprintf(f, "    <string>/usr/local/bin/hidpi-daemon</string>\n");
    fprintf(f, "    <key>ProgramArguments</key>\n");
    fprintf(f, "    <array>\n");
    fprintf(f, "        <string>/usr/local/bin/hidpi-daemon</string>\n");
    fprintf(f, "        <string>-d</string>\n");
    fprintf(f, "        <string>%sx%s</string>\n", _defRes.width, _defRes.hight);
    fprintf(f, "        <string>-r</string>\n");
    fprintf(f, "        <string>%sx%s</string>\n", _hidRes.width, _hidRes.hight);
    fprintf(f, "    </array>\n");
    fprintf(f, "    <key>RunAtLoad</key>\n");
    fprintf(f, "    <true/>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "</plist>\n");
    fclose(f);
    return 0;
}

void printHelp() {
    printf("\n\n Usage:\thidpi-daemon <[Options] [Values]> <[Option]>\n\n");
    printf(" Options:\n\n");
    printf("    -d\tDefault Resolution: 1920x1080 etc.\n");
    printf("    -r\tHiDPi(Retina) Resolution: 1600x900 etc.\n");
    printf("    -o\tExport launchd.plist to file.\n");
    printf("    -h\tPrint this help.\n\n");
    printf(" examples:\n\n");
    printf("    hidpi-daemon -d 1920x1080 -r 1600x900\n");
    printf(" Note:\n");
    printf("    Assume that you have installed it as /usr/local/bin/hidpi-deamon\n");
    printf("    Assume that you have installed RDM.app in /Application\n");
    printf("    sudo hidpi-daemon -d 1920x1080 -r 1600x900 -o\n");
    printf("        Then launchd.plist will appeared into /Library/LaunchDaemons\n");
    printf("        as \"com.edward-p.hidpi.daemon.plist\"\n\n");
}

