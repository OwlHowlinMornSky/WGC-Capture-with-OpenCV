#include "pch.h"
#include "include/ohms/WGC.h"
#include "Capture.h"

namespace {

ohms::wgc::ICapture* g_instance{ nullptr };

bool setupInstance() {
	try {
		winrt::init_apartment();
		::g_instance = new ohms::wgc::Capture();
	}
	catch (...) {
		return false;
	}
	return true;
}

} // namespace

namespace ohms {
namespace wgc {

ICapture* ICapture::getInstance() {
	if (!g_instance && !::setupInstance())
		return nullptr;
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
