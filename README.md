# CRWCR
A Continuous Random Walker Model with Explicit Coherence Regularization for Image Segmentation 

---

This is a tiny software which implements the CRWCR algorithm (will be accepted by IEEE Transactions on Image Processing).

---
## Performance
With 10 1D initializations and 10 2D PR iterations (enough for most applications), the running time on my laptop ( with Intel i7-6700HQ CPU and GTX965M GPU) are shown in the table below.

| Image Size | 481X321 | 680X669 | 1024X1024 | 1443X963 | 1924X1284 |
| ---------- | ------- | ------- | --------- | -------- | --------- |
| Time (ms)  | 32      | 64      | 123       | 190      | 327       |



---

## Building requirement

+ The project can be compiled with Visual Studio (>= 2017.15.8).
+ The building through CMake requires a version number >= 3.8.
+ The primary GUI is based on Qt (Supported >= 5.10).
+ GPU is supported, please check the USE_CUDA option if you have a GPU device. The GPU version is based on CUDA (Supported >= 9.0). 
+ The image rendering is based on OpenGL (>= 4.0).

## Other

The code has been tested under Windows 10 with Visual Studio 2017. It might also work under Linux, but have not been tested. 
Any feedback regarding compilation with Linux is appreciated. 
