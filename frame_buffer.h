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
#include <stdexcept>

namespace linux_util {

    static const std::string FRAME_BUFFER_PATH = "/dev/fb1";

    class frame_buffer {

    public:

        frame_buffer(const std::string device_path = FRAME_BUFFER_PATH);

        bool open_buffer();

        bool close_buffer();

        size_t size();

        std::pair<int, int> dimensions();

        size_t bits_per_pixel();

        uint16_t rgb(uint8_t r, uint8_t g, uint8_t b);

        void clear(uint32_t colour = 0xFFFFFFFF);

    private:

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
