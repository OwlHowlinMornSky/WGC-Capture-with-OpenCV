/*
*    WGC-Capture-with-OpenCV
*
*     Copyright 2023-2024  Tyler Parret True
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*
* @Authors
*    Tyler Parret True <mysteryworldgod@outlook.com><https://github.com/OwlHowlinMornSky>
*/

// You can use Spy++ to get the info about target window.
constexpr wchar_t TargetWindowName[] = L"ExampleName";
constexpr wchar_t TargetWindowClass[] = L"ExampleClassName";

#include <ohms/WGC.h>
#include <opencv2/opencv.hpp>
#include <Windows.h>

int main() {
	// Initialization.
	ohms::wgc::ICapture::setup(false);
	auto r_capture = ohms::wgc::ICapture::getInstance();
	// Find Window.
	HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
	if (hwnd == NULL) {
		return 1;
	}
	if (!IsWindow(hwnd) || IsIconic(hwnd)) { // Requirements for capture.
		return 2;
	}
	if (!r_capture->startCapture(hwnd)) {
		return 3;
	}
	r_capture->setClipToClientArea(true);
	// Run
	r_capture->askForRefresh(); // Ask for the first.
	MSG msg{ 0 };
	size_t testCnt = 3300; // About 100 seconds.
	while (testCnt > 0) {
		if (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE)) { // Neccessary for get frames.
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {
			if (r_capture->isRefreshed()) {
				cv::Mat mat;
				if (r_capture->copyMatTo(mat, false)) {
					cv::imshow("show", mat);
				}
				r_capture->askForRefresh(); // Ask for next one.
			}
			Sleep(30);
			--testCnt;
		}
	}
	// Clear.
	ohms::wgc::ICapture::drop();
	return 0;
}

