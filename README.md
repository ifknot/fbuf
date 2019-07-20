# fbuf
![rptx logo](https://cldup.com/W9fXBWKrXq.png)
### C++17 RPi Linux low level framebuffer graphics
**N.B.** command line only
It seems [link](https://stackoverflow.com/questions/13907471/linux-framebuffer-graphics-and-vsync) that the ability to *fully* manipulate the frame buffer is anachronistic and limited to Raspbian on the RPi and *possibly* other command line only linux deployments.
### Direct Rendering Manager (DRM) 
The general purpose solution seems to be to use ```libdrm```(/dev/dri/card0) to draw low level graphics to the screen. 

[e.g. 1.](https://github.com/dvdhrm/docs/blob/master/drm-howto/modeset-vsync.c)
[e.g. 2.](http://betteros.org/tut/graphics1.php#fbdev)


