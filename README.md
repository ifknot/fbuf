# fbuf
![rptx logo](https://cldup.com/W9fXBWKrXq.png)
### C++17 Linux low level framebuffer graphics
### Double Buffering Fail :(
Unfortunately the LCD display does not allow memory access to the memory mapped addresses beyond the screeensize.

Memory mapping a double the screen size as a virtual screen:
```cpp
fbmap = static_cast<uint8_t *>(mmap(0, screensize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0));
```
And then swapping the pointers to memory map:
```cpp
void swap() {
    vinfo.yoffset = (vinfo.yoffset == 0u) ?screensize :0u;
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
    std::swap(fbmap, vbmap);
}
```
Results in either Segmentation Fault (SIGSEGV) or Bus Error (SIGBUS) when writing or reading the virtual screen buffer:
```cpp
void pixel(uint32_t x, uint32_t y, pixel_t colour) {
        *((pixel_t*) (vbmap + ((x + vinfo.xoffset) << 1) + (y + vinfo.yoffset) * finfo.line_length)) = colour;
}
```
:(
```
Segmentation fault
```


