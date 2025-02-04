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
#include "include/WGC/WGC.h"
#include "Factory.h"

namespace wgc {

std::shared_ptr<IFactory> IFactory::createInstance(bool winrt_initialized) {
	if (!winrt_initialized) {
		winrt::init_apartment();
	}
	return std::make_shared<wgc::Factory>();
}

std::shared_ptr<IFactory> IFactory::createInstanceNoThrow(bool winrt_initialized) noexcept {
	if (!winrt_initialized) {
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
		return std::make_shared<wgc::Factory>();
	}
	catch (...) {
		MessageBoxW(NULL, L"Failed to create instance.", L"WGC: Init failed", MB_ICONERROR);
	}
	return nullptr;
}

} // namespace wgc
