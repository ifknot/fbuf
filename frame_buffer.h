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

    static const std::string FRAME_BUFFER_PATH = "/dev/fb0";

    class frame_buffer {

    public:

        frame_buffer(const std::string& device_path);

        int open_buffer();

        //int close_buffer();

        //size_t screensize();

    private:

        std::string device_path;

        int fbfd; //framebuffer filedescriptor

        /**
         * Used to describe the features of a video card that are _user_ defined.
         * With ```fb_var_screeninfo``` such as depth and the resolution can be defined.
         */
        struct fb_var_screeninfo variable_info;

        /**
         *  Defines the _fixed_ properties of a video card that are created when a mode is set.
         *  E.g. the start of the frame buffer memory - the address of the frame buffer memory cannot be changed or moved.
         */
        struct fb_fix_screeninfo fixed_info;

        size_t screensize_;

    };

}

#endif // __linux__

#endif //FBUF_FRAME_BUFFER_H
