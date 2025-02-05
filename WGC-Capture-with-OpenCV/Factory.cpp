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
#include "Factory.h"
#include "Capturer.h"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Capture;

namespace {

inline auto CreateD3DDevice(
	D3D_DRIVER_TYPE const type,
	com_ptr<ID3D11Device>& device
) {
	WINRT_ASSERT(!device);
	constexpr UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	return D3D11CreateDevice(nullptr, type, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, device.put(), nullptr, nullptr);
}

inline auto CreateD3DDevice() {
	com_ptr<ID3D11Device> device;
	HRESULT hr = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE, device);
	if (DXGI_ERROR_UNSUPPORTED == hr)
		hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
	check_hresult(hr);
	return device;
}

extern "C" {
	HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(
		IDXGIDevice* dxgiDevice,
		::IInspectable** graphicsDevice
	);
}

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device) {
	com_ptr<::IInspectable> d3d_device;
	check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
	return d3d_device.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

size_t g_capturerCnt = 0;

} // namespace 

namespace wgc {

Factory::Factory() :
	m_device(nullptr) {
	com_ptr<ID3D11Device> d3dDevice = CreateD3DDevice();
	com_ptr<IDXGIDevice> dxgiDevice = d3dDevice.as<IDXGIDevice>();
	m_device = CreateDirect3DDevice(dxgiDevice.get());
}

Factory::~Factory() {
	m_capturers.clear();
	m_device.Close();
}

std::weak_ptr<ICapturer> Factory::createCapturer() {
	size_t id = g_capturerCnt++;
	auto capture = std::make_shared<Capturer>(m_device, id);
	m_capturers.emplace(id, capture);
	return capture;
}

void Factory::destroyCapturer(std::weak_ptr<ICapturer> instance) {
	auto capturer = instance.lock();
	if (capturer == nullptr)
		return;
	auto id = capturer->getId();
	auto it = m_capturers.find(id);
	if (it == m_capturers.end())
		return;
	m_capturers.erase(it);
}

} // namespace wgc
