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

    uint32_t frame_buffer::size() {
        return size_;
    }

    frame_buffer::pixel_t frame_buffer::rgb(uint8_t r, uint8_t g, uint8_t b) {
        return (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset);
    }

    void frame_buffer::pixel(uint32_t x, uint32_t y, pixel_t colour) {
        *((uint32_t *) (fbmap + ((x + vinfo.xoffset) << 1) + (y + vinfo.yoffset) * finfo.line_length)) = colour;
    }

    void frame_buffer::clear() {
        for(uint32_t i{0}; i < size_ ; ++i) {
            *((uint32_t *) (fbmap + i + vinfo.xoffset)) = 0u;
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
            << "\nmemory\t\t" << size_ << " bytes"
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
