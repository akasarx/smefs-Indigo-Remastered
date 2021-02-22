#pragma once
#include "CSX.h"
#include <assert.h>

/*Idea: Steam hook and some others really don't like vfunc_hooking,
and will crash.
So instead, I'm going to merge all 3 methods of hooks. Let's call it Tri-Hooking
We have the vfunc_hooks - which are trampoline hooks, then the other, let's call em pseudo-VMT (without a module), and then of course the original VMT hooks*/

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
		protect_guard(void* base, size_t len, std::uint32_t flags) {
			_base = base;
			_length = len;
			if(!VirtualProtect(base, len, flags, (PDWORD)&_old)) { //why can't I fucking CSX::Log here??????
/*#if ENABLE_DEBUG_FILE == 1 
				CSX::Log::Add("\n[CSX::Hook - protect_guard failed to protect region.]\n");
#endif*/
				//throw std::runtime_error("Failed to protect region.");
			}
		}
		~protect_guard() {
			VirtualProtect(_base, _length, _old, (PDWORD)&_old);
		}

	private:
		void*         _base;
		size_t        _length;
		std::uint32_t _old;
	};
}

//Visual Studio: 'CSX' is not a class or namespace name (compiling source file CSX_)
namespace CSX {
	class Hook { //VTable - vfunc_hook
	public:
		//vfunc_hook
		uintptr_t * search_free_data_page(const char* moduleName, const std::size_t vmt_size);
		Hook(); //vfunc_hook(); - VTable()
		Hook(void* base); //vfunc_hook(void* base);
		~Hook(); //destructor - ~vfunc_hook();

		//bool setup(void* base, const char* moduleName = nullptr);
		bool InitTable(void* base = nullptr, bool VMT = false, const char* moduleName = nullptr);

		//just a cast, really not fun. - noodled
		template<typename T>
		void hook_index(int index, T fun) {
			assert(index >= 0 && index <= (int)vftbl_len);
			new_vftbl[index + 1] = reinterpret_cast<std::uintptr_t>(fun);
		}
		void unhook_index(int index) {
			new_vftbl[index] = old_vftbl[index];
		}
		void unhook_all() {
			try {
				if(old_vftbl != nullptr) {
					auto guard = detail::protect_guard{ class_base, sizeof(std::uintptr_t), PAGE_READWRITE };
					*(std::uintptr_t**)class_base = old_vftbl;
					old_vftbl = nullptr;
				}
			}
			catch(...) { //Literally in the namespace of CSX...... but yet cannot log
/*#if ENABLE_DEBUG_FILE == 1
				Log::Add("\n[CSX::Hook - unhook_all() fail!]\n");
#endif*/
			}
		}

		/*Same as VTable::GetHookIndex,just with cast<index> instead*/
		template<typename T>
		T get_original(int index) {
			return (T)old_vftbl[index];
		}

		//[swap_lines]
		
		//Indigo VMT Hooking
		PVOID VMT_WriteTable(PVOID pTablePtr, PVOID pFuncAddress, DWORD dwIndex);
		bool VMT_InitTable(PVOID pTablePtrPtr); //bool setup
		void VMT_HookIndex(DWORD dwIndex, PVOID pNewAddress); //hook_index
		PVOID VMT_GetHookIndex(DWORD dwIndex, PVOID pNewAddress); //dunno tbh
		PVOID VMT_GetFuncAddress(DWORD dwIndex); //same as get_original
		void VMT_UnHook();
		void VMT_ReHook();

		//Test
		void New_UnHook();
		void New_ReHook();
		
		//[/swap_lines]
	private:
		//[swap_lines]
		static inline std::size_t estimate_vftbl_length(std::uintptr_t* vftbl_start);

		/*what a pain, Indigo uses PVOID* while vfunc_hooks use only PVOID 
		cba bothering to keep 1 var so I'll keep them seperate.
		change to std::uintptr_t* tho, it's pretty much just PVOID* but larger*/

		std::uintptr_t* pclass_base; //Indigo - PVOID* pPtrPtrTable;
		void* class_base; //class_base
		std::size_t vftbl_len; //Indigo - DWORD dwSizeTable; ?
		std::uintptr_t* new_vftbl; //Indigo - PVOID* pPtrNewTable;
		std::uintptr_t* old_vftbl; //Indigo - PVOID* pPtrOldTable;
		
		//should be as it sounds
		LPCVOID search_base = nullptr; //does this do the same as dwCountFunc?
		bool wasAllocated = false; //hooks allocated check, mostly useless imo - maybe should remove this and the check from .cpp

		//Indigo
		DWORD dwCountFunc; //counting the funcs along the vtable is my guess

		//[/swap_lines]
	};
}