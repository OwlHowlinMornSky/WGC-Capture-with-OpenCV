

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
	r_capture->askForRefresh();
	MSG msg{ 0 };
	size_t testCnt = 3300; // About 100 seconds.
	while (testCnt > 0) {
		if (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {
			if (r_capture->isRefreshed()) {
				cv::Mat mat;
				if (r_capture->copyMatTo(mat, false)) {
					cv::imshow("show", mat);
				}
				r_capture->askForRefresh();
			}
			Sleep(30);
			--testCnt;
		}
	}
	// Clear.
	ohms::wgc::ICapture::drop();
	return 0;
}

