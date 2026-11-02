# WGC-Capture-with-OpenCV

WGC-Capture-with-OpenCV is [Free/Libre](https://www.gnu.org/philosophy/free-sw.en.html).

WGC-Capture-with-OpenCV 是 [自由的](https://www.gnu.org/philosophy/free-sw.html)。

## Features

* Capture windows or monitors using WGC;
* Write captured frames into a cv::Mat for farther process.
* Packaged into a DLL so you don't need to care about anything of WGC or C++/WinRT.
* `ExampleWin32` demonstrates window and multi-monitor screen capture,
  decimation/scale preview, and saving 3-channel BGR or 4-channel BGRA PNG files.

## Requirements

* Windows 10 1903 or later (Windows 11 recommended; verified with VS2026/v145).
* An interactive desktop with DWM. The host process must run at Medium
  integrity or higher; Low Integrity/AppContainer/sandboxed processes are
  rejected by WGC with `E_ACCESSDENIED`. Administrator privileges are not
  required for normal Medium-integrity use.
* Visual Studio 2026 with the v145 toolset to build the DLL.
* OpenCV 4.10.0.
* C++20 for the DLL; the public header itself is C++17-compatible.
* The host and the DLL must use the same compiler/STL, the same Debug/Release
  configuration, the same OpenCV build and the same `/MD` or `/MDd` runtime.

## Building

The project reads OpenCV from `$(OHMS_LIB_DIR)\opencv\4.10.0\build`.
Define `OHMS_LIB_DIR` (for example `C:\Users\me\dev_libs`) before building.
`WGC-Capture-with-OpenCV.sln` builds the library and both examples.

## Tests

`Tests/WGCSmoke.vcxproj` (Debug/Release x64) checks factory/capturer lifetime,
empty-frame behavior, stop/restart, pull mode, callback mode and window capture.
It requires an interactive desktop with DWM and screen-capture permission.
When WGC is unavailable the test prints `SKIP` and returns 77; on a normal
desktop it returns 0 only when every capture check passes.

Run it from the repository root with the OpenCV and `x64\<Configuration>` DLL
directories on `PATH`. It prints the process integrity level, session id, remote
session flag and the library's last HRESULT, so an `E_ACCESSDENIED` can be
distinguished from a capture/parameter bug.

When a `startCapture*` call returns false, call `wgc::getLastError()` and
`wgc::getLastErrorMessage()` (both thread-local). ExampleBasic and ExampleWin32
now display these values.

## Usage

0. Build or obtain `wgc-capture.dll` (Release) or `wgc-capture-d.dll` (Debug).
1. Add `WGC-Capture-with-OpenCV/include` to *Additional Include Directories*.
2. Add the directory containing `wgc-capture.lib` / `wgc-capture-d.lib`
   (normally `x64\<Configuration>`) to *Additional Library Directories*.
3. Add `wgc-capture.lib` (Release) or `wgc-capture-d.lib` (Debug) to
   *Additional Dependencies*.
4. Include `WGC/WGC.h` where needed.
5. Copy `wgc-capture*.dll` and the matching OpenCV DLLs next to the executable.
6. Examples are available; build them through the solution so that
   `$(SolutionDir)` is defined.
