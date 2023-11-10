#pragma once
#include "supermode.h"
#include "../settings.h"
#include <Windows.h>
#include <iostream>
#include <unordered_map>

namespace rwptm
{
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // Must be 0 for PML4E.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PML4E, * PPML4E;

	std::unordered_map<int, PML4E> cached_pml4;

	// populates internal pml4 cache you should call this when attached to the target pocess
	void populate_cached_pml4(uintptr_t cr3)
	{
		std::cout << "Copying PML4 of target process to local cache\n";
		if (cr3 == 0)
		{
			std::cout << "not attached\n";
			return;
		}

		// traverse all 512 entries in the pml4
		for (int i = 0; i < 512; i++)
		{
			PML4E pml4e;
			supermode_comm::read_physical_memory((cr3 + i * sizeof(uintptr_t)), sizeof(PML4E), (uint64_t*)&pml4e);

			// page backs physical memory
			if (pml4e.Present)
			{
				cached_pml4[i] = pml4e;
			}
		}

		std::cout << "Stored " << cached_pml4.size() << " PML4 entries\n";
	}

	// get free local pml4e to create target pml4e in
	void setup_pml4_table(uintptr_t cr3)
	{
		std::cout << "Copying PML4 of target process to local pml4\n";
		if (cr3 == 0)
		{
			std::cout << "not attached\n";
			return;
		}

		// get all free pml4 of local process we will use those fuckers later
		for (int i = 0; i < 512; i++)
		{
			PML4E pml4e_attacker;
			supermode_comm::read_physical_memory((cr3 + i * sizeof(uintptr_t)), sizeof(PML4E), (uint64_t*)&pml4e_attacker);

			// page is free for our use
			if (!pml4e_attacker.Present)
			{
				supermode_comm::free_pml4s.push_back(i);
			}
		}
		std::cout << "stored " << supermode_comm::free_pml4s.size() << " free entries!\n";
	}

	uintptr_t swap_pml4e_from_va(uint64_t new_pml4, uintptr_t original_va)
	{
		const uint64_t originalAddress = original_va;
		uint64_t newAddress;
		const uint64_t pml4Mask = 0x0000FF8000000000;
		newAddress = originalAddress & ~pml4Mask;
		newAddress |= new_pml4 << 39;
		return newAddress;
	}

	// returns the index of the created pml4
	uint64_t create_pml4e(ULONG64 pml4e)
	{
		return supermode_comm::create_pml4(pml4e);
	}
	
	// destroys the pml4 at specified index
	void destroy_pml4e(uint64_t pml4e_ind)
	{
		supermode_comm::destroy_pml4e(pml4e_ind);
	}

	uintptr_t target_base, target_cr3, target_eproc, local_base, local_cr3, local_eproc;

	// -------- USER FUNCS ---------

	bool init(const char* target_application, const char* local_application)
	{
		srand(time(0));

		supermode_comm::load();

		target_base = supermode::attach(target_application, &target_cr3, &target_eproc);
		if (!target_base)
			return false;

		if (EACCR3)
		{
			std::cout << "populating using bruteforced DTB: " << std::hex << supermode_comm::target_cr3 << std::endl;
			rwptm::populate_cached_pml4(supermode_comm::target_cr3);
		}
		else
		{
			std::cout << "populating using EPROCESS DTB: " << std::hex << target_cr3 << std::endl;
			rwptm::populate_cached_pml4(target_cr3);
		}

		local_base = supermode::attach(local_application, &local_cr3, &local_eproc);
		std::cout << std::hex << local_eproc << std::endl;
		rwptm::setup_pml4_table(local_cr3);

		return true;
	}

	template <typename T>
	T read_virtual_memory(uintptr_t address)
	{
		T out{};

		unsigned short orig_pml4e_ind = (unsigned short)((address >> 39) & 0x1FF);

		// reading invalid addr
		if (cached_pml4.find(orig_pml4e_ind) == cached_pml4.end())
			return out;

		uint64_t created_pml4e_ind = create_pml4e(cached_pml4[orig_pml4e_ind].Value);
		uintptr_t fixed_addr = swap_pml4e_from_va(created_pml4e_ind, address);

		MemoryFence();

		memcpy(&out, (void*)fixed_addr, sizeof(T));

		destroy_pml4e(created_pml4e_ind);

		MemoryFence();

		return out;
	}

	template <typename T>
	void write_virtual_memory(uintptr_t address, T val)
	{
		unsigned short orig_pml4e_ind = (unsigned short)((address >> 39) & 0x1FF);

		// writing invalid addr
		if (cached_pml4.find(orig_pml4e_ind) == cached_pml4.end())
			return;

		uint64_t created_pml4e_ind = create_pml4e(cached_pml4[orig_pml4e_ind].Value);
		uintptr_t fixed_addr = swap_pml4e_from_va(created_pml4e_ind, address);
		MemoryFence();

		if (IsBadWritePtr((void*)fixed_addr, sizeof(T)))
			return;

		memcpy((void*)fixed_addr, &val, sizeof(T));

		destroy_pml4e(created_pml4e_ind);

		MemoryFence();

		return;
	}
}