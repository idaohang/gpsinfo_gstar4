#include "gstar.h";

int main()
{
	Gstar::Instance()->Start("COM3");
	for(int i=0; i<=10; i++)
	{
		std::cout << Gstar::Instance()->gps_info << std::endl;
		Sleep(1000);
	}
	Gstar::Instance()->Stop();
	std::cin.ignore();

return 0;
}
