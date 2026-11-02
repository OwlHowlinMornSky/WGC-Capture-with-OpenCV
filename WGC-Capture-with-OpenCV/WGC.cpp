/*
*    WGC-Capture-with-OpenCV
*
*     Copyright 2023-2025  Tyler Parret True
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

#include <cwchar>

namespace {

thread_local HRESULT g_lastError = S_OK;
thread_local wchar_t g_lastErrorMessage[512] = {};

} // namespace

namespace wgc {

void SetCaptureError(HRESULT code, const wchar_t* message) noexcept {
	g_lastError = code;
	if (code == S_OK) {
		g_lastErrorMessage[0] = L'\0';
		return;
	}
	wcsncpy_s(
		g_lastErrorMessage,
		sizeof(g_lastErrorMessage) / sizeof(g_lastErrorMessage[0]),
		message ? message : L"",
		_TRUNCATE
	);
}

HRESULT getLastError() noexcept {
	return g_lastError;
}

const wchar_t* getLastErrorMessage() noexcept {
	return g_lastErrorMessage;
}

std::shared_ptr<IFactory> IFactory::createInstance(bool winrt_initialized) {
	if (!winrt_initialized) {
		winrt::init_apartment();
	}
	return std::make_shared<wgc::Factory>();
}

std::shared_ptr<IFactory> IFactory::createInstanceNoThrow(bool winrt_initialized) noexcept {
	try {
		if (!winrt_initialized) {
			winrt::init_apartment();
		}
		return std::make_shared<wgc::Factory>();
	}
	catch (...) {
		SetCaptureError(E_FAIL, L"Failed to create the capture factory.");
		return nullptr;
	}
}

} // namespace wgc
