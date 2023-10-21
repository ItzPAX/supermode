#include "../supermode.h"
#include "offsets.h"

#include <winternl.h>

namespace utils
{
	auto getuworld( uintptr_t pointer ) -> uintptr_t
	{
		uintptr_t uworld_addr = supermode::read_virtual_memory< uintptr_t >( pointer + offsets::uworldptr );

		unsigned long long uworld_offset;

		if ( uworld_addr > 0x10000000000 )
		{
			uworld_offset = uworld_addr - 0x10000000000;
		}
		else {
			uworld_offset = uworld_addr - 0x8000000000;
		}

		return pointer + uworld_offset;
	}

//bluefire1337
	inline static bool isguarded( uintptr_t pointer )
	{
		static constexpr uintptr_t filter = 0xFFFFFFF000000000;
		uintptr_t result = pointer & filter;
		return result == 0x8000000000 || result == 0x10000000000;
	}

	typedef struct _SYSTEM_BIGPOOL_ENTRY
	{
		union {
			PVOID VirtualAddress;
			ULONG_PTR NonPaged : 1;
		};
		ULONG_PTR SizeInBytes;
		union {
			UCHAR Tag[4];
			ULONG TagUlong;
		};
	} SYSTEM_BIGPOOL_ENTRY, * PSYSTEM_BIGPOOL_ENTRY;

	typedef struct _SYSTEM_BIGPOOL_INFORMATION {
		ULONG Count;
		SYSTEM_BIGPOOL_ENTRY AllocatedInfo[ANYSIZE_ARRAY];
	} SYSTEM_BIGPOOL_INFORMATION, * PSYSTEM_BIGPOOL_INFORMATION;

	typedef enum _SYSTEM_INFORMATION_CLASS {
		SystemBigPoolInformation = 0x42
	} SYSTEM_INFORMATION_CLASS;

	typedef NTSTATUS(WINAPI* PNtQuerySystemInformation)(
		__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
		__inout PVOID SystemInformation,
		__in ULONG SystemInformationLength,
		__out_opt PULONG ReturnLength
		);

	typedef NTSTATUS(WINAPI* pNtQuerySystemInformation)(
		IN SYSTEM_INFORMATION_CLASS	SystemInformationClass,
		OUT PVOID                   SystemInformation,
		IN ULONG                    SystemInformationLength,
		OUT PULONG                  ReturnLength
		);


	__forceinline auto query_bigpools() -> PSYSTEM_BIGPOOL_INFORMATION
	{
		static const pNtQuerySystemInformation NtQuerySystemInformation =
			(pNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");

		DWORD length = 0;
		DWORD size = 0;
		LPVOID heap = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0);
		heap = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, heap, 0xFF);
		NTSTATUS ntLastStatus = NtQuerySystemInformation(SystemBigPoolInformation, heap, 0x30, &length);
		heap = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, heap, length + 0x1F);
		size = length;
		ntLastStatus = NtQuerySystemInformation(SystemBigPoolInformation, heap, size, &length);

		return reinterpret_cast<PSYSTEM_BIGPOOL_INFORMATION>(heap);
	}
	__forceinline auto retrieve_guarded() -> uintptr_t
	{
		auto pool_information = query_bigpools();
		uintptr_t guarded = 0;

		if (pool_information)
		{
			auto count = pool_information->Count;
			for (auto i = 0ul; i < count; i++)
			{
				SYSTEM_BIGPOOL_ENTRY* allocation_entry = &pool_information->AllocatedInfo[i];
				const auto virtual_address = (PVOID)((uintptr_t)allocation_entry->VirtualAddress & ~1ull);
				if (allocation_entry->NonPaged && allocation_entry->SizeInBytes == 0x200000)
					if (guarded == 0 && allocation_entry->TagUlong == 'TnoC')
						guarded = reinterpret_cast<uintptr_t>(virtual_address);
			}
		}

		return guarded;
	}
}
