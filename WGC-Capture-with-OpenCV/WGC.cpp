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
