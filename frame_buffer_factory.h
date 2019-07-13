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

        frame_buffer_factory(const std::string device_path = "/dev/fb0"): device_path(device_path) {
            open_buffer();
            init_screen();
            hide_cursor();
        }

        void rgb(pixel_t r, pixel_t g, pixel_t b, pixel_t a = 0xFF) {
            colour = (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset) | (a << vinfo.transp.offset);
        }

        void clear() {
            memset(fbmap, 0u, screensize);
        }

        void fill(size_t x, size_t y, size_t width, size_t height)  {
            uint64_t c = colour;
            c <<= vinfo.bits_per_pixel;
            c |= colour;
            size_t h{y + height};
            size_t w{(x + width) >> 1}; // 2 x 32bit pixel_t per uint64_t
            size_t linesize{finfo.line_length >> 3}; // 4 x 8bit bytes per pixel_t
            size_t offset{y * linesize};
            for(size_t i{y}; i < h; ++i) {
                for (size_t j{(x >> 1)}; j < w; ++j) {
                    ((uint64_t *) (fbmap))[offset + j] = c;
                }
                offset += linesize;
            }
        }

        inline void pixel(uint x, uint y) {
            ((pixel_t *) (fbmap))[(y * vinfo.xres) + x] = colour;
        }

        void hline(size_t x, size_t y, size_t width)  {
            y *= vinfo.xres;
            y += x;
            for(size_t i{0}; i < width; ++i) {
                ((pixel_t *) (vbmap))[y + i] = colour;
            }
        }

        void vline(size_t x, size_t y, size_t height)  {
            y *= vinfo.xres;
            y += x;
            for(size_t i{0}; i < height; ++i) {
                ((pixel_t *) (vbmap))[y] = colour;
                y += vinfo.xres;
            }
        }

        void rect(size_t x, size_t y, size_t width, size_t height)  {
            hline(x, y, width);
            vline(x, y, height);
            hline(x, y + height, width);
            vline(x + width, y, height);
        }

        void line(uint x1, uint y1, uint x2, uint y2) {

        }

        void swap() {
            std::swap(fbmap, vbmap);
        }

        void slide() {

            vinfo.yoffset = (vinfo.yoffset == 0) ?vinfo.yres :0u;

            ioctl(fbfd, FBIO_WAITFORVSYNC, 0);
            ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

        }
        void slideb() {

            vinfo.yoffset = 0u;

            ioctl(fbfd, FBIO_WAITFORVSYNC, 0);
            ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

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
                << "\nfinfo.smem_start\t" << finfo.smem_start
                << "\nscreen memory\t" << std::dec << screensize << " bytes"
                << "\nbuffer memory\t" << finfo.smem_len << " bytes"
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
            if (close(fbfd) == 0)
                return;
            else
                throw std::invalid_argument(strerror(errno));
        }

        void init_screen() {
            vioctl(FBIOGET_VSCREENINFO); // acquire variable info
            memcpy(&vinfo_old, &vinfo, sizeof(struct fb_var_screeninfo)); // copy for restore
            vinfo.xres = 640;
            vinfo.yres = 480;
            vinfo.xres_virtual = vinfo.xres;
            vinfo.yres_virtual = vinfo.yres * 2;
            vinfo.grayscale = 0; // ensure colour
            vinfo.bits_per_pixel = DEFAULT_BPP;
            vinfo.activate = FB_ACTIVATE_VBL;
            vioctl(FBIOPUT_VSCREENINFO); // write new vinfo
            vioctl(FBIOGET_VSCREENINFO); // re-acquire variable info
            fioctl(FBIOGET_FSCREENINFO); // acquire fixed info
            screensize = vinfo.yres * finfo.line_length;
            fbmap = static_cast<uint8_t *>(mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0));
            vbmap = fbmap + screensize;
        }

        void restore_screen() {
            if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo_old)) {
                throw std::invalid_argument(strerror(errno));
            }
            munmap(fbmap, screensize);
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

        inline void vioctl(unsigned long request) {
            if (ioctl(fbfd, request, &vinfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        inline void fioctl(unsigned long request) {
            if (ioctl(fbfd, request, &finfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        std::string device_path;
        int fbfd{-1}; //frame buffer file descriptor
        int kbfd{-1}; //keyboard file descriptor
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
