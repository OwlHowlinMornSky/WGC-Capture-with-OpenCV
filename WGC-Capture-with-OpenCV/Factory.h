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
#pragma once

#include "pch.h"

#include "include/WGC/WGC.h"
#include <map>

namespace wgc {

class Factory final :
	public IFactory {
public:
	Factory();
	virtual ~Factory() override;

public:
	virtual std::weak_ptr<ICapturer> createCapturer() override;
	virtual std::weak_ptr<ICapturer> createCapturerNoThrow() override;
	virtual void destroyCapturer(std::weak_ptr<ICapturer> instance) override;

protected:
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device;
	std::map<size_t, std::shared_ptr<ICapturer>> m_capturers;
};

} // namespace wgc
