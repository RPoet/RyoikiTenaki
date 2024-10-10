#pragma once
#include "RenderBackend.h"

extern RRenderBackend* GBackend;
void InitBackend(const String& BackendName);
void TeardownBackend();