#include "drv.h"

bool wnbios_lib::to_file()
{
	if (std::filesystem::exists(store_at + '\\' + drv_name))
		return 1;

	std::filesystem::create_directories(store_at);

	std::ofstream out_driver(store_at + '\\' + drv_name, std::ios::beg | std::ios::binary);
	if (!out_driver.is_open())
		return 0;

	for (auto& c : driver::wnbios64)
		out_driver << c;
	out_driver.close();

	return 1;
}

bool wnbios_lib::register_and_start()
{
	const static DWORD ServiceTypeKernel = 1;
	const std::string driver_name = drv_name;
	const std::string servicesPath = "SYSTEM\\CurrentControlSet\\Services\\" + service_name;
	const std::string nPath = "\\??\\" + store_at + '\\' + driver_name;

	HKEY dservice;
	LSTATUS status = RegCreateKeyA(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &dservice);
	if (status != ERROR_SUCCESS)
		return false;


	status = RegSetKeyValueA(dservice, NULL, "ImagePath", REG_EXPAND_SZ, nPath.c_str(), (DWORD)(nPath.size() + 1));
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		return false;
	}

	status = RegSetKeyValueA(dservice, NULL, "Type", REG_DWORD, &ServiceTypeKernel, sizeof(DWORD));
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		return false;
	}

	RegCloseKey(dservice);

	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	if (ntdll == NULL) {
		return false;
	}

	auto RtlAdjustPrivilege = (nt::RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
	auto NtLoadDriver = (nt::NtLoadDriver)GetProcAddress(ntdll, "NtLoadDriver");

	ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
	BOOLEAN SeLoadDriverWasEnabled;
	NTSTATUS Status = RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE, TRUE, FALSE, &SeLoadDriverWasEnabled);
	if (!NT_SUCCESS(Status)) {
		return false;
	}

	std::string driver_reg_path = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + service_name;
	std::wstring wdriver_reg_path(driver_reg_path.begin(), driver_reg_path.end());
	UNICODE_STRING serviceStr;
	RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

	Status = NtLoadDriver(&serviceStr);
	if (Status == 0xC000010E) {
		return true;
	}

	return NT_SUCCESS(Status);
}

bool wnbios_lib::stop_and_delete()
{
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	if (ntdll == NULL)
		return false;

	std::string driver_reg_path = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + service_name;
	std::wstring wdriver_reg_path(driver_reg_path.begin(), driver_reg_path.end());
	UNICODE_STRING serviceStr;
	RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

	HKEY driver_service;
	std::string servicesPath = "SYSTEM\\CurrentControlSet\\Services\\" + service_name;
	LSTATUS status = RegOpenKeyA(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &driver_service);
	if (status != ERROR_SUCCESS) {
		if (status == ERROR_FILE_NOT_FOUND) {
			return true;
		}
		return false;
	}
	RegCloseKey(driver_service);

	auto NtUnloadDriver = (nt::NtUnloadDriver)GetProcAddress(ntdll, "NtUnloadDriver");
	NTSTATUS st = NtUnloadDriver(&serviceStr);
	if (st != 0x0) {
		status = RegDeleteTreeA(HKEY_LOCAL_MACHINE, servicesPath.c_str());
		return false; 
	}


	status = RegDeleteTreeA(HKEY_LOCAL_MACHINE, servicesPath.c_str());
	if (status != ERROR_SUCCESS) {
		return false;
	}
	return true;
}

void wnbios_lib::get_eprocess_offsets() {

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
		EP_VADROOT = 0x7d8;
		break;
	case 19045: // WIN10_22H2
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		EP_VADROOT = 0x7d8;
		break;
	case 19044: //WIN10_21H2
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		EP_VADROOT = 0x7d8;
		break;
	case 19043: //WIN10_21H1
		EP_UNIQUEPROCESSID = 0x440;
		EP_ACTIVEPROCESSLINK = 0x448;
		EP_VIRTUALSIZE = 0x498;
		EP_SECTIONBASE = 0x520;
		EP_IMAGEFILENAME = 0x5a8;
		EP_VADROOT = 0x7d8;
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
		EP_VADROOT = 0x7d8;
		break;
	case 18363: //WIN10_19H2
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x340;
		EP_SECTIONBASE = 0x3c8;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x658;
		break;
	case 18362: //WIN10_19H1
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x340;
		EP_SECTIONBASE = 0x3c8;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x658;
		break;
	case 17763: //WIN10_RS5
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x628;
		break;
	case 17134: //WIN10_RS4
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x628;
		break;
	case 16299: //WIN10_RS3
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x628;
		break;
	case 15063: //WIN10_RS2
		EP_UNIQUEPROCESSID = 0x2e0;
		EP_ACTIVEPROCESSLINK = 0x2e8;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x628;
		break;
	case 14393: //WIN10_RS1
		EP_UNIQUEPROCESSID = 0x2e8;
		EP_ACTIVEPROCESSLINK = 0x2f0;
		EP_VIRTUALSIZE = 0x338;
		EP_SECTIONBASE = 0x3c0;
		EP_IMAGEFILENAME = 0x450;
		EP_VADROOT = 0x620;
		break;
	default:
		exit(0);
		break;
	}
}

uintptr_t wnbios_lib::leak_kprocess()
{
	std::vector<uintptr_t> pointers;

	if (!leak_kpointers(pointers))
	{
		return false;
	}

	const unsigned int sanity_check = 0x3;

	for (uintptr_t pointer : pointers)
	{
		unsigned int check = 0;

		read_virtual_memory(pointer, &check, sizeof(unsigned int));

		if (check == sanity_check)
		{
			return pointer;
			break;
		}
	}

	return NULL;
}


bool wnbios_lib::leak_kpointers(std::vector<uintptr_t>& pointers)
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

uint32_t wnbios_lib::find_self_referencing_pml4e()
{
	auto dirbase = get_system_dirbase();

	// find a valid entry
	for (int i = 1; i < 512; i++)
	{
		driver::PML4E pml4e;
		if (!read_physical_memory((dirbase + i * sizeof(uintptr_t)), &pml4e, sizeof(driver::PML4E)))
		{
			return 0;
		}

		// page backs physical memory
		if (pml4e.Present && pml4e.PageFrameNumber * 0x1000 == dirbase)
		{
			return i;
		}
	}

	return -1;
}


uintptr_t wnbios_lib::map_physical(uint64_t address, size_t size, wnbios_mem& mem)
{
	memset(&mem, 0, sizeof(wnbios_mem));
	mem.addr = address;
	mem.size = size;
	DWORD retSize;
	auto status = DeviceIoControl(hHandle, 0x80102040, &mem, sizeof(wnbios_mem), &mem, sizeof(wnbios_mem), &retSize, 0);
	if (!status)
		return 0;
	
	return mem.outPtr;
}

uintptr_t wnbios_lib::unmap_physical(wnbios_mem& mem)
{
	DWORD bytes_returned;
	auto status = DeviceIoControl(hHandle, 0x80102044, &mem, sizeof(wnbios_mem), 0, 0, &bytes_returned, 0);
	if (!status)
		return 0;

	return 1;
}

uintptr_t wnbios_lib::get_system_dirbase()
{
	for (int i = 0; i < 10; i++)
	{
		wnbios_mem mem;
		uintptr_t lpBuffer = map_physical(i * 0x10000, 0x10000, mem);

		for (int uOffset = 0; uOffset < 0x10000; uOffset += 0x1000)
		{
			if (0x00000001000600E9 ^ (0xffffffffffff00ff & *reinterpret_cast<uintptr_t*>(lpBuffer + uOffset)))
				continue;
			if (0xfffff80000000000 ^ (0xfffff80000000000 & *reinterpret_cast<uintptr_t*>(lpBuffer + uOffset + 0x70)))
				continue;
			if (0xffffff0000000fff & *reinterpret_cast<uintptr_t*>(lpBuffer + uOffset + 0xa0))
				continue;

			return *reinterpret_cast<uintptr_t*>(lpBuffer + uOffset + 0xa0);
		}

		unmap_physical(mem);
	}

	return NULL;
}

uintptr_t wnbios_lib::get_process_id(const char* image_name)
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

uintptr_t wnbios_lib::get_process_base(const char* image_name)
{
	get_eprocess_offsets();
	cr3 = get_system_dirbase();

	if (!cr3)
		return NULL;

	uintptr_t kprocess_initial = leak_kprocess();

	if (!kprocess_initial)
		return NULL;

	//printf("system_kprocess: %llx\n", kprocess_initial);
	//printf("system_cr3: %llx\n", cr3);

	const unsigned long limit = 400;

	uintptr_t link_start = kprocess_initial + EP_ACTIVEPROCESSLINK;
	uintptr_t flink = link_start;
	uintptr_t image_base_out = 0;


	for (int a = 0; a < limit; a++)
	{
		read_virtual_memory(flink, &flink, sizeof(PVOID));

		uintptr_t kprocess = flink - EP_ACTIVEPROCESSLINK;
		uintptr_t virtual_size = read_virtual_memory<uintptr_t>(kprocess + EP_VIRTUALSIZE);

		if (virtual_size == 0)
			continue;

		uintptr_t directory_table = read_virtual_memory<uintptr_t>(kprocess + EP_DIRECTORYTABLE);
		uintptr_t base_address = read_virtual_memory<uintptr_t>(kprocess + EP_SECTIONBASE);

		char name[16] = { };
		read_virtual_memory(kprocess + EP_IMAGEFILENAME, &name, sizeof(name));

		int process_id = 0;
		read_virtual_memory(kprocess + EP_UNIQUEPROCESSID, &process_id, sizeof(process_id));

		if (strstr(image_name, name) && process_id == get_process_id(image_name))
		{
			printf("process_id: %i\n", process_id);
			printf("process_base: %llx\n", base_address);
			printf("process_cr3: %llx\n", directory_table);

			image_base_out = base_address;
			cr3 = directory_table;
			attached_proc = process_id;
			attached_eproc = kprocess;

			break;
		}
	}
	
	return image_base_out;
}

uintptr_t wnbios_lib::get_process_base_um(uint64_t pid, const char* name)
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
				printf("Entry: %s\n", modEntry.szModule);
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

uintptr_t wnbios_lib::get_ntoskrnl_base()
{
	for (int i = 0; i < (0x100000 * 120); i += 0x100000)
	{
		unsigned char* buffer = (unsigned char*)VirtualAlloc(0, 8, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (read_physical_memory(i + 0x2BF, (uint64_t*)buffer, 1)) {
			if (buffer && buffer[0] == '@')
				return i;
		}
	}

	return 0;
}

uintptr_t wnbios_lib::find_pattern_at_kernel(uintptr_t base, byte* pattern, const char* mask)
{
	if (!base) 
		return 0;

	uintptr_t offset = 0;
	int retries = 0;
	uintptr_t chunk_size = 0x100000;
	
	retry:
	auto sectionData = std::make_unique<BYTE[]>(chunk_size);
	if (!read_physical_memory(base + offset, sectionData.get(), chunk_size))
		return 0;

	auto result = utils::find_pattern((uintptr_t)sectionData.get(), chunk_size, pattern, mask);

	if (result <= 0)
	{
		offset += chunk_size;
		retries++;
		goto retry;
	}

	result = (base + offset) - (uintptr_t)sectionData.get() + result;
	return result;
}

uintptr_t wnbios_lib::find_dtb_from_base(uintptr_t base)
{
	uint32_t self_ref_entry = find_self_referencing_pml4e();
	if (self_ref_entry == -1)
		return 0;

	for (std::uintptr_t i = 0x100000; i != 0x50000000; i++)
	{
		std::uintptr_t dtb = i << 12;

		if (i % 0x100000 == 0)
			printf("reached dtb: 0x%p\n", dtb);

		driver::PML4E PML4E;
		if (!read_physical_memory((dtb + self_ref_entry * sizeof(uintptr_t)), &PML4E, sizeof(PML4E)))
			continue;

		if (PML4E.Present == 0 || PML4E.PageFrameNumber * 0x1000 != dtb)
			continue;

		const auto bytes = read_virtual_memory<short>(base, dtb);
		if (bytes == 0x5A4D)
		{
			return dtb;
		}
	}
	return 0;
}

bool wnbios_lib::read_physical_memory(uintptr_t physical_address, void* output, unsigned long size)
{
	wnbios_mem mem;
	uintptr_t virtual_address = map_physical(physical_address, size, mem);

	if (!virtual_address)
		return false;

	memcpy(output, reinterpret_cast<void*>(virtual_address), size);
	unmap_physical(mem);
	return true;
}

bool wnbios_lib::write_physical_memory(uintptr_t physical_address, void* data, unsigned long size)
{
	if (!data)
		return false;

	wnbios_mem mem;
	uintptr_t virtual_address = map_physical(physical_address, size, mem);

	if (!virtual_address)
		return false;

	memcpy(reinterpret_cast<void*>(virtual_address), reinterpret_cast<void*>(data), size);
	unmap_physical(mem);
	return true;
}

uintptr_t wnbios_lib::convert_virtual_to_physical(uintptr_t virtual_address, uintptr_t dtb)
{
	if (dtb == -1)
		dtb = cr3;

	uintptr_t directoryTableBase = dtb & ~0xf;

	UINT64 pageOffset = virtual_address & ~(~0ul << PAGE_OFFSET_SIZE);
	UINT64 pte = ((virtual_address >> 12) & (0x1ffll));
	UINT64 pt = ((virtual_address >> 21) & (0x1ffll));
	UINT64 pd = ((virtual_address >> 30) & (0x1ffll));
	UINT64 pdp = ((virtual_address >> 39) & (0x1ffll));

	SIZE_T readsize = 0;
	UINT64 pdpe = 0;
	read_physical_memory((directoryTableBase + 8 * pdp), &pdpe, sizeof(pdpe));
	if (~pdpe & 1)
		return 0;

	UINT64 pde = 0;
	read_physical_memory(((pdpe & PMASK) + 8 * pd), &pde, sizeof(pde));
	if (~pde & 1)
		return 0;

	/* 1GB large page, use pde's 12-34 bits */
	if (pde & 0x80)
		return (pde & (~0ull << 42 >> 12)) + (virtual_address & ~(~0ull << 30));

	UINT64 pteAddr = 0;
	read_physical_memory(((pde & PMASK) + 8 * pt), &pteAddr, sizeof(pteAddr));
	if (~pteAddr & 1)
		return 0;

	/* 2MB large page */
	if (pteAddr & 0x80)
		return (pteAddr & PMASK) + (virtual_address & ~(~0ull << 21));

	virtual_address = 0;
	read_physical_memory(((pteAddr & PMASK) + 8 * pte), &virtual_address, sizeof(virtual_address));
	virtual_address &= PMASK;

	if (!virtual_address)
		return 0;

	return virtual_address + pageOffset;
}

bool wnbios_lib::read_virtual_memory(uintptr_t address, LPVOID output, unsigned long size, uintptr_t dtb)
{
	if (!address)
		return false;

	if (!size)
		return false;

	uintptr_t physical_address = convert_virtual_to_physical(address, dtb);

	if (!physical_address)
		return false;

	read_physical_memory(physical_address, output, size);
	return true;
}

bool wnbios_lib::write_virtual_memory(uintptr_t address, LPVOID data, unsigned long size)
{
	uintptr_t physical_address = convert_virtual_to_physical(address);

	if (!physical_address)
		return false;

	write_physical_memory(physical_address, data, size);
	return true;
}

bool utils::data_compare(const BYTE* data, const BYTE* pattern, const char* mask)
{
	for (; *mask; ++mask, ++data, ++pattern)
		if (*mask == 'x' && *data != *pattern)
			return 0;
	return (*mask) == 0;
}

uintptr_t utils::find_pattern(uintptr_t base, uintptr_t len, BYTE* pattern, const char* mask)
{
	size_t max_len = len - strlen(mask);
	for (uintptr_t i = 0; i < max_len; i++)
		if (data_compare((BYTE*)(base + i), pattern, mask))
			return (uintptr_t)(base + i);
	return 0;
}