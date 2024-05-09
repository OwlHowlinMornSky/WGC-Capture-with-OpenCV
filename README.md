# WGC-Capture-with-OpenCV

WGC-Capture-with-OpenCV is [Free/Libre](https://www.gnu.org/philosophy/free-sw.en.html).

WGC-Capture-with-OpenCV 是 [自由的](https://www.gnu.org/philosophy/free-sw.html)。

## Features

* Capture a window or monitor at one time using WGC;
* Write captured frames into a cv::Mat for farther process.
* Packaged into a DLL so you don't need to care about anything of WGC or C++/WinRT.

## Requirements

* OpenCV 4.8.0

## Usage

0. Setup OpenCV 4.8.0.
1. Add `/include` into *Additional Include Directories*.
2. Add `/lib` into *Additional Library Directories*.
3. Add `wgc-capture.lib` (Release) or `wgc-capture-d.lib` (Debug) into *Additional Dependencies*.
4. Include `ohms/WGC.h` at anywhere needed.
5. Remember to copy necessary DLL files in `/bin`.
6. Examples are available.
