#include "pch.h"
#include "include/ohms/WGC.h"
#include "Capture.h"

namespace {

ohms::wgc::ICapture* g_instance{ nullptr };

bool setupInstance(bool initialized) {
	if (!initialized) {
		try {
			winrt::init_apartment();
		}
		catch (winrt::hresult_error const& ex) {
			winrt::hresult hr = ex.code(); // HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND).
			winrt::hstring message = ex.message(); // The system cannot find the file specified.
			MessageBoxW(NULL, message.c_str(), L"WGC: WinRT init failed", MB_ICONERROR);
		}
	}
	try {
		::g_instance = new ohms::wgc::Capture();
	}
	catch (...) {
		MessageBoxW(NULL, L"Failed to create instance.", L"WGC: Init failed", MB_ICONERROR);
		return false;
	}
	return true;
}

} // namespace

namespace ohms {
namespace wgc {

bool ICapture::setup(bool winrt_initialized) {
	if (!g_instance && !::setupInstance(winrt_initialized))
		return false;
	return true;
}

ICapture* ICapture::getInstance() {
	return g_instance;
}

void ICapture::drop() {
	if (g_instance) {
		delete ::g_instance;
		winrt::uninit_apartment();
	}
	return;
}

} // namespace wgc
} // namespace ohms
