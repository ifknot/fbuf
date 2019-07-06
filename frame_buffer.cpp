#include "frame_buffer.h"

namespace linux_util {

    frame_buffer::frame_buffer(const std::string device_path): device_path(device_path) {
#ifdef NDEBUG
        open_buffer();
#endif
    }

    bool frame_buffer::open_buffer() {
        fbfd = open(device_path.c_str(), O_RDWR);
        if(fbfd != -1) {
            ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); // acquire variable info
            vinfo.grayscale = 0; // ensure colour
            vinfo.bits_per_pixel = DEFAULT_BPB;
            ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);
            ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo); // re-acquire variable info
            ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
            size_ = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;//vinfo.yres_virtual * finfo.line_length;
            fbmap = static_cast<uint8_t*>(mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0));
            return true;
        } else {
            throw std::invalid_argument(strerror(errno));
        }
    }

    bool frame_buffer::close_buffer() {
        munmap(fbmap, size_);
        if (close(fbfd) == 0)
            return true;
        else
            throw std::invalid_argument(strerror(errno));
    }

    size_t frame_buffer::size() {
        return size_;
    }

    std::pair<int, int> frame_buffer::dimensions() {
        return std::make_pair<int, int>(vinfo.xres, vinfo.yres);
    }

    frame_buffer::pixel_t frame_buffer::rgb(uint8_t r, uint8_t g, uint8_t b) {
        return (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset);
    }

    void frame_buffer::pixel(uint32_t x, uint32_t y, pixel_t colour) {
        uint32_t offset = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
        *((uint32_t *) (fbmap + offset)) = colour;
    }

    void frame_buffer::clear(pixel_t colour) {
        for (u_int32_t x = 0; x < vinfo.xres; x++) {
            for (uint32_t y = 0; y < vinfo.yres; y++) {
                uint32_t location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
                *((uint32_t *) (fbmap + location)) = rgb(0xFF, 0x00, 0x00);
            }
        }
    }

    frame_buffer::~frame_buffer() {
#ifdef NDEBUG
        close_buffer();
#endif
    }

#ifndef NDEBUG

    std::string frame_buffer::info() {
        std::stringstream ss;
        ss  << "xres\t\t" << vinfo.xres
            << "\nyres\t\t" << vinfo.yres
            << "\nxres_virtual\t" << vinfo.xres_virtual
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

#endif

}
