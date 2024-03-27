#include <iostream>

#include "Application/Application.h"

class MyApp : public Aqua::Application
{
public:
	MyApp(int argc, char** argv) : Aqua::Application(argc, argv) {}
	// void run() override {}
};

#ifdef NDEBUG
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	std::cout << "Hello world!" << std::endl;

	MyApp app(1, &cmdline);

	app.run();

	return 0;
}
#endif

int main(int argc, char** argv)
{
	std::cout << "Hello world!" << std::endl;

	MyApp app(argc, argv);

	app.run();

	return 0;
}