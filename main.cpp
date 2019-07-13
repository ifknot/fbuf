

#include "canvas_factory.h"
#include <iostream>

int main() {

    using canvas_t = linux_util::canvas_factory<linux_util::HDMI>;

    canvas_t canvas{640, 480};

    for(size_t i{0}; i < 100; ++i) {
        canvas.rgb(0xff, 0xff, 0xff);
        canvas.rect(160, 120, 320, 240);
        canvas.swap();
        usleep(100000);

        canvas.rgb(0xff, 0, 0xff);
        canvas.rect(180, 140, 280, 200);
        canvas.swap();
        usleep(100000);
    }
    canvas.clear();


    std::cout << canvas.variable_info() << std::endl;

    return 0;

}

