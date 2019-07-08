

#include "frame_buffer.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer;

    fbuf_t fb;

/*
    for(uint32_t x = 0; x < 320; ++x) {
        fb.pixel(x, x, fb.rgb(0xFF,0,0));
    }
*/
    //fb.clear();

    fb.fill(fb.rgb(0xff,0,0));
    fb.swap();
    //fb.pixel(200,200,fb.rgb(0xff,0,0));
    std::cout << fb.info() << std::endl;

    return 0;

}

