#pragma once
#include "Datatypes.h"
#include "FileManagement/path.h"
#include "Strings/String.h"

namespace LeEK
{
	//global configuration functions.
	//you can get and set settings here,
	//and also save and load config files with settings.
	//4 types of settings:
	//	* signed int
	//	* float
	//	* bool
	//	* string
	namespace Config
	{
		class SettingBase
		{
		public:
			static const U32 NAME_MAX_LEN = 64;
			char Name[NAME_MAX_LEN];
			enum SetngType { INT = 1, FLOAT = 2, BOOL = 3, STRING = 4, INVALID = 0};
			SetngType Type;
			SettingBase() : Type(INVALID) {}
		};

		class IntSetting : public SettingBase
		{
		public:
			I32 Value;
			IntSetting() : Value(0)
			{
				Type = INT;
			}
		};

		class FloatSetting : public SettingBase
		{
		public:
			F32 Value;
			FloatSetting() : Value(0)
			{
				Type = FLOAT;
			}
		};

		class BoolSetting : public SettingBase
		{
		public:
			bool Value;
			BoolSetting() : Value(false)
			{
				Type = BOOL;
			}
		};

		class StrSetting : public SettingBase
		{
		public:
			static const U32 MAX_LEN = 256;
			char Value[MAX_LEN];
			StrSetting()
			{
				Type = STRING;
			}
		};

		void ListSettings();
		void ClearSettings();

		const I32 GetIntSetting(const char* name);
		const F32 GetFloatSetting(const char* name);
		const bool GetBoolSetting(const char* name);
		const char* GetStrSetting(const char* name);

		void SetIntSetting(const char* name, I32 val);
		void SetFloatSetting(const char* name, F32 val);
		void SetBoolSetting(const char* name, bool val);
		void SetStrSetting(const char* name, const char* val);

		bool LoadCfgFile(const Path& path);
		bool SaveCfgFile(const Path& path);
	}
}