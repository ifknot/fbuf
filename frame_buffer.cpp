#include "frame_buffer.h"

namespace linux_util {

    frame_buffer::frame_buffer(const std::string& device_path = FRAME_BUFFER_PATH): device_path(device_path) {}

    int frame_buffer::open_buffer() {
        fbfd = open(device_path.c_str(), O_RDWR);
        return (fbfd != -1) ? fbfd : throw std::invalid_argument(strerror(errno));
    }




}
