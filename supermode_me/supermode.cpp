#include "supermode.h"

void supermode::get_eprocess_offsets()
{
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;

	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"),
		"RtlGetVersion");

	DWORD build = 0;

	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
		build = osInfo.dwBuildNumber;
	}

	switch (build)
	{
	case 22000: //WIN11
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 19045: // WIN10_22H2
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 19044: //WIN10_21H2
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 19043: //WIN10_21H1
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 19042: //WIN10_20H2
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 19041: //WIN10_20H1
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		break;
	case 18363: //WIN10_19H2
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x340;
		EP_SECTIONBASE = 0x3c8;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 18362: //WIN10_19H1
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x340;
		EP_SECTIONBASE = 0x3c8;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 17763: //WIN10_RS5
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 17134: //WIN10_RS4
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 16299: //WIN10_RS3
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 15063: //WIN10_RS2
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		break;
	case 14393: //WIN10_RS1
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		break;
	default:
		exit(0);
		break;
	}
}

uint64_t supermode::get_process_id(const char* image_name)
{
	HANDLE hsnap;
	PROCESSENTRY32 pt;
	DWORD PiD;
	hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	do {
		if (!strcmp(pt.szExeFile, image_name)) {
			CloseHandle(hsnap);
			PiD = pt.th32ProcessID;
			return PiD;
			if (PiD != NULL) {
				return 0;
			}
		}
	} while (Process32Next(hsnap, &pt));
	return 1;
}

uintptr_t supermode::get_process_base_um(uint64_t pid, const char* name)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_stricmp(modEntry.szModule, name))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

uintptr_t supermode::get_dtb_from_process_base(uintptr_t base, uintptr_t valid_dtb)
{
	auto ntdll_address = (uintptr_t)GetModuleHandleA("ntdll.dll");
	if (!ntdll_address) {
		return false;
	}

	auto nt_dll_physical = supermode::convert_virtual_to_physical(ntdll_address, valid_dtb);

	for (std::uintptr_t i = 3000; i != 0x50000000; i++)
	{
		std::uintptr_t dtb = i << 12;

		if (dtb == valid_dtb)
			continue;

		std::cout << "Checking DTB " << std::hex << dtb << std::dec << "..." << std::endl;

		auto phys_address = supermode::convert_virtual_to_physical(ntdll_address, dtb);

		if (!phys_address)
			continue;

		if (phys_address == nt_dll_physical)
		{
			std::cout << "Valid DTB " << std::hex << dtb << std::dec << "!!!!!!!" << std::endl;
			bool succ;
			const auto bytes = supermode::read_virtual_memory<short>(base, &succ, dtb);
			if (succ && bytes == 0x5A4D)
			{
				return dtb;
			}
		}
	}

	return 0;
}

uintptr_t supermode::get_dtb_from_kprocess(uintptr_t kprocess)
{
	byte kproc_buf[0x1000];
	read_virtual_memory(kprocess, (uint64_t*)kproc_buf, 0x1000, supermode_comm::system_cr3);

	uintptr_t virtual_size;
	memcpy(&virtual_size, &kproc_buf[EP_VIRTUALSIZE], sizeof(uintptr_t));

	if (virtual_size == 0)
		return 0;

	bool success = true;
	uintptr_t directory_table;
	memcpy(&directory_table, &kproc_buf[EP_DIRECTORYTABLE], sizeof(uintptr_t));

	return directory_table;
}

uintptr_t supermode::attach(const char* image_name, uintptr_t* out_cr3, uintptr_t* out_kprocess)
{
	supermode_comm::load();

	get_eprocess_offsets();

	if (!supermode_comm::system_cr3)
	{
		std::cout << "system cr3 NOT set: " << std::hex << supermode_comm::system_cr3 << std::endl;
		return NULL;
	}

	uintptr_t kprocess_initial = leak_kprocess();

	if (!kprocess_initial)
	{
		std::cout << "couldnt find kprocess\n";
		return NULL;
	}

	printf("system_kprocess: %llx\n", kprocess_initial);

	const unsigned long limit = 400;

	uintptr_t link_start = kprocess_initial + EP_ACTIVEPROCESSLINK;
	uintptr_t flink = link_start;
	uintptr_t image_base_out = 0;

	for (int a = 0; a < limit; a++)
	{
		read_virtual_memory(flink, (uintptr_t*)&flink, sizeof(PVOID), supermode_comm::system_cr3);

		uintptr_t kprocess = flink - EP_ACTIVEPROCESSLINK;

		byte kproc_buf[0x1000];
		read_virtual_memory(kprocess, (uint64_t*)kproc_buf, 0x1000, supermode_comm::system_cr3);

		uintptr_t virtual_size;
		memcpy(&virtual_size, &kproc_buf[EP_VIRTUALSIZE], sizeof(uintptr_t));

		if (virtual_size == 0)
			continue;

		int process_id = 0;
		memcpy(&process_id, &kproc_buf[EP_UNIQUEPROCESSID], sizeof(int));

		char name[16] = { };
		memcpy((uintptr_t*) & name, &kproc_buf[EP_IMAGEFILENAME], sizeof(name));

		if (strstr(image_name, name) && process_id == get_process_id(image_name))
		{
			bool success = true;
			uintptr_t directory_table;
			memcpy(&directory_table, &kproc_buf[EP_DIRECTORYTABLE], sizeof(uintptr_t));

			uintptr_t base_address;
			memcpy(&base_address, &kproc_buf[EP_SECTIONBASE], sizeof(uintptr_t));

			printf("process_id: %i\n", process_id);
			printf("process_base: %llx\n", base_address);
			printf("process_cr3: %llx\n", directory_table);

			image_base_out = base_address;
			attached_cr3 = directory_table;
			attached_proc = process_id;

			*out_kprocess = kprocess;
			*out_cr3 = directory_table;

			break;
		}
	}
	return image_base_out;
}

uintptr_t supermode::leak_kprocess()
{
	if (!supermode_comm::system_cr3)
	{
		std::cout << "system cr3 not set!\n";
		exit(2);
	}

	std::vector<uintptr_t> pointers;

	if (!leak_kpointers(pointers))
	{
		std::cout << "failed to leak kpointers\n";
		return false;
	}
	std::cout << "got kpointers!\n";

	const unsigned int sanity_check = 0x3;

	for (uintptr_t pointer : pointers)
	{
		unsigned int check = 0;

		read_virtual_memory(pointer, (uintptr_t*) & check, sizeof(unsigned int), supermode_comm::system_cr3);

		if (check == sanity_check)
		{
			std::cout << "found kprocess\n" << std::endl;
			return pointer;
			break;
		}
	}

	return NULL;
}

bool supermode::leak_kpointers(std::vector<uintptr_t>& pointers)
{
	const unsigned long SystemExtendedHandleInformation = 0x40;

	unsigned long buffer_length = 0;
	unsigned char tempbuffer[1024] = { 0 };
	NTSTATUS status = NtQuerySystemInformation(static_cast<SYSTEM_INFORMATION_CLASS>(SystemExtendedHandleInformation), &tempbuffer, sizeof(tempbuffer), &buffer_length);

	buffer_length += 50 * (sizeof(SYSTEM_HANDLE_INFORMATION_EX) + sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX));

	PVOID buffer = VirtualAlloc(nullptr, buffer_length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	RtlSecureZeroMemory(buffer, buffer_length);

	unsigned long buffer_length_correct = 0;
	status = NtQuerySystemInformation(static_cast<SYSTEM_INFORMATION_CLASS>(SystemExtendedHandleInformation), buffer, buffer_length, &buffer_length_correct);

	SYSTEM_HANDLE_INFORMATION_EX* handle_information = reinterpret_cast<SYSTEM_HANDLE_INFORMATION_EX*>(buffer);

	for (unsigned int i = 0; i < handle_information->NumberOfHandles; i++)
	{
		const unsigned int SystemUniqueReserved = 4;
		const unsigned int SystemKProcessHandleAttributes = 0x102A;

		if (handle_information->Handles[i].UniqueProcessId == SystemUniqueReserved &&
			handle_information->Handles[i].HandleAttributes == SystemKProcessHandleAttributes)
		{
			pointers.push_back(reinterpret_cast<uintptr_t>(handle_information->Handles[i].Object));
		}
	}

	VirtualFree(buffer, 0, MEM_RELEASE);
	return true;
}

bool supermode::read_virtual_memory(uintptr_t address, uint64_t* output, unsigned long size, uint64_t cr3)
{
	if (!address)
		return false;

	if (!size || size > 0x1000)
		return false;

	uintptr_t physical_address = convert_virtual_to_physical(address, cr3);

	if (!physical_address)
		return false;

	supermode_comm::read_physical_memory(physical_address, size, output);
	return true;
}

bool supermode::write_virtual_memory(uintptr_t address, uint64_t* data, unsigned long size, uint64_t cr3)
{
	uintptr_t physical_address = convert_virtual_to_physical(address, cr3);

	if (!physical_address)
		return false;

	supermode_comm::write_physical_memory(physical_address, size, data);
	return true;
}

uintptr_t supermode::convert_virtual_to_physical(uintptr_t virtual_address, uint64_t cr3)
{
	if (cr3 == USE_PROCESS_CR3)
		cr3 = attached_cr3;

	uintptr_t va = virtual_address;

	if (tlb[va + cr3] != 0)
		return tlb[va + cr3];

	unsigned short PML4 = (unsigned short)((va >> 39) & 0x1FF);
	supermode_comm::PML4E PML4E;

	if (!supermode_comm::read_physical_memory((cr3 + PML4 * sizeof(uintptr_t)), sizeof(PML4E), (uintptr_t*)&PML4E))
		return 0;

	if (PML4E.Present == 0 || PML4E.Reserved != 0)
	{
		return 0;
	}

	unsigned short DirectoryPtr = (unsigned short)((va >> 30) & 0x1FF);

	supermode_comm::PDPTE PDPTE;
	
	if (!supermode_comm::read_physical_memory(((PML4E.Value & 0xFFFFFFFFFF000) + DirectoryPtr * sizeof(uintptr_t)), sizeof(PDPTE), (uintptr_t*)&PDPTE))
		return 0;

	if (PDPTE.Present == 0 || PDPTE.Reserved != 0)
	{
		return 0;
	}

	if (PDPTE.PageSize != 0)
	{
		tlb[va + cr3] = (PDPTE.Value & 0xFFFFFC0000000) + (va & 0x3FFFFFFF);
		return (PDPTE.Value & 0xFFFFFC0000000) + (va & 0x3FFFFFFF);
	}

	unsigned short Directory = (unsigned short)((va >> 21) & 0x1FF);

	supermode_comm::PDE PDE;

	if (!supermode_comm::read_physical_memory(((PDPTE.Value & 0xFFFFFFFFFF000) + Directory * sizeof(uintptr_t)), sizeof(PDE), (uintptr_t*)&PDE))
		return 0;

	if (PDE.Present == 0 || PDE.Reserved != 0)
	{
		return 0;
	}

	if (PDE.PageSize != 0)
	{
		tlb[va + cr3] = (PDE.Value & 0xFFFFFFFE00000) + (va & 0x1FFFFF);
		return (PDE.Value & 0xFFFFFFFE00000) + (va & 0x1FFFFF);
	}

	unsigned short Table = (unsigned short)((va >> 12) & 0x1FF);

	supermode_comm::PTE PTE;

	if (!supermode_comm::read_physical_memory(((PDE.Value & 0xFFFFFFFFFF000) + Table * sizeof(uintptr_t)), sizeof(PTE), (uintptr_t*)&PTE))
		return 0;

	if (PTE.Present == 0 || PTE.Reserved != 0)
	{
		return 0;
	}

	tlb[va + cr3] = (PTE.Value & 0xFFFFFFFFFF000) + (va & 0xFFF);
	return (PTE.Value & 0xFFFFFFFFFF000) + (va & 0xFFF);
}
