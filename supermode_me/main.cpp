#include "rwptm.h"

uintptr_t val_cr3 = 0;
void startup()
{
	std::cout << "start the game now!\n";
	system("pause");

	rwptm::init("explorer.exe", "supermode_me.exe");

	Sleep(1000);

	std::cout << rwptm::read_virtual_memory<short>(rwptm::target_base) << std::endl;

	system("pause");
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please start supermode.exe instead of this application\n";
		system("pause");
		return -1;
	}
	else if (strcmp(argv[1], "SUPERMODE") != 0)
	{
		std::cout << "Please start supermode.exe instead of this application\n";
		system("pause");
		return -1;
	}

	std::cout << "Starting...\n";

	while (!supermode_comm::load())
	{
		std::cout << "retrying to load...\n";
		Sleep(1000);
	}

	Sleep(1000);
	startup();
	system("pause");

	return 1;
}