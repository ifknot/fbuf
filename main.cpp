

#include "frame_buffer_factory.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer_factory<linux_util::HDMI>;

    fbuf_t fb;

    for(size_t i{0}; i < 100; ++i) {
        fb.rgb(0xff, 0xff, 0xff);
        fb.rect(160, 120, 320, 240);
        fb.swap();
        usleep(100000);

        fb.rgb(0xff, 0, 0xff);
        fb.rect(180, 140, 280, 200);
        fb.swap();
        usleep(100000);
    }
    fb.clear();


    std::cout << fb.variable_info() << std::endl;

    return 0;

}

