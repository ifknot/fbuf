
#include "catch2.h"

#include "../frame_buffer.h"

#include <iostream>

SCENARIO("instantiating a frame_buffer, open and close succeeds", "[fbuf]") {

    using fbuf_t = linux_util::frame_buffer;

    GIVEN("a frame_buffer") {

        fbuf_t fb;

#ifndef NDEBUG

        WHEN("it is opened")
            THEN("valid file descriptor returned") REQUIRE(fb.open_buffer());

        AND_GIVEN("open frame buffer") {
            fb.open_buffer();
            WHEN("buffer size query")
                THEN("valid size returned") {
                    REQUIRE(fb.size() != 0);

                    std::cout << fb.info() << std::endl;
            }

            WHEN("it is closed")
                THEN("success is returned") REQUIRE(fb.close_buffer());
        }

#endif

        fb.clear();

        //std::cout << fb.info() << std::endl;

    }

#ifndef NDEBUG

    GIVEN("a frame_buffer invalid device path") {

        fbuf_t fb{"/dev/"};

        WHEN("it is opened")
            THEN("an invalid_argument exception is thrown") CHECK_THROWS_AS(fb.open_buffer(), std::invalid_argument);


        WHEN("it is closed")
            THEN("an invalid_argument exception is thrown") CHECK_THROWS_AS(fb.open_buffer(), std::invalid_argument);

    }
#endif

}

