#ifndef FBUF_FRAME_BUFFER_H
#define FBUF_FRAME_BUFFER_H

#if (defined (LINUX) || defined (__linux__))

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <string>
#include <sstream>
#include <stdexcept>


namespace linux_util {

    static const std::string FRAME_BUFFER_PATH = "/dev/fb1"; //Raspbian RPi 3.5" LCD on 2nd frame buffer
    static const int DEFAULT_BPB = 16; //RPi 3.5" LCD 16 bits per pixel 65536 colours XGA "High Color"

    class frame_buffer {

    public:

        using pixel_t = uint16_t; //RPi 3.5" LCD 16 bits per pixel

        frame_buffer(const std::string device_path = FRAME_BUFFER_PATH);

        size_t size();

        pixel_t rgb(uint8_t r, uint8_t g, uint8_t b);

        void pixel(uint x, uint y, pixel_t colour);

        void clear(pixel_t colour = 0u);

        ~frame_buffer();

#ifndef NDEBUG
        std::string info();
#endif

#ifdef NDEBUG
    private:
#endif

        bool open_buffer();

        bool close_buffer();

        std::string device_path;

        int fbfd{-1}; //frame buffer file descriptor

        /**
         * Used to describe the features of a video card that are _user_ defined.
         * With ```fb_var_screeninfo``` such as depth and the resolution can be defined.
         */
        struct fb_var_screeninfo vinfo;

        /**
         *  Defines the _fixed_ properties of a video card that are created when a mode is set.
         *  E.g. the start of the frame buffer memory - the address of the frame buffer memory cannot be changed or moved.
         */
        struct fb_fix_screeninfo finfo;

        /**
         * frame buffer size (bytes)
         */
        size_t size_{0};

        /**
         * frame buffer memory map
         */
         uint8_t* fbmap;

    };

}

#endif // __linux__

#endif //FBUF_FRAME_BUFFER_H
