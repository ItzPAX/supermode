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

	uintptr_t supermode_dtb, supermode_kproc;
	supermode::attach("TotallyAccurateBattlegrounds.exe", &supermode_dtb, &supermode_kproc);

	while (true)
	{
		std::cout << "Found DTB: " << std::hex << supermode::get_dtb_from_kprocess(supermode_kproc) << std::endl;

		Sleep(1000);
	}

	return 1;
}