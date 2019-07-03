/*
#define CATCH_CONFIG_RUNNER
#include "tests/catch2.h"

int main( int argc, char* argv[] ) {

    // https://github.com/ifknot/Catch2
    return Catch::Session().run( argc, argv );

}
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <string.h>

int main(int argc, char* argv[])
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb1", O_RDWR);
    if (!fbfd) {
        printf("Error: cannot open framebuffer device.\n");
        return(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed information.\n");
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable information.\n");
    }
    printf("%dx%d, %d bpp\n", vinfo.xres, vinfo.yres,
           vinfo.bits_per_pixel );

    // map framebuffer to user memory
    screensize = finfo.smem_len;

    int tty_fd = open("/dev/tty0", O_RDWR);
    ioctl(tty_fd,KDSETMODE,KD_GRAPHICS);

    fbp = (char*)mmap(0,
                      screensize,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      fbfd, 0);



        // draw...
        // just fill upper half of the screen with something
        memset(fbp, 0xff, screensize/2);
        // and lower half with something else
        memset(fbp + screensize/2, 0x18, screensize/2);


        usleep(1000000);

    // cleanup
    munmap(fbp, screensize);
    close(fbfd);

    ioctl(tty_fd,KDSETMODE,KD_TEXT);
    return 0;
}
