#ifndef FBUF_FRAME_BUFFER_FACTORY_H
#define FBUF_FRAME_BUFFER_FACTORY_H

#if (defined (LINUX) || defined (__linux__))

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <string>
#include <sstream>
#include <stdexcept>


namespace linux_util {

    /**
     * Raspberry Pi:
     * LCD map /dev/fb1
     * HDMI map /dev/fb0
     */
    enum screen_t {HDMI, LCD};

    /**
     * LCD frame buffer factory creates objects memory mapping the linux display screen buffer
     * @tparam LCD
     */
    template<screen_t HDMI>
    class frame_buffer_factory {

        const uint32_t DEFAULT_BPP = 32;

    public:

        using pixel_t = uint32_t;

        //Raspbian RPi 3.5" LCD on 2nd frame buffer ie /dev/fb1
        frame_buffer_factory(const std::string device_path = "/dev/fb0"): device_path(device_path) {
            open_buffer();
            hide_cursor();
            init_screen();
        }

        void rgb(pixel_t r, pixel_t g, pixel_t b, pixel_t a = 0xFF) {
            colour = (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset) | (a << vinfo.transp.offset);
        }

        void pixel(uint x, uint y) {
            *((pixel_t*) (fbmap + ((x + vinfo.xoffset) >> 1) + (y + vinfo.yoffset) * finfo.line_length)) = colour;
            //((pixel_t*) (fbmap))[((x + vinfo.xoffset) >> 1) + ((y + vinfo.yoffset) * finfo.line_length)] = colour;
        }

        void line(uint x1, uint y1, uint x2, uint y2) {

        }

        void clear() {
            memset(fbmap, 0u, screensize);
        }

        void fill()  {
            uint64_t c = colour;
            c <<= vinfo.bits_per_pixel;
            c |= colour;
            size_t offset{0};
            for(size_t y{0}; y < vinfo.yres >> 1; ++y) {
                for (size_t x{0}; x < vinfo.xres >> 2; ++x) {
                    ((uint64_t *) (fbmap))[offset + x] = c;
                }
                offset += finfo.line_length >> 3;
            }
        }

        void swap() {
            //vinfo.yoffset =  * vinfo.yres;
            ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
            ioctl(fbfd, FBIO_WAITFORVSYNC, 0);
        }

        ~frame_buffer_factory()  {
            restore_screen();
            show_cursor();
            close_buffer();
        }

        std::string variable_info()  {
            vioctl(FBIOGET_VSCREENINFO); // acquire variable info
            std::stringstream ss;
            ss  << "\nxres\t\t" << vinfo.xres
                << "\nyres\t\t" << vinfo.yres
                << "\nbuffer addr\t" << std::hex << static_cast<const void *>(fbmap)
                << "\nline memory\t" << std::dec << linesize
                << "\nscreen memory\t" << screensize << " bytes"
                << "\nxres_virtual\t" << vinfo.xres_virtual
                << "\nyres_virtual\t" << vinfo.yres_virtual
                << "\nxoffset\t\t" << vinfo.xoffset
                << "\nyoffset\t\t" << vinfo.yoffset
                << "\nbits_per_pixel\t" << vinfo.bits_per_pixel
                << "\nr shift\t\t" << vinfo.red.offset
                << "\ng shift\t\t" << vinfo.green.offset
                << "\nb shift\t\t" << vinfo.blue.offset
                << "\nt shift\t\t" << vinfo.transp.offset
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

        void init_screen() {
            vioctl(FBIOGET_VSCREENINFO); // acquire variable info
            memcpy(&vinfo_old, &vinfo, sizeof(struct fb_var_screeninfo)); // copy for restore
            vinfo.grayscale = 0; // ensure colour
            vinfo.bits_per_pixel = DEFAULT_BPP;
            //vinfo.yres_virtual = vinfo.yres * 2; //make space for virtual screen
            vioctl(FBIOPUT_VSCREENINFO);
            vioctl(FBIOGET_VSCREENINFO); // re-acquire variable info
            fioctl(FBIOGET_FSCREENINFO); // acquire fixed info
            linesize = finfo.line_length / 2;
            screensize = vinfo.yres * linesize;
            fbmap = static_cast<uint8_t *>(mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0));
        }

        void restore_screen() {
            if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo_old)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        void hide_cursor() {
            std::string kbfds{"/dev/tty"};
            kbfd = open(kbfds.c_str(), O_WRONLY);
            if (kbfd >= 0) {
                ioctl(kbfd, KDSETMODE, KD_GRAPHICS);
            } else {
                throw std::invalid_argument(strerror(errno));
            }
        }

        void show_cursor() {
            if (kbfd >= 0) {
                ioctl(kbfd, KDSETMODE, KD_TEXT);
                close(kbfd);
            }
        }

        void vioctl(unsigned long request) {
            if (ioctl(fbfd, request, &vinfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        void fioctl(unsigned long request) {
            if (ioctl(fbfd, request, &finfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        std::string device_path;
        int fbfd{-1}; //frame buffer file descriptor
        int kbfd{-1}; //keyboard file descriptor
        uint32_t linesize{0}; //visible line size bytes
        uint32_t screensize{0}; //visible screen size bytes
        uint8_t* fbmap{0}; //frame buffer memory map
        uint8_t* vbmap{0}; //virtual buffer memory map
        pixel_t colour{0};

        /**
         * Used to describe the features of a video card that are _user_ defined.
         * With ```fb_var_screeninfo``` such as depth and the resolution can be defined.
         */
        struct fb_var_screeninfo vinfo;

        /**
         * Original system vinfo for restore
         */
        struct fb_var_screeninfo vinfo_old;

        /**
         *  Defines the _fixed_ properties of a video card that are created when a mode is set.
         *  E.g. the start of the frame buffer memory - the address of the frame buffer memory cannot be changed or moved.
         */
        struct fb_fix_screeninfo finfo;

    };

}

#endif // __linux__

#endif //FBUF_FRAME_BUFFER_FACTORY_H
