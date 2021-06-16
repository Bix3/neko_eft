#pragma once
#include <windows.h>
#include <winternl.h>
#include <cstdint>
#include <TlHelp32.h>
#include <codecvt>
#include <vector>

#include "../utils/communication_structures.hpp"

inline unsigned int pid;

namespace OMG
{
	namespace KERNAL
	{
		template<typename ... A>
		inline NTSTATUS send(const A ... arguments)
		{
			static auto control_function = PVOID(GetProcAddress(LoadLibraryA("win32u.dll"), xorget("NtGdiDdDDIQueryRemoteVidPnSourceFromGdiDisplayName")));

			using FunctionFn = NTSTATUS(__stdcall*)(A...);
			const auto Function = reinterpret_cast<FunctionFn>(control_function);

			return NT_SUCCESS(Function(arguments...));
		}

		inline bool status()
		{
			auto request = OMG::KERNAL::c_status_request();
			return OMG::KERNAL::send(&request, sizeof(request));
		}

		inline DWORD get_pid(const char* proc_name)
		{
			PROCESSENTRY32 proc_info;
			proc_info.dwSize = sizeof(proc_info);

			HANDLE proc_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (proc_snapshot == INVALID_HANDLE_VALUE)
				return NULL;

			Process32First(proc_snapshot, &proc_info);
			if (!strcmp(proc_info.szExeFile, proc_name))
			{
				CloseHandle(proc_snapshot);
				return proc_info.th32ProcessID;
			}

			while (Process32Next(proc_snapshot, &proc_info))
			{
				if (!strcmp(proc_info.szExeFile, proc_name))
				{
					CloseHandle(proc_snapshot);
					return proc_info.th32ProcessID;
				}
			}

			CloseHandle(proc_snapshot);
			return NULL;
		}

		inline bool attach(const uint32_t hash, unsigned int pid)
		{
			auto request = OMG::KERNAL::c_attach_process_request(hash, pid);
			return OMG::KERNAL::send(&request, sizeof(request));
		}

		inline bool detach()
		{
			auto request = OMG::KERNAL::c_detach_process_request();
			return OMG::KERNAL::send(&request, sizeof(request));
		}

		inline uintptr_t find(const uint32_t hash)
		{
			auto output = uintptr_t{};
			auto request = OMG::KERNAL::c_find_module_request(hash, &output);
			OMG::KERNAL::send(&request, sizeof(request));
			return output;
		}

		template <typename t>
		inline t read(const uint64_t address)
		{
			auto buffer = t();

			if (address < 0xffffff)
				return buffer;
			if (address > 0x7fffffff0000)
				return buffer;

			auto request = OMG::KERNAL::c_read_write_request(OMG::KERNAL::control_codes::read,
				reinterpret_cast<void*>(address),
				&buffer,
				sizeof(t));

			OMG::KERNAL::send(&request, sizeof(request));
			return buffer;
		}

		template <class T>
		inline T read_chain(uintptr_t base, const std::vector<T>& offsets) {
			T result = OMG::KERNAL::read<T>(base + offsets.at(0));
			for (int i = 1; i < offsets.size(); i++) {
				result = OMG::KERNAL::read<T>(result + offsets.at(i));
			}
			return result;
		}

		template <typename t>
		inline bool read(const uintptr_t address, const t buffer, const size_t size)
		{
			if (address < 0xffffff)
				return false;
			if (address > 0x7fffffff0000)
				return false;

			auto request = OMG::KERNAL::c_read_write_request(OMG::KERNAL::control_codes::read,
				reinterpret_cast<void*>(address),
				reinterpret_cast<void*>(buffer),
				size);

			return OMG::KERNAL::send(&request, sizeof(request));
		}

		template <typename t>
		inline bool write(const uintptr_t address, t buffer)
		{
			auto request = OMG::KERNAL::c_read_write_request(OMG::KERNAL::control_codes::write,
				reinterpret_cast<void*>(address),
				reinterpret_cast<void*>(&buffer),
				sizeof(t));

			return OMG::KERNAL::send(&request, sizeof(request));
		}

		template <typename t>
		inline bool write(const uintptr_t address, const t buffer, const size_t size)
		{
			auto request = OMG::KERNAL::c_read_write_request(OMG::KERNAL::control_codes::write,
				reinterpret_cast<void*>(address),
				reinterpret_cast<void*>(buffer),
				size);

			return OMG::KERNAL::send(&request, sizeof(request));
		}

		inline void init()
		{
			// lol? - chloe
			LoadLibraryA("user32.dll");
		}

		inline std::string utf16_to_utf8(std::u16string utf16_string)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
			auto p = reinterpret_cast<const int16_t*>(utf16_string.data());
			return convert.to_bytes(p, p + utf16_string.size());
		}

		inline std::string read_ascii(const uintptr_t address, const size_t size)
		{
			std::unique_ptr<char[]> buffer(new char[size]);
			read(address, (uintptr_t)&buffer, size);
			if (!buffer.get())
				return std::string();
			return std::string(buffer.get());
		}

		inline std::string read_unicode(const uintptr_t address)
		{
			//crashes
			char16_t wcharTemp[64] = { '\0' };

			const auto stringLength = read<int32_t>(address + 0x10);

			read(address, (uintptr_t)&wcharTemp, stringLength * 2);
			if (!wcharTemp)
				return std::string("");

			std::string u8_conv = utf16_to_utf8(wcharTemp);

			return u8_conv;
		}
	}
}