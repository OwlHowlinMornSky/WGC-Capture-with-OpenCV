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

namespace {

size_t g_capturerCnt = 0;

} // namespace 

namespace wgc {

Factory::Factory() = default;

Factory::~Factory() {
	m_capturers.clear();
}

std::weak_ptr<ICapturer> Factory::createCapturer() {
	size_t id = g_capturerCnt++;
	auto capture = std::make_shared<Capturer>(id);
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
