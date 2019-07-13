

#include "frame_buffer_factory.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer_factory<linux_util::HDMI>;

    fbuf_t fb;
    //fb.clear();

    fb.rgb(0,0xff,0);
    fb.fill(0,0,640,480);
    usleep(500000);
    fb.rgb(0xff,0,0);
    fb.fill(160,120,320,240);
    usleep(500000);
    fb.clear();
    fb.rgb(0xff,0xff,0xff);
    fb.pixel(320,240);
    fb.hline(160,120,320);
    fb.vline(160,120,240);
    usleep(5000000);

    std::cout << fb.variable_info() << std::endl;

    return 0;

}

