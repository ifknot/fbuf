

#include "frame_buffer_factory.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer_factory<linux_util::LCD>;

    //fbuf_t fb{"/dev/fb0"};
    fbuf_t fb;
/*
    for(uint32_t x = 0; x < 320; ++x) {
        fb.pixel(x, x, fb.rgb(0xFF,0,0));
    }
*/
    //fb.clear();

    fb.fill(fb.rgb(0xff,0,0));
    usleep(5000);
    fb.clear();

    fb.pixel(200,200,fb.rgb(0xff,0,0));
    std::cout << fb.variable_info() << std::endl;

    return 0;

}

