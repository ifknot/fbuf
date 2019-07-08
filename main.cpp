
#define CATCH_CONFIG_RUNNER
#include "tests/catch2.h"

#include "frame_buffer.h"

int main( int argc, char* argv[] ) {

    using fbuf_t = linux_util::frame_buffer;

    fbuf_t fb;
    //fb.swap();

    for(uint32_t x = 0; x < fb.vinfo.yres; ++x) {
        fb.fill(fb.rgb(0xFF, 0xFF, 0xFF));
        fb.pixel(x, x, fb.rgb(0xFF,0,0));
        usleep(5000);

    }
    fb.clear();
    //fb.swap();
    std::cout << fb.info() << std::endl;

    // https://github.com/ifknot/Catch2
    //return Catch::Session().run( argc, argv );

}

