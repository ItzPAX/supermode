#include "rwptm.h"
#include "entity.h"
#include "offsets.h"

int start_sm(int argc, char* argv[])
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

	return 1;
}

void update_entity(entity* pent)
{
	pent->position = rwptm::read_virtual_memory<vec3>(pent->address + player::vOrigin);
	pent->health = rwptm::read_virtual_memory<int>(pent->address + player::iHealth);
}

void cls()
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD topLeft = { 0, 0 };
	std::cout.flush();

	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		abort();
	}
	DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

	DWORD written;

	FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

	FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

	SetConsoleCursorPosition(hOut, topLeft);
}

void cheat_thread()
{
	DWORD csid = supermode::get_process_id("cs2.exe");
	std::cout << "csid: " << csid << std::endl;

	entity local_player{};
	std::vector<entity> entity_list;

	uintptr_t client;

	client = supermode::get_module_base(L"client.dll", rwptm::target_eproc, rwptm::target_cr3);
	std::cout << "clientdll: " << client << std::endl;

	while (true)
	{
		entity_list.clear();

		local_player.address = rwptm::read_virtual_memory<uintptr_t>(client + client_dll::dwLocalPlayerPawn);
		if (!local_player.address)
		{
			continue;
		}

		update_entity(&local_player);

		cls();

		std::cout << "health: " << std::dec << local_player.health << std::endl;
		std::cout << "pos x: " << std::dec << local_player.position.x << " pos y: " << local_player.position.y << " pos z: " << local_player.position.z << std::endl;

		Sleep(1);
	}
}

int main(int argc, char* argv[])
{
	if (start_sm(argc, argv) != 1)
		return -1;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)cheat_thread, NULL, NULL, NULL);
	_fgetchar();

	return 1;
}