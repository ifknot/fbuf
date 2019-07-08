

#include "frame_buffer.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer;

    fbuf_t fb;
    //fb.swap();

    for(uint32_t x = 0; x < 320; ++x) {
        fb.fill(fb.rgb(0xFF, 0xFF, 0xFF));
        fb.pixel(x, x, fb.rgb(0xFF,0,0));
        usleep(5000);

    }

    fb.clear();
    //fb.swap();
    std::cout << fb.info() << std::endl;

    return 0;

}

