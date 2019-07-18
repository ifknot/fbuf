#ifndef FBUF_CANVAS_FACTORY_H
#define FBUF_CANVAS_FACTORY_H

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
     * Raspberry Pi:
     * HDMI map /dev/fb0 is double buffered
     * SPI map /dev/fb1 and no double buffering due to hardware limitations
     */
    enum screen_t {HDMI, SPI};

    /**
     * HDMI 24bit colour canvas - the factory creates a RAII double buffered memory mapping of the linux display screen buffer for simple graphics
     * True colour 16,777,216 color variations
     * @tparam HDMI
     */
    template<screen_t HDMI>
    class canvas_factory {

        const uint32_t DEFAULT_BPP = 32; //24bit colour + 8bit transparency - 16,777,216 colours

    public:

        /**
         * 32-bits, 4 bytes per pixel
         */
        using pixel_t = uint32_t;

        /**
         * Build a True Colour, double buffered, graphics canvas
         * @param width         desired width as pixels
         * @param height        desired height as pixels
         * @param device_path   defaults to "/dev/fb0"
         */
        canvas_factory(size_t width, size_t height, const std::string device_path = "/dev/fb0"): device_path(device_path) {
            open_buffer();
            init_screen(width, height);
        }

        /**
         * Set the active colour used by the graphics primitives
         * @param r     8-bit red value
         * @param g     8-bit green value
         * @param b     8-bit blue value
         * @param a     8-bit transparent value
         */
        void rgb(pixel_t r, pixel_t g, pixel_t b, pixel_t a = 0xFF) {
            colour = (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset) | (a << vinfo.transp.offset);
        }

        /**
         * Clear the current active screen to black
         * @note  changes not visible until ```swap()```
         */
        void clear() {
            memset(screen1, 0u, screensize);
        }

        /**
         * Draw a rectangular block of colour
         * @note  changes not visible until ```swap()```
         * @param x
         * @param y
         * @param width
         * @param height
         */
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
                    ((uint64_t *) (screen1))[offset + j] = c;
                }
                offset += linesize;
            }
        }

        /**
         * Plot a single pixel current colour
         * @note  changes not visible until ```swap()```
         * @param x
         * @param y
         */
        inline void pixel(uint x, uint y) {
            ((pixel_t *) (screen1))[(y * vinfo.xres) + x] = colour;
        }

        /**
         * Fast horizontal line in current colour
         * @note  changes not visible until ```swap()```
         * @param x
         * @param y
         * @param width
         */
        void hline(size_t x, size_t y, size_t width)  {
            y *= vinfo.xres;
            y += x;
            for(size_t i{0}; i < width; ++i) {
                ((pixel_t *) (screen1))[y + i] = colour;
            }
        }

        /**
         * Fast vertical line in current colour
         * @note  changes not visible until ```swap()```
         * @param x
         * @param y
         * @param height
         */
        void vline(size_t x, size_t y, size_t height)  {
            y *= vinfo.xres;
            y += x;
            for(size_t i{0}; i < height; ++i) {
                ((pixel_t *) (screen1))[y] = colour;
                y += vinfo.xres;
            }
        }

        /**
         * Draw a rectangle lines in current colour
         * @note  changes not visible until ```swap()```
         * @param x
         * @param y
         * @param width
         * @param height
         */
        void rect(size_t x, size_t y, size_t width, size_t height)  {
            hline(x, y, width);
            vline(x, y, height);
            hline(x, y + height, width);
            vline(x + width, y, height);
        }

        /**
         * TODO Bresh. line algo
         * @note  changes not visible until ```swap()```
         * @param x1
         * @param y1
         * @param x2
         * @param y2
         */
        void line(uint x1, uint y1, uint x2, uint y2) {

        }

        /**
         * Swap the off-screen active working buffer into the visible inactive state
         */
        void swap() {
            vinfo.yoffset = (vinfo.yoffset == screen1_yoffset) ?screen2_yoffset :screen1_yoffset;
            ioctl(fbfd, FBIO_WAITFORVSYNC, 0);
            ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
            std::swap(screen1, screen2);
        }

        /**
         * Safely RAII release memory resources
         */
        ~canvas_factory()  {
            restore_screen();
            close_buffer();
        }

        /**
         * Get framebuffer variable screen info
         * @note Omits obselete, timing  & reserved info
         * @return string - tabulated info
         */
        std::string variable_info()  {
            vioctl(FBIOGET_VSCREENINFO); // acquire variable info
            std::stringstream ss;
            ss  << "\nxres\t\t" << vinfo.xres           // visible resolution
                << "\nyres\t\t" << vinfo.yres
                << "\nxres_virtual\t" << vinfo.xres_virtual // virtual resolution
                << "\nyres_virtual\t" << vinfo.yres_virtual
                << "\nxoffset\t\t" << vinfo.xoffset     // offsets from virtual to visible resolution
                << "\nyoffset\t\t" << vinfo.yoffset
                << "\nbits_per_pixel\t" << vinfo.bits_per_pixel
                << "\nr shift\t\t" << vinfo.red.offset
                << "\ng shift\t\t" << vinfo.green.offset
                << "\nb shift\t\t" << vinfo.blue.offset
                << "\nt shift\t\t" << vinfo.transp.offset
                << "\ngrayscale\t" << vinfo.grayscale   // 0 = color, 1 = grayscale, >1 = FOURCC
                << "\nnonstd\t\t" << vinfo.nonstd       // != 0 non standard pixel format
                << "\nactivate\t" << vinfo.activate     // e.g FB_ACTIVATE_NOW set values immediately (or vbl), FB_ACTIVATE_FORCE force apply even when no change
                << "\nheight\t\t" << vinfo.height << "mm"   // height of picture in mm
                << "\nwidth\t\t" << vinfo.width << "mm\n";  // width of picture in mm
            return ss.str();
        }

        /**
         * Get framebuffer fixed screen info
         * @return string - tabulated info
         */
        std::string fixed_info() {
            fioctl(FBIOGET_FSCREENINFO);
            std::stringstream ss;
            ss  << "\nid\t\t" << std::string(finfo.id); // identification string
            ss  << "\nmem start\t" << std::hex << finfo.smem_start  // Start of frame buffer mem (physical address)
                << "\nmem length\t" << std::dec << finfo.smem_len << " bytes"   // Length of frame buffer mem
                << "\ntype\t\t" << finfo.type
                << "\naux\t\t"<< finfo.type_aux         // Interleave for interleaved Planes
                << "\nvisual\t\t"<< finfo.visual
                << "\nxpanstep\t"<< finfo.xpanstep      // zero if no hardware panning
                << "\nypanstep\t"<< finfo.ypanstep      // zero if no hardware panning
                << "\nywrapstep\t"<< finfo.ywrapstep    // zero if no hardware ywrap
                << "\nline length\t"<< finfo.line_length    // length of a line in bytes
                << "\nmap IO addr\t"<< std::hex << finfo.mmio_start // Start of Memory Mapped I/O (physical address)
                << "\nmap length\t"<< std::dec << finfo. mmio_len << " bytes"   // Length of Memory Mapped I/O
                << "\naccel\t\t"<< finfo.accel		    // Indicate to driver which	specific chip/card we have
                << "\ncapabilities\t"<< finfo.capabilities << "\n";
            return ss.str();
        }

    private:

        /**
         * Acquire the frame buffer file descriptor in read and write mode
         * @throws std::invalid_argument(strerror(errno))
         */
        void open_buffer() {
            fbfd = open(device_path.c_str(), O_RDWR);
            if(fbfd != -1) {
                return;
            } else {
                throw std::invalid_argument(strerror(errno));
            }
        }

        /**
         * Release the file descriptor
         * @throws std::invalid_argument(strerror(errno))
         */
        void close_buffer()  {
            if (close(fbfd) == 0)
                return;
            else
                throw std::invalid_argument(strerror(errno));
        }

        /**
         * Change the screen resolution, bits per pixel, and set up 2 x virtual screens for the canvas
         * @note canvas uses 2 new buffers and leaves the original screen buffer (screen0) unmolested
         * @param width - visible screen dimensions
         * @param height
         */
        void init_screen(size_t width, size_t height) {
            vioctl(FBIOGET_VSCREENINFO); // acquire variable info
            memcpy(&vinfo_old, &vinfo, sizeof(struct fb_var_screeninfo)); // copy for restore
            vinfo.xres = width;
            vinfo.yres = height;
            screen1_yoffset = vinfo.yres;
            screen2_yoffset = screen1_yoffset + vinfo.yres;
            vinfo.xres_virtual = vinfo.xres;
            vinfo.yres_virtual = vinfo.yres * 3; //make space for 2 virtual screens
            vinfo.yoffset = screen2_yoffset; //so the first swap works correctly
            vinfo.grayscale = 0; // ensure colour
            vinfo.bits_per_pixel = DEFAULT_BPP;
            vinfo.activate = FB_ACTIVATE_VBL;
            vioctl(FBIOPUT_VSCREENINFO); // write new vinfo
            vioctl(FBIOGET_VSCREENINFO); // re-acquire variable info
            fioctl(FBIOGET_FSCREENINFO); // acquire fixed info
            screensize = vinfo.yres * finfo.line_length; // size of visible area
            //memory map entire frame buffer of 3 x "screens"
            screen0 = static_cast<uint8_t *>(mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0));
            screen1 = screen0 + screensize; // offset each of the virtual screens
            screen2 = screen1 + screensize;
        }

        /**
         * Restore the original screen parameters from ```vinfo_old```
         * @throws std::invalid_argument(strerror(errno))
         */
        void restore_screen() {
            if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo_old)) {
                throw std::invalid_argument(strerror(errno));
            }
            munmap(screen0, finfo.smem_len);
        }

        /**
         * Variable info helper
         * @throws std::invalid_argument(strerror(errno))
         * @param request
         */
        inline void vioctl(unsigned long request) {
            if (ioctl(fbfd, request, &vinfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        /**
         * Fixed info helper
         * @throws std::invalid_argument(strerror(errno))
         * @param request
         */
        inline void fioctl(unsigned long request) {
            if (ioctl(fbfd, request, &finfo)) {
                throw std::invalid_argument(strerror(errno));
            }
        }

        std::string device_path; //TODO remove this and pass to init
        int fbfd{-1}; //frame buffer file descriptor
        //TODO remove screensize and local to init
        uint32_t screensize; //visible screen size bytes
        //original screen & 2 virtual screen maps
        uint8_t* screen0{0};
        uint8_t* screen1{0};
        uint8_t* screen2{0};
        //offsets into the virtual screen memory maps
        uint32_t screen1_yoffset;
        uint32_t screen2_yoffset;
        //colour used by graphic primitives
        pixel_t colour;

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

#endif //FBUF_CANVAS_FACTORY_H
