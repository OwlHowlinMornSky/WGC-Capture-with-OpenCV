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
constexpr wchar_t TargetWindowName[] = L"ExampleWindowName";
constexpr wchar_t TargetWindowName2[] = L"ExampleWindowName";
constexpr wchar_t TargetWindowClass[] = L"ExampleClassName";
constexpr bool    TestCaptureMonitor = true;
constexpr bool    TestFreeThreaded = true;

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <WGC/WGC.h>

int TestNormal();
int TestCallback();

int main() { // You can switch the function.
	return TestNormal();
	//return TestCallback();
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

void Test2(const cv::Mat& mat) {
	if constexpr (TestCaptureMonitor) {
		cv::Mat tmp;
		mat.copyTo(tmp);
		auto sz = tmp.size();
		cv::resize(
			tmp, tmp,
			sz / 2,
			0.0, 0.0, cv::InterpolationFlags::INTER_LINEAR
		);
		cv::imshow("show2", tmp);
		cv::waitKey(1);
	}
	else {
		cv::imshow("show2", mat);
		cv::waitKey(1);
	}
	//cv::imwrite(std::string("test\\") + std::to_string(cnt++) + ".png", mat);
}

int TestNormal() {
	// Initialization.
	auto factory = wgc::IFactory::createInstance(false);
	auto capture1 = factory->createCapturer().lock();
	auto capture2 = factory->createCapturer().lock();
	// Find Window.
	HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
	HWND hwnd2 = FindWindowW(TargetWindowClass, TargetWindowName2);

	if constexpr (TestCaptureMonitor) {
		HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		if (hmonitor == NULL) {
			return 2;
		}
		if (!capture1->startCaptureMonitor(hmonitor, TestFreeThreaded)) {
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
		capture1->setClipToClientArea(true);
		capture2->setClipToClientArea(true);
		if (!capture1->startCaptureWindow(hwnd, TestFreeThreaded)) {
			return 5;
		}
		if (!capture2->startCaptureWindow(hwnd2, TestFreeThreaded)) {
			return 6;
	}
	}

	// Run
	capture1->askForRefresh(); // Ask for the first.
	capture2->askForRefresh(); // Ask for the first.
	MSG msg{ 0 };
	size_t testCnt = 3000; // About 10 seconds.
	while (testCnt > 0) {
		if (capture1->isRefreshed()) {
			cv::Mat mat;
			capture1->copyMatTo(mat, true);
				Test(mat);
			capture1->askForRefresh(); // Ask for next one.
			}
		if (capture2->isRefreshed()) {
			cv::Mat mat;
			capture2->copyMatTo(mat, true);
			Test2(mat);
			capture2->askForRefresh(); // Ask for next one.
		}
		cv::waitKey(20);
		//--testCnt;
	}
	// Clear.
	//wgc::ICapture::drop();
	return 0;
}

int TestCallback() {
	// Initialization.
	auto factory = wgc::IFactory::createInstance(false);
	auto capture1 = factory->createCapturer().lock();
	auto capture2 = factory->createCapturer().lock();
	// Find Window.
	HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
	HWND hwnd2 = FindWindowW(TargetWindowClass, TargetWindowName2);

	if constexpr (TestCaptureMonitor) {
		HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		if (hmonitor == NULL) {
			return 2;
		}

		if (!capture1->startCaptureMonitorWithCallback(hmonitor, &Test)) {
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
		capture1->setClipToClientArea(true);
		capture2->setClipToClientArea(true);
		if (!capture1->startCaptureWindowWithCallback(hwnd, &Test)) {
			return 5;
		}
		if (!capture2->startCaptureWindowWithCallback(hwnd2, &Test2)) {
			return 6;
		}
	}

	int a;
	std::cin >> a;

	//wgc::ICapture::drop();
	return 0;
}
