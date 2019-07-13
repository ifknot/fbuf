

#include "frame_buffer_factory.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer_factory<linux_util::HDMI>;

    fbuf_t fb;
    //fb.clear();

    fb.rgb(0,0xff,0);
    //fb.fill();
    //usleep(5000000);
    //fb.rgb(0,0,0);
    //fb.fill();
    fb.clear();

    //fb.pixel(320,240);

    std::cout << fb.variable_info() << std::endl;

    return 0;

}

