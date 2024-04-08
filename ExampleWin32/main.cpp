
#include <ohms/WGC.h>

#include "framework.h"
#include "Window/MainWindow.h"

int CALLBACK wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
) {
	// Init
	ohms::wgc::ICapture::setup(false);
	// Run
	{
		ohms::MainWindow mainWnd;
		mainWnd.create(nShowCmd);
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		mainWnd.destroy();
	}
	// Clear
	ohms::wgc::ICapture::drop();
	return 0;
}
