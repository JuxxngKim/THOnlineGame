#include "CommonPCH.h"
#include "StrUtil.h"
#include <codecvt>
#include <google/protobuf/util/json_util.h>

namespace th
{
	namespace StrUtil
	{
		std::string ToLower(std::string str)
		{
			int(*tl)(int) = std::tolower;
			std::ranges::transform(str, str.begin(), tl);
			return str;
		}

		std::string ToUTF8(const std::wstring& wstr)
		{
			if (wstr.empty()) return {};
			int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
			std::string result(size, 0);
			WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), size, nullptr, nullptr);
			return result;
		}

		std::wstring ToWSTR(const std::string& u8str)
		{
			if (u8str.empty()) return {};
			int size = MultiByteToWideChar(CP_UTF8, 0, u8str.data(), (int)u8str.size(), nullptr, 0);
			std::wstring result(size, 0);
			MultiByteToWideChar(CP_UTF8, 0, u8str.data(), (int)u8str.size(), result.data(), size);
			return result;
		}

		int64_t ToInt64(const std::string& str)
		{
			try
			{
				return std::stoll(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int64_t ToInt64(const std::wstring& str)
		{
			try
			{
				return std::stoll(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int32_t ToInt32(const std::string& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int32_t ToInt32(const std::wstring& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int16_t ToInt16(const std::string& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int16_t ToInt16(const std::wstring& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int8_t ToInt8(const std::string& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		int8_t ToInt8(const std::wstring& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		float ToFloat(const std::string& str)
		{
			try
			{
				return std::stof(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		float ToFloat(const std::wstring& str)
		{
			try
			{
				return std::stof(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		unsigned long ToUnsignedLong(const std::string& str)
		{
			try
			{
				return std::stoul(str);
			}
			catch (...)
			{
				return 0;
			}
		}

		bool IsUTF8(const std::string& str)
		{
			size_t idx = 0;
			size_t total = str.size();
			if (total <= 0)
			{
				return true;
			}

			bool isUTF8 = true;

			while (idx < total && isUTF8)
			{
				int supplemental_bytes = 0;

				if ((str[idx] & 0b11111000) == 0b11110000) {
					++idx;
					supplemental_bytes = 3;
				}
				else if ((str[idx] & 0b11110000) == 0b11100000) {
					++idx;
					supplemental_bytes = 2;
				}
				else if ((str[idx] & 0b11100000) == 0b11000000) {
					++idx;
					supplemental_bytes = 1;
				}
				else if ((str[idx] & 0b10000000) == 0b00000000) {
					++idx;
				}
				else {
					isUTF8 = false;
				}

				while (isUTF8 && supplemental_bytes--)
				{
					if (idx >= total)	isUTF8 = false;
					else if ((str[idx++] & 0xC0) != 0x80)	isUTF8 = false;
				}
			}

			return isUTF8;
		}

		void Format(std::string& ret, const char* fmt, ...)
		{
			if (fmt == nullptr) return;

			va_list arg;

			va_start(arg, fmt);

			int next = _vscprintf(fmt, arg);
			if (next > 0)
			{
				//_vsnprintf_s 로 가변인자를 받아올때 맨뒤에 널문자가 포함되어서 실제 문자열 길이보다 1 길다
				//때문에 사이즈를 1 늘려서 잡는다
				++next;
				ret.resize(next);

				//복사
				_vsnprintf_s((char*)ret.c_str(), next, _TRUNCATE, fmt, arg);

				//널문자 제거
				ret.resize(ret.size() - 1);
			}

			va_end(arg);
		}

		void Format(std::wstring& ret, const wchar_t* fmt, ...)
		{
			if (fmt == nullptr) return;

			va_list arg;

			va_start(arg, fmt);

			int next = _vscwprintf(fmt, arg);
			if (next > 0)
			{
				//_vsnwprintf_s 로 가변인자를 받아올때 맨뒤에 널문자가 포함되어서 실제 문자열 길이보다 1 길다
				//때문에 사이즈를 1 늘려서 잡는다
				++next;
				ret.resize(next);

				//복사
				_vsnwprintf_s((wchar_t*)ret.c_str(), next, _TRUNCATE, fmt, arg);

				//널문자 제거
				ret.resize(ret.size() - 1);
			}

			va_end(arg);
		}

		std::string ToJson(const google::protobuf::Message& message)
		{
			if (message.GetDescriptor() == nullptr)
			{
				return message.GetTypeName();
			}

			std::string str;
			google::protobuf::util::JsonPrintOptions op;
			//op.add_whitespace = true;
			op.always_print_primitive_fields = true;
			google::protobuf::util::MessageToJsonString(message, &str, op);
			return str;
		}
	}
}