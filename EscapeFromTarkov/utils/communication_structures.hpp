#pragma once

#define base_cookie 'VAGI'

namespace OMG
{
	namespace KERNAL
	{
		enum class control_codes : uint8_t
		{
			none,
			status,
			attach_process,
			detach_process,
			get_window_thread,
			set_window_thread,
			find_module,
			key_state,
			read,
			write,
			read_injected,
		};

		struct c_base_request
		{
			uint32_t cookie;
			control_codes control_code;

			bool has_valid_cookie()
			{
				return (this->cookie == base_cookie);
			}

			c_base_request()
			{
				this->cookie = base_cookie;
				this->control_code = OMG::KERNAL::control_codes::none;
			}
		};

		struct c_status_request : c_base_request
		{
			c_status_request() : c_base_request()
			{
				this->control_code = OMG::KERNAL::control_codes::status;
			}
		};

		struct c_attach_process_request : c_base_request
		{
			uint32_t hash;
			unsigned int pid;

			c_attach_process_request(const uint32_t hash, unsigned int pid) : c_attach_process_request()
			{
				this->hash = hash;
				this->pid = pid;
			}
			c_attach_process_request() : c_base_request()
			{
				this->control_code = OMG::KERNAL::control_codes::attach_process;
				this->hash = 0;
				this->pid = 0;
			}
		};

		struct c_detach_process_request : c_base_request
		{
			c_detach_process_request() : c_base_request()
			{
				this->control_code = OMG::KERNAL::control_codes::detach_process;
			}
		};

		struct c_find_module_request : c_base_request
		{
			uint32_t hash;
			uintptr_t* output;

			c_find_module_request(const uint32_t hash,
				uintptr_t* output) : c_find_module_request()
			{
				this->hash = hash;
				this->output = output;
			}
			c_find_module_request() : c_base_request()
			{
				this->control_code = OMG::KERNAL::control_codes::find_module;
				this->hash = 0;
				this->output = nullptr;
			}
		};

		struct c_read_write_request : c_base_request
		{
			void* address;
			void* buffer;
			size_t size;

			c_read_write_request(const control_codes control_code,
				void* address,
				void* buffer,
				const size_t size) : c_read_write_request()
			{
				this->control_code = control_code;
				this->address = address;
				this->buffer = buffer;
				this->size = size;
			}

			c_read_write_request() :c_base_request()
			{
				this->address = 0;
				this->buffer = 0;
				this->size = 0;
			}
		};
	}
}