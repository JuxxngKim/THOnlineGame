#pragma once

namespace th
{
	namespace StrUtil
	{
		std::string ToLower(std::string str);
		std::string ToUTF8(const std::wstring& wstr);
		std::wstring ToWSTR(const std::string& u8str);

		int64_t ToInt64(const std::string& str);
		int64_t ToInt64(const std::wstring& str);
		int32_t ToInt32(const std::string& str);
		int32_t ToInt32(const std::wstring& str);
		int16_t ToInt16(const std::string& str);
		int16_t ToInt16(const std::wstring& str);
		int8_t ToInt8(const std::string& str);
		int8_t ToInt8(const std::wstring& str);
		float ToFloat(const std::string& str);
		float ToFloat(const std::wstring& str);
		unsigned long ToUnsignedLong(const std::string& str);

		bool IsUTF8(const std::string& str);

		void Format(std::string& ret, const char* fmt, ...);
		void Format(std::wstring& ret, const wchar_t* fmt, ...);

		std::string ToJson(const google::protobuf::Message& message);
	}
}
