#include "rwptm.h"

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
	
	std::cout << "Open the game...\n";
	system("pause");

	rwptm::init(TARGET_NAME.c_str(), LOCAL_NAME.c_str());
	system("pause");

	std::cout << "NTDLL base: " << supermode::get_module_base(L"ntdll.dll", rwptm::local_eproc, rwptm::local_cr3) << std::endl;

	while (true)
	{
		std::cout << std::hex << rwptm::read_virtual_memory<short>(rwptm::target_base) << std::endl;
		Sleep(1);
	}

	system("pause");

	return 1;
}