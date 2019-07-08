#include "frame_buffer.h"

namespace linux_util {

    frame_buffer::frame_buffer(const std::string device_path): device_path(device_path) {
        //open_buffer();
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
            size_ = vinfo.yres_virtual * finfo.line_length;
            fbmap = static_cast<uint8_t*>(mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0));
            vbmap = fbmap;// + size_;
            printf("\nframe\t\t%p\nvirtual\t\t%p", fbmap, vbmap);
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
        *((pixel_t*) (vbmap + ((x + vinfo.xoffset) << 1) + (y + vinfo.yoffset) * finfo.line_length)) = colour;
    }

    void frame_buffer::clear() {
        for (size_t i{0}; i < (vinfo.yres_virtual * finfo.line_length) / 2; ++i) {
            ((pixel_t*)(vbmap))[i] = 0u;
        }
    }

    void frame_buffer::fill(frame_buffer::pixel_t colour) {
        for (size_t i{0}; i < (vinfo.yres_virtual * finfo.line_length) / 2; ++i) {
            ((pixel_t*)(vbmap))[i] = colour;
        }
    }

    void frame_buffer::swap() {
        vinfo.yoffset = (vinfo.yoffset == 0u) ?size_ :0u;
        ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);
        ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
        std::swap(fbmap, vbmap);
    }

    frame_buffer::~frame_buffer() {
        //close_buffer();
    }

    std::string frame_buffer::info() {
        std::stringstream ss;
        ss  << "\nxres\t\t" << vinfo.xres
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

}
