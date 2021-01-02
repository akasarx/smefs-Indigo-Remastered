#pragma once
#include "CSX.h"
#include <assert.h>

/*Idea: Steam hook and some others really don't like the current method of hooking,
and will crash.
So instead, I'm going to merge all 3 methods of hooks. Let's call it Tri-Hooking
We have the vfunc_hooks - which are trampoline hooks, then let's call em pseudo-VMT (without a module), and then of course the original VMT hooks*/

/*some useful info from unknowncheats
You should definitely checkout the official minhook documentation:
https://github.com/TsudaKageyu/minhook/wiki

This isn't related to minhook, but if you're doing hooks in an x86 program, I recommend
using Detours. No need to store a pointer for the trampoline, much more convenient imo
https://github.com/microsoft/Detours/wiki/Using-Detours

But if you still need MinHook, the documentation has all what you need
*/

/*WINAPI
VirtualProtect(
	_In_  LPVOID lpAddress,
	_In_  SIZE_T dwSize,
	_In_  DWORD flNewProtect,
	_Out_ PDWORD lpflOldProtect
) {*/

namespace detail {
	class protect_guard {
	public:
		protect_guard(PVOID base, DWORD len, int flags) { //void* base, size_t len, std::uint32_t flags)
			_base = base;
			_length = len;
			if(!VirtualProtect(base, len, flags, (PDWORD)&_old)) {
				throw std::runtime_error("Failed to protect region."); //fuck you, can't even access CSX::Log here!
			}
		}
		~protect_guard() {
			VirtualProtect(_base, _length, _old, (PDWORD)&_old);
		}
	private:
		PVOID _base; //void*
		DWORD _length; //size_t
		int _old; //std::uint32_t
	};
}

namespace CSX {
	class Hook { //VTable - vfunc_hook
	public:
		//OG Indigo VMT Hooking
		PVOID WriteVMTTable(PVOID pTablePtr, PVOID pFuncAddress, DWORD dwIndex);

		//uintptr_t* search_free_data_page(const char* module_name, const std::size_t vmt_size)
		PVOID* search_free_data_page(const char* module_name, const DWORD vmt_size);
		Hook(); //vfunc_hook();
		Hook(PVOID base); //vfunc_hook(void* base);
		~Hook(); //destructor - ~vfunc_hook();

		//bool setup(PVOID base, const char* moduleName = nullptr);
		bool InitTable(PVOID pTablePtrPtr, bool VMT = false, const char* moduleName = nullptr); //bool setup - VMT and modulename default setup incase

		template<typename T>
		void hook_index(int index, T fun) {
			assert(index >= 0 && index <= (int)vftbl_len);
			new_vftbl[index + 1] = reinterpret_cast<PVOID>(fun);
		}
		void unhook_index(int index) {
			new_vftbl[index] = old_vftbl[index];
		}
		void unhook_all() {
			try {
				if(old_vftbl != nullptr) {
					auto guard = detail::protect_guard{ class_base, sizeof(PVOID), PAGE_READWRITE };
					*(PVOID**)class_base = old_vftbl;
					old_vftbl = nullptr;
				}
			}
			catch(...) {
			}
		}

		/*Same as VTable::GetHookIndex,just with cast<index> instead*/
		template<typename T>
		T get_original(int index) {
			return (T)old_vftbl[index];
		}

		//[swap_lines]
		
		//OG Indigo VMT Hooking
		bool InitVMTTable(PVOID pTablePtrPtr); //bool setup
		void HookIndex(DWORD dwIndex, PVOID pNewAddress); //hook_index
		PVOID GetFuncAddress(DWORD dwIndex); //get_original
		PVOID GetHookIndex(DWORD dwIndex, PVOID pNewAddress); //dunno tbh
		void UnHook();
		void ReHook();

		
		//[/swap_lines]
	private:
		//[swap_lines]
		//static inline std::size_t estimate_vftbl_length(std::uintptr_t* vftbl_start)
		static inline DWORD estimate_vftbl_length(PVOID* vftbl_start);

		//OG Indigo VMT Hooking vars
		PVOID class_base; //void* class_base, but it's a double pointer, so derefrence?
		PVOID* old_vftbl; //std::uintptr_t* old_vftbl - PVOID* pPtrOldTable
		PVOID* new_vftbl; //std::uintptr_t* new_vftbl - PVOID* pPtrNewTable
		DWORD dwCountFunc; //counting the funcs along the vtable is my guess
		DWORD vftbl_len; //std::size_t vftbl_len; - dwSizeTable;  //size of vtable

		LPCVOID search_base = nullptr; //does this do the same as dwCountFunc?
		bool wasAllocated = false; //no idea
		//[/swap_lines]
	};
}