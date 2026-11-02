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

// You can use Spy++ to get the info about the target window.
constexpr wchar_t TargetWindowName[] = L"ExampleWindowName";
constexpr wchar_t TargetWindowName2[] = L"ExampleWindowName";
constexpr wchar_t TargetWindowClass[] = L"ExampleClassName";
constexpr bool    TestCaptureMonitor = true;
constexpr bool    TestFreeThreaded = true;

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <WGC/WGC.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>

constexpr DWORD TestDurationMs = 10000;

namespace {

struct FrameMailbox {
	std::mutex mutex;
	cv::Mat frame;
	std::atomic<bool> hasFrame{ false };

	void push(const cv::Mat& mat) {
		std::lock_guard lock(mutex);
		frame = mat; // Mat owns its pixels; this shares the refcount.
		hasFrame.store(true);
	}

	bool take(cv::Mat& out) {
		std::lock_guard lock(mutex);
		if (!hasFrame.exchange(false))
			return false;
		out = frame;
		frame.release();
		return true;
	}
};

void ShowFrame(const cv::Mat& frame, const char* window, bool halfSize) {
	if (frame.empty())
		return;

	if (halfSize) {
		const cv::Size size = frame.size() / 2;
		if (size.width <= 0 || size.height <= 0)
			return;
		cv::Mat resized;
		cv::resize(frame, resized, size, 0.0, 0.0, cv::InterpolationFlags::INTER_LINEAR);
		cv::imshow(window, resized);
	}
	else {
		cv::imshow(window, frame);
	}
}

std::shared_ptr<wgc::ICapturer> MakeCapturer(wgc::IFactory& factory) {
	try {
		return factory.createCapturer().lock();
	}
	catch (...) {
		return nullptr;
	}
}

} // namespace

int TestNormal();
int TestCallback();

int main() { // You can switch the function.
	return TestNormal();
	//return TestCallback();
}

int TestNormal() {
	auto factory = wgc::IFactory::createInstanceNoThrow(false);
	if (!factory) {
		std::cerr << "Failed to create the capture factory." << std::endl;
		return 1;
	}

	auto capture1 = MakeCapturer(*factory);
	if (!capture1) {
		std::cerr << "Failed to create the first capturer." << std::endl;
		return 2;
	}

	std::shared_ptr<wgc::ICapturer> capture2;

	if constexpr (TestCaptureMonitor) {
		const HMONITOR monitor = MonitorFromPoint(
			POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY
		);
		if (monitor == nullptr) {
			std::cerr << "Failed to find the primary monitor." << std::endl;
			return 3;
		}
		if (!capture1->startCaptureMonitor(monitor, TestFreeThreaded)) {
			std::cerr << "Failed to start monitor capture." << std::endl;
			return 4;
		}
	}
	else {
		const HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
		const HWND hwnd2 = FindWindowW(TargetWindowClass, TargetWindowName2);
		if (hwnd == nullptr || hwnd2 == nullptr) {
			std::cerr << "Target window not found." << std::endl;
			return 5;
		}

		capture2 = MakeCapturer(*factory);
		if (!capture2) {
			std::cerr << "Failed to create the second capturer." << std::endl;
			return 6;
		}

		capture1->setClipToClientArea(true);
		capture2->setClipToClientArea(true);
		if (!capture1->startCaptureWindow(hwnd, TestFreeThreaded) ||
			!capture2->startCaptureWindow(hwnd2, TestFreeThreaded)) {
			std::cerr << "Failed to start window capture." << std::endl;
			capture1->stopCapture();
			capture2->stopCapture();
			return 7;
		}
	}

	const ULONGLONG deadline = GetTickCount64() + TestDurationMs;
	while (GetTickCount64() < deadline) {
		if (capture1->isRefreshed()) {
			cv::Mat frame;
			capture1->copyMatTo(frame, true);
			capture1->askForRefresh();
			ShowFrame(frame, "show", TestCaptureMonitor);
		}

		if (capture2 && capture2->isRefreshed()) {
			cv::Mat frame;
			capture2->copyMatTo(frame, true);
			capture2->askForRefresh();
			ShowFrame(frame, "show2", false);
		}

		if (cv::waitKey(20) == 27)
			break;
	}

	if (capture2)
		capture2->stopCapture();
	capture1->stopCapture();
	cv::destroyAllWindows();
	return 0;
}

int TestCallback() {
	auto factory = wgc::IFactory::createInstanceNoThrow(false);
	if (!factory) {
		std::cerr << "Failed to create the capture factory." << std::endl;
		return 1;
	}

	auto capture1 = MakeCapturer(*factory);
	if (!capture1) {
		std::cerr << "Failed to create the first capturer." << std::endl;
		return 2;
	}

	std::shared_ptr<wgc::ICapturer> capture2;
	FrameMailbox mailbox1;
	std::unique_ptr<FrameMailbox> mailbox2;

	if constexpr (TestCaptureMonitor) {
		const HMONITOR monitor = MonitorFromPoint(
			POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY
		);
		if (monitor == nullptr) {
			std::cerr << "Failed to find the primary monitor." << std::endl;
			return 3;
		}

		if (!capture1->startCaptureMonitorWithCallback(
			monitor,
			[&mailbox1](const cv::Mat& mat) { mailbox1.push(mat); }
		)) {
			std::cerr << "Failed to start callback monitor capture." << std::endl;
			return 4;
		}
	}
	else {
		const HWND hwnd = FindWindowW(TargetWindowClass, TargetWindowName);
		const HWND hwnd2 = FindWindowW(TargetWindowClass, TargetWindowName2);
		if (hwnd == nullptr || hwnd2 == nullptr) {
			std::cerr << "Target window not found." << std::endl;
			return 5;
		}

		capture2 = MakeCapturer(*factory);
		mailbox2 = std::make_unique<FrameMailbox>();
		if (!capture2) {
			std::cerr << "Failed to create the second capturer." << std::endl;
			return 6;
		}

		capture1->setClipToClientArea(true);
		capture2->setClipToClientArea(true);
		if (!capture1->startCaptureWindowWithCallback(
				hwnd,
				[&mailbox1](const cv::Mat& mat) { mailbox1.push(mat); }
			) ||
			!capture2->startCaptureWindowWithCallback(
				hwnd2,
				[&mailbox2](const cv::Mat& mat) {
					if (mailbox2)
						mailbox2->push(mat);
				}
			)) {
			std::cerr << "Failed to start callback window capture." << std::endl;
			capture1->stopCapture();
			if (capture2)
				capture2->stopCapture();
			return 7;
		}
	}

	const ULONGLONG deadline = GetTickCount64() + TestDurationMs;
	while (GetTickCount64() < deadline) {
		cv::Mat frame;
		if (mailbox1.take(frame))
			ShowFrame(frame, "show", TestCaptureMonitor);

		if (mailbox2 && mailbox2->take(frame))
			ShowFrame(frame, "show2", false);

		if (cv::waitKey(20) == 27)
			break;
	}

	// The callbacks run on frame threads; stop them before the mailboxes die.
	if (capture2)
		capture2->stopCapture();
	capture1->stopCapture();
	cv::destroyAllWindows();
	return 0;
}
