//=============================================================================
// main.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "RayTracerProgram.h"

#if (PLATFORM_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	RayTracerProgram Program;
	Program.Run();

	return 0;
}
