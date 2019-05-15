#pragma once

extern IShaderSystem* g_ShaderSystem;
inline class IShaderSystem* GetShaderSystem() { return g_ShaderSystem; }
