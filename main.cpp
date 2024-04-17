#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

void Log(const std::string& message)
{
	OutputDebugStringA(message.c_str());
}