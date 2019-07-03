#include "frame_buffer.h"

namespace linux_util {

    frame_buffer::frame_buffer(const std::string device_path): device_path(device_path) {}

    bool frame_buffer::open_buffer() {
        fbfd = open(device_path.c_str(), O_RDWR);
        if(fbfd != -1) {
            ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
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
        if(close(fbfd) == 0)
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

    size_t frame_buffer::bits_per_pixel() {
        return vinfo.bits_per_pixel;
    }

    uint16_t frame_buffer::rgb(uint8_t r, uint8_t g, uint8_t b) {
        return (r << vinfo.red.offset) | (g << vinfo.green.offset) | (b << vinfo.blue.offset);
    }

    void frame_buffer::clear(uint32_t colour) {
        for (int x = 0; x < vinfo.xres; x++) {
            for (int y = 0; y < vinfo.yres; y++) {
                long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
                *((uint16_t *) (fbmap + location)) = rgb(0xFF, 0x00, 0xFF);
            }
        }
    }

}
