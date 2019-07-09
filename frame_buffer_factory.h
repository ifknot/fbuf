#ifndef FBUF_FRAME_BUFFER_FACTORY_H
#define FBUF_FRAME_BUFFER_FACTORY_H

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

    /**
     * LCD map /dev/fb1 directly on the RPi ie no double buffering
     * HDMI map /dev/fb0 on the RPi *uses* double buffering
     */
    enum screen_t {LCD, HDMI};

    /**
     * LCD frame buffer factory creates objects memory mapping the linux display screen buffer
     * @tparam LCD
     */
    template<screen_t LCD>
    class frame_buffer_factory {

        const int DEFAULT_BPB = 16; //RPi 3.5" LCD 16 bits per pixel 65536 colours XGA "High Color"

    public:

        using pixel_t = uint16_t; //RPi 3.5" LCD 16 bits per pixel

        //Raspbian RPi 3.5" LCD on 2nd frame buffer ie /dev/fb1
        frame_buffer_factory(const std::string device_path = "/dev/fb1"): device_path(device_path) {
            open_buffer();
        }

        pixel_t rgb(uint8_t r, uint8_t g, uint8_t b) {
            return (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset);
        }

        void pixel(uint x, uint y, pixel_t colour) {
            *((pixel_t*) (fbmap + ((x + vinfo.xoffset) << 1) + (y + vinfo.yoffset) * finfo.line_length)) = colour;
        }

        void clear() {
            for (size_t i{0}; i < (screensize / 8); ++i) {
                ((uint64_t*)(fbmap))[i] = 0u;
            }
        }

        void fill(pixel_t colour)  {
            uint32_t c = colour;
            c <<= 16;
            c |= colour;
            //c <<= 32;
            //c |= colour;
            //c <<= 48;
            //c |= colour;
            for (size_t i{0}; i < (screensize / 8); ++i) {
                ((uint32_t*)(fbmap))[i] = c;
            }
        }

        ~frame_buffer_factory()  {
            close_buffer();
        }

        std::string variable_info()  {
            std::stringstream ss;
            ss  << "\nxres\t\t" << vinfo.xres
                << "\nyres\t\t" << vinfo.yres
                << "\nbuffer addr\t" << std::hex << static_cast<const void *>(fbmap)
                << "\nscreen memory\t" << vinfo.yres_virtual * finfo.line_length << " bytes"
                << "\nxres_virtual\t" << std::dec << vinfo.xres_virtual
                << "\nyres_virtual\t" << vinfo.yres_virtual
                << "\nxoffset\t\t" << vinfo.xoffset
                << "\nyoffset\t\t" << vinfo.yoffset
                << "\nbits_per_pixel\t" << vinfo.bits_per_pixel
                << "\ngrayscale\t" << vinfo.grayscale
                << "\nnonstd\t\t" << vinfo.nonstd
                << "\nactivate\t" << vinfo.activate
                << "\nheight\t\t" << vinfo.height << "mm"
                << "\nwidth\t\t" << vinfo.width << "mm\n";
            return ss.str();
        }

    private:

        void open_buffer() {
            fbfd = open(device_path.c_str(), O_RDWR);
            if(fbfd != -1) {
                ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); // acquire variable info
                vinfo.grayscale = 0; // ensure colour
                vinfo.bits_per_pixel = DEFAULT_BPB; //ensure 16 bits per pixel
                ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);
                ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); // re-acquire variable info
                ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo); // aquire fixed info
                screensize = vinfo.yres_virtual * finfo.line_length;
                fbmap = static_cast<uint8_t *>(mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0));
                return;
            } else {
                throw std::invalid_argument(strerror(errno));
            }
        }

        void close_buffer()  {
            munmap(fbmap, screensize);
            if (close(fbfd) == 0)
                return;
            else
                throw std::invalid_argument(strerror(errno));
        }

        std::string device_path;
        int fbfd{-1}; //frame buffer file descriptor
        uint32_t screensize{0};
        uint8_t* fbmap{0}; //frame buffer memory map

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

    };

}

#endif // __linux__

#endif //FBUF_FRAME_BUFFER_FACTORY_H
