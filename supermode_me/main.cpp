#include "val_shit/utils.h"

uintptr_t val_cr3 = 0;
void startup()
{
	std::cout << "start the game now!\n";
	system("pause");

	//rwptm::init("VALORANT-Win64-Shipping.exe", "supermode_me.exe");

	supermode::attach("VALORANT-Win64-Shipping.exe", &val_cr3);

	auto guardedregion = utils::retrieve_guarded();
	printf("guardedregion: 0x%p\n", guardedregion);

	Sleep(1000);

	while (true)
	{
		auto uworld = utils::getuworld(guardedregion);
		printf("uworld: 0x%p\n", uworld);

		auto ulevel = supermode::read_virtual_memory< uintptr_t >(uworld + offsets::ulevel, val_cr3);
		printf("ulevel: 0x%p\n", ulevel);

		Sleep(1);
	}
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