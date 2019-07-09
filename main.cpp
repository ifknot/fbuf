

#include "frame_buffer_factory.h"
#include <iostream>

int main() {

    using fbuf_t = linux_util::frame_buffer_factory<linux_util::LCD>;

    fbuf_t fb{"/dev/fb0"};
    //fbuf_t fb;

    fb.rgb(0,0xff,0);
    fb.fill();
    usleep(5000000);
    fb.clear();

    fb.pixel(200,200);

    std::cout << fb.variable_info() << std::endl;

    return 0;

}

