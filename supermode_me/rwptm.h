#pragma once
#include "supermode.h"
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
	std::unordered_map<int, int> translation_table;

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

	// encrypt pml4e pfn, copy all present pml4e into local pml4 and setup translation table
	void setup_pml4_table(uintptr_t cr3)
	{
		std::cout << "Copying PML4 of target process to local pml4\n";
		if (cr3 == 0)
		{
			std::cout << "not attached\n";
			return;
		}

		// get all valid pml4 of local process
		std::vector<uintptr_t> local_pml4s;
		for (auto& pml4e_map : cached_pml4)
		{
			PML4E pml4e_target = pml4e_map.second;

			for (int i = 0; i < 512; i++)
			{
				PML4E pml4e_attacker;
				supermode_comm::read_physical_memory((cr3 + i * sizeof(uintptr_t)), sizeof(PML4E), (uint64_t*)&pml4e_attacker);

				// page is free for our use
				if (pml4e_attacker.Present)
				{
					std::cout << "found valid pml4 locally\n";
					local_pml4s.push_back(pml4e_attacker.PageFrameNumber);
					break;
				}
			}
		}

		// populate local pml4 with target pml4 entries
		for (auto& pml4e_map : cached_pml4)
		{
			PML4E pml4e_target = pml4e_map.second;

			for (int i = 0; i < 512; i++)
			{
				PML4E pml4e_attacker;
				supermode_comm::read_physical_memory((cr3 + i * sizeof(uintptr_t)), sizeof(PML4E), (uint64_t*)&pml4e_attacker);

				// page is free for our use
				if (!pml4e_attacker.Present)
				{
					translation_table[pml4e_map.first] = i;

					std::cout << i << ": changing pml4e pfn from " << pml4e_target.PageFrameNumber;
					uint64_t orig_pfn = pml4e_target.PageFrameNumber;
					int pfnidx = (rand() + i) % local_pml4s.size();
					pml4e_target.PageFrameNumber = local_pml4s[pfnidx];
					supermode_comm::decryption_table[pml4e_target.PageFrameNumber + i] = orig_pfn;
					std::cout << " to " << pml4e_target.PageFrameNumber << " stored for decryption: " << supermode_comm::decryption_table[pml4e_target.PageFrameNumber + i] << " index in decryption table: " << pml4e_target.PageFrameNumber + pml4e_map.first << std::endl;

					supermode_comm::write_physical_memory(cr3 + i * sizeof(uintptr_t), sizeof(PML4E), (uint64_t*) & pml4e_target);
					break;
				}
			}
		}
	}

	uintptr_t correct_virtual_address(uintptr_t virtual_address)
	{
		const uint64_t originalAddress = virtual_address;
		uint64_t newAddress;

		unsigned short originalPML4 = (unsigned short)((originalAddress >> 39) & 0x1FF);

		const uint64_t pml4Mask = 0x0000FF8000000000;

		newAddress = originalAddress & ~pml4Mask;

		newAddress |= (uint64_t)(translation_table[originalPML4]) << 39;

		return newAddress;
	}

	uint64_t dec_pml4e_pfn(uintptr_t virtual_address)
	{
		const uint64_t originalAddress = virtual_address;
		unsigned short originalPML4 = (unsigned short)((originalAddress >> 39) & 0x1FF);

		return supermode_comm::dec_pml4e(originalPML4);
	}

	void enc_pml4e_pfn(uintptr_t virtual_address, uint64_t fake_pfn)
	{
		const uint64_t originalAddress = virtual_address;
		unsigned short originalPML4 = (unsigned short)((originalAddress >> 39) & 0x1FF);

		supermode_comm::enc_pml4e(originalPML4, fake_pfn);
	}

	// -------- USER FUNCS ---------
	uintptr_t target_base = 0;

	bool init(const char* target_application, const char* local_application)
	{
		supermode_comm::load();

		uintptr_t target_cr3, target_eproc;
		target_base = supermode::attach(target_application, &target_cr3, &target_eproc);
		if (!target_base)
			return false;

		std::cout << "populating using: " << std::hex << supermode_comm::target_cr3 << std::endl;
		rwptm::populate_cached_pml4(supermode_comm::target_cr3);

		uintptr_t attacker_cr3, attacker_eproc;
		uintptr_t attacker_base = supermode::attach(local_application, &attacker_cr3, &attacker_eproc);

		std::cout << std::hex << attacker_eproc << std::endl;

		rwptm::setup_pml4_table(attacker_cr3);

		return true;
	}

	template <typename T>
	T read_virtual_memory(uintptr_t address)
	{
		T out;
		uintptr_t fixed_addr = correct_virtual_address(address);
		uint64_t fake_pfn = dec_pml4e_pfn(fixed_addr);

		MemoryFence();

		memcpy(&out, (void*)fixed_addr, sizeof(T));
		enc_pml4e_pfn(fixed_addr, fake_pfn);

		MemoryFence();

		return out;
	}

	template <typename T>
	void write_virtual_memory(uintptr_t address, T val)
	{
		uintptr_t fixed_addr = correct_virtual_address(address);
		uint64_t fake_pfn = dec_pml4e_pfn(fixed_addr);

		MemoryFence();

		memcpy((void*)fixed_addr, (void*)&val, sizeof(T));		
		enc_pml4e_pfn(fixed_addr, fake_pfn);

		MemoryFence();

		return;
	}
}