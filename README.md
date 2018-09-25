# CRWCR
A Continuous Random Walker Model with Explicit Coherence Regularization for Image Segmentation 

---

This is a tiny software which implement the CRWCR algorithm (will be accepted by IEEE Transactions on Image Processing) use CUDA.

---
## Performance
On my laptop ( with Intel i7-6700HQ CPU and GTX965M GPU),  I tested different sizes of images with 10 1D initializations  and 10  2D PR iterations. Cost time as shown in the table below.

| Image Size | 481X321 | 680X669 | 1024X1024 | 1443X963 | 1924X1284 |
| ---------- | ------- | ------- | --------- | -------- | --------- |
| Time (ms)  | 32      | 64      | 123       | 190      | 327       |



---

## How to build

+ The project can be compiled in Visual Studio (>= 2017.15.8).
+ The project through CMake (>= 3.8).
+ The primary GUI is based on Qt (Supported >= 5.10).
+ The CRWCR algorithm GPU version based on CUDA (Supported >= 9.0). If your GPU is available, please check the USE_CUDA option.
+ The image rendering based on OpenGL (>= 4.0).

## Other

Code has been tested under Windows 10 with Visual Studio 2017. Maybe the program can work under Linux, it's welcome if anyone could test it.
