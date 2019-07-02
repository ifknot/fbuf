
#define CATCH_CONFIG_RUNNER
#include "tests/catch2.h"

int main( int argc, char* argv[] ) {

    // https://github.com/ifknot/Catch2
    return Catch::Session().run( argc, argv );

}

/*
int main(int argc, char* argv[]) {
    int fbfd = 0; // framebuffer filedescriptor
    struct fb_var_screeninfo var_info;

    // Open the framebuffer device file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        printf("Error: cannot open framebuffer device.\n");
        return(1);
    }
    printf("The framebuffer device opened.\n");

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &var_info)) {
        printf("Error reading variable screen info.\n");
    }
    printf("Display info %dx%d, %d bpp\n",
           var_info.xres, var_info.yres,
           var_info.bits_per_pixel );

    // close file
    close(fbfd);

    return 0;

}
*/