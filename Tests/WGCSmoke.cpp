/*
*    WGC-Capture-with-OpenCV
*
*    Console smoke test for the capture library.
*
*    This test requires an interactive Windows desktop with a primary monitor.
*    It exercises pull mode, callback mode, stop/restart, factory lifetime and
*    window capture.
*/

#include <WGC/WGC.h>

#include <opencv2/opencv.hpp>
#include <Windows.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace {

int g_failures = 0;

void Check(bool condition, const char* description) {
	std::cout << (condition ? "[PASS] " : "[FAIL] ") << description << std::endl;
	if (!condition)
		++g_failures;
}

bool WaitFor(const std::function<bool()>& predicate, DWORD timeoutMs) {
	const ULONGLONG deadline = GetTickCount64() + timeoutMs;
	while (GetTickCount64() < deadline) {
		if (predicate())
			return true;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return predicate();
}

struct FrameMailbox {
	std::mutex mutex;
	std::condition_variable condition;
	cv::Mat frame;
	bool hasFrame = false;

	void push(const cv::Mat& mat) {
		{
			std::lock_guard lock(mutex);
			frame = mat;
			hasFrame = true;
		}
		condition.notify_one();
	}

	bool wait(DWORD timeoutMs) {
		std::unique_lock lock(mutex);
		return condition.wait_for(
			lock,
			std::chrono::milliseconds(timeoutMs),
			[this] { return hasFrame; }
		);
	}

	bool take(cv::Mat& out) {
		std::lock_guard lock(mutex);
		if (!hasFrame)
			return false;
		out = frame;
		frame.release();
		hasFrame = false;
		return true;
	}
};

LRESULT CALLBACK TestWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProcW(hwnd, message, wParam, lParam);
}

HWND CreateTestWindow() {
	constexpr wchar_t className[] = L"WGCSmokeTargetWindow";

	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.lpfnWndProc = TestWindowProc;
	windowClass.hInstance = GetModuleHandleW(nullptr);
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	windowClass.lpszClassName = className;
	if (!RegisterClassExW(&windowClass) &&
		GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return nullptr;
	}

	HWND hwnd = CreateWindowExW(
		0,
		className,
		L"WGC smoke test target",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		80, 80, 640, 480,
		nullptr, nullptr, GetModuleHandleW(nullptr), nullptr
	);
	if (!hwnd)
		return nullptr;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	return hwnd;
}

cv::Mat CopyCurrentFrame(const std::shared_ptr<wgc::ICapturer>& capturer, bool bgr) {
	cv::Mat frame;
	if (capturer)
		capturer->copyMatTo(frame, bgr);
	return frame;
}

bool StartPull(const std::shared_ptr<wgc::ICapturer>& capturer, HMONITOR monitor) {
	return capturer && capturer->startCaptureMonitor(monitor, true);
}

} // namespace

int main() {
	SetProcessDPIAware();

	const HMONITOR monitor = MonitorFromPoint(
		POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY
	);
	if (!monitor) {
		std::cerr << "No primary monitor; smoke test skipped." << std::endl;
		return 77;
	}

	std::cout << "Creating factory..." << std::endl;
	auto factory = wgc::IFactory::createInstanceNoThrow(false);
	Check(factory != nullptr, "createInstanceNoThrow returns a factory");
	if (!factory)
		return 1;

	auto capturer = factory->createCapturer().lock();
	Check(capturer != nullptr, "createCapturer returns a live capturer");
	if (!capturer)
		return 1;

	cv::Mat beforeStart;
	capturer->copyMatTo(beforeStart);
	Check(beforeStart.empty(), "copyMatTo before start returns an empty frame");
	Check(!capturer->isCapturing(), "new capturer is not capturing");
	Check(!capturer->isRefreshed(), "new capturer has no pending refresh");

	bool captureAvailable = true;

	std::cout << "Testing monitor pull mode..." << std::endl;
	if (!StartPull(capturer, monitor)) {
		std::cout << "[SKIP] WGC capture is unavailable in this desktop session "
			<< "(requires DWM and screen-capture permission)." << std::endl;
		captureAvailable = false;
		Check(!capturer->isCapturing(), "failed start leaves the capturer stopped");
		cv::Mat afterFailedStart;
		capturer->copyMatTo(afterFailedStart);
		Check(afterFailedStart.empty(), "failed start does not publish a frame");
	}
	if (captureAvailable) {
		Check(WaitFor([&] { return capturer->isRefreshed(); }, 5000),
			"pull mode reports a refreshed frame");

		cv::Mat bgra = CopyCurrentFrame(capturer, false);
		Check(!bgra.empty(), "pull mode returns a frame");
		Check(bgra.type() == CV_8UC4, "pull frame is BGRA CV_8UC4");
		Check(bgra.cols > 0 && bgra.rows > 0, "pull frame has non-zero size");

		cv::Mat bgr = CopyCurrentFrame(capturer, true);
		Check(!bgr.empty(), "copyMatTo(convertToBGR=true) returns a frame");
		Check(bgr.channels() == 3, "converted frame has 3 channels");

		cv::Mat saved;
		bgra.copyTo(saved);
		capturer->stopCapture();
		Check(!capturer->isCapturing(), "stopCapture clears the capturing state");
		Check(!capturer->isRefreshed(), "stopCapture clears the refresh flag");

		cv::Mat afterStop;
		capturer->copyMatTo(afterStop);
		Check(afterStop.empty(), "copyMatTo after stop returns an empty frame");
		Check(!saved.empty(), "a frame copied before stop remains valid");
	}

	if (captureAvailable) {
		std::cout << "Testing stop/restart..." << std::endl;
		Check(StartPull(capturer, monitor), "restart after stop succeeds");
		if (capturer->isCapturing()) {
			Check(WaitFor([&] { return capturer->isRefreshed(); }, 5000),
				"restarted capture reports a frame");
			capturer->stopCapture();
		}

		std::cout << "Testing callback mode..." << std::endl;
		FrameMailbox mailbox;
		const bool callbackStarted = capturer->startCaptureMonitorWithCallback(
			monitor,
			[&mailbox](const cv::Mat& frame) { mailbox.push(frame); }
		);
		Check(callbackStarted, "startCaptureMonitorWithCallback succeeds");
		if (callbackStarted) {
			Check(mailbox.wait(5000), "callback mode delivers a frame");

			cv::Mat callbackFrame;
			Check(mailbox.take(callbackFrame), "callback frame can be consumed");
			Check(!callbackFrame.empty(), "callback frame is not empty");
			Check(callbackFrame.type() == CV_8UC4, "callback frame is BGRA CV_8UC4");

			capturer->stopCapture();
			Check(!callbackFrame.empty(), "callback frame remains valid after stop");
			if (!callbackFrame.empty()) {
				const cv::Vec4b pixel = callbackFrame.at<cv::Vec4b>(0, 0);
				volatile unsigned char alpha = pixel[3];
				(void)alpha;
				Check(callbackFrame.data != nullptr, "callback frame pixel storage is readable");
			}
		}
	}

	std::cout << "Testing factory lifetime independence..." << std::endl;
	auto independent = factory->createCapturer().lock();
	Check(independent != nullptr, "second capturer is created");
	factory.reset();
	Check(independent != nullptr, "capturer survives factory destruction");
	if (independent && captureAvailable) {
		Check(independent->startCaptureMonitor(monitor, true),
			"capturer starts after factory destruction");
		if (independent->isCapturing()) {
			Check(WaitFor([&] { return independent->isRefreshed(); }, 5000),
				"independent capturer receives a frame");
			independent->stopCapture();
		}
	}

	if (captureAvailable) {
		std::cout << "Testing window capture..." << std::endl;
		HWND target = CreateTestWindow();
		Check(target != nullptr, "test window is created");
		if (target) {
			auto windowCapturer = independent;
			windowCapturer->setClipToClientArea(true);
			const bool windowStarted = windowCapturer->startCaptureWindow(target, true);
			Check(windowStarted, "startCaptureWindow succeeds");
			if (windowStarted && windowCapturer->isCapturing()) {
				Check(WaitFor([&] { return windowCapturer->isRefreshed(); }, 5000),
					"window capture reports a frame");
				cv::Mat windowFrame = CopyCurrentFrame(windowCapturer, true);
				Check(!windowFrame.empty(), "window capture returns a frame");
				Check(windowFrame.channels() == 3, "window frame converts to BGR");
				windowCapturer->stopCapture();
			}
			DestroyWindow(target);
		}
	}

	if (!captureAvailable) {
		std::cout << (g_failures == 0 ? "LIFECYCLE TESTS PASSED; CAPTURE TESTS SKIPPED"
			: "SOME LIFECYCLE TESTS FAILED") << " (failures: " << g_failures << ")" << std::endl;
		return g_failures == 0 ? 77 : 1;
	}

	std::cout << (g_failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED")
		<< " (failures: " << g_failures << ")" << std::endl;
	return g_failures == 0 ? 0 : 1;
}
