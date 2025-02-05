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
constexpr wchar_t TargetWindowName[] = L"崩坏：星穹铁道";
constexpr wchar_t TargetWindowName2[] = L"原神";
constexpr wchar_t TargetWindowClass[] = L"UnityWndClass";
constexpr bool    TestCaptureMonitor = false;
constexpr bool    TestFreeThreaded = false;

#include <ohms/WGC.h>
#include <opencv2/opencv.hpp>
#include <Windows.h>

int TestNormal();
int TestCallback();

int main() { // You can switch the function.
	//return TestNormal();
	return TestCallback();
}

size_t cnt = 0;

void Test(const cv::Mat& mat) {
	if constexpr (TestCaptureMonitor) {
		cv::Mat tmp;
		mat.copyTo(tmp);
		auto sz = tmp.size();
		cv::resize(
			tmp, tmp,
			sz / 2,
			0.0, 0.0, cv::InterpolationFlags::INTER_LINEAR
		);
		cv::imshow("show", tmp);
		cv::waitKey(1);
	}
	else {
		cv::imshow("show", mat);
		cv::waitKey(1);
	}
	//cv::imwrite(std::string("test\\") + std::to_string(cnt++) + ".png", mat);
}

int TestNormal() {
	// Initialization.
	ohms::wgc::ICapture::setup(false);
	auto r_capture = ohms::wgc::ICapture::getInstance();
	// Find Window.
<<<<<<< Updated upstream
	HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
=======
	HWND hwnd = FindWindowW(0, L"MAA - v5.13.0-beta.4 - 相见欢#0128");
	HWND hwnd2 = FindWindowW(0, L"Works.Debug [D:\\#COPY] – FreeFileSync 14.0");
>>>>>>> Stashed changes

	if (TestCaptureMonitor) {
		HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		if (hmonitor == NULL) {
			return 2;
		}
		if (!r_capture->startCaptureMonitor(hmonitor, TestFreeThreaded)) {
			return 3;
		}
	}
	else {
		if (hwnd == NULL) {
			return 1;
		}
		if (!IsWindow(hwnd) || IsIconic(hwnd)) { // Requirements for capture window.
			return 4;
		}
		r_capture->setClipToClientArea(true);
		if (!r_capture->startCaptureWindow(hwnd, TestFreeThreaded)) {
			return 5;
		}
<<<<<<< Updated upstream
=======
		if (!capture2->startCaptureWindow(hwnd2, TestFreeThreaded)) {
			return 6;
		}
>>>>>>> Stashed changes
	}

	// Run
	r_capture->askForRefresh(); // Ask for the first.
	MSG msg{ 0 };
	size_t testCnt = 3000; // About 10 seconds.
	while (testCnt > 0) {
		if (r_capture->isRefreshed()) {
			cv::Mat mat;
<<<<<<< Updated upstream
			if (r_capture->copyMatTo(mat, true)) {
				Test(mat);
			}
			r_capture->askForRefresh(); // Ask for next one.
=======
			capture1->copyMatTo(mat, true);
			Test(mat);
			capture1->askForRefresh(); // Ask for next one.
		}
		if (capture2->isRefreshed()) {
			cv::Mat mat;
			capture2->copyMatTo(mat, true);
			Test2(mat);
			capture2->askForRefresh(); // Ask for next one.
>>>>>>> Stashed changes
		}
		Sleep(20);
		--testCnt;
	}
	// Clear.
	ohms::wgc::ICapture::drop();
	return 0;
}

int TestCallback() {
	// Initialization.
	ohms::wgc::ICapture::setup(false);
	auto r_capture = ohms::wgc::ICapture::getInstance();
	// Find Window.
	HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);

	if (TestCaptureMonitor) {
		HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		if (hmonitor == NULL) {
			return 2;
		}
		if (!r_capture->startCaptureMonitorWithCallback(hmonitor, &Test)) {
			return 3;
		}
	}
	else {
		if (hwnd == NULL) {
			return 1;
		}
		if (!IsWindow(hwnd) || IsIconic(hwnd)) { // Requirements for capture window.
			return 4;
		}
		r_capture->setClipToClientArea(true);
		if (!r_capture->startCaptureWindowWithCallback(hwnd, &Test)) {
			return 5;
		}
	}

	int a;
	std::cin >> a;

	ohms::wgc::ICapture::drop();
	return 0;
}
