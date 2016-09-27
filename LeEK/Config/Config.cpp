#include "Config.h"
#include "Constants/AllocTypes.h"
#include "Logging/Log.h"
#include "Hashing/HashMap.h"
#include "Memory/Handle.h"
#include "Libraries/TinyXML2/tinyxml2.h"
#include "FileManagement/Filesystem.h"

using namespace LeEK;

typedef TypedHandle<Config::SettingBase> SettingHandle;
typedef HashMap<SettingHandle> SettingMap;

SettingMap settingMap = SettingMap();

//XML tag constants
const char* ROOT_NAME = "Config";
const char* SETTING_NAME = "Setting";
const char* NAME_ATTR_NAME = "name";
const char* TYPE_ATTR_NAME = "type";
const char* VALUE_ATTR_NAME = "value";

void Config::ListSettings()
{
	LogD("Listing settings:");
	for(SettingMap::iterator it = settingMap.begin(); it != settingMap.end(); ++it)
	{
		LogD(String("Setting ") + it->first + ":");
		LogD(String("\tName: ") + it->second->Name);
		LogD(String("\tType: ") + it->second->Type);
	}
}

void Config::ClearSettings()
{
	LogD("Clearing all settings!");
	for(SettingMap::iterator it = settingMap.begin(); it != settingMap.end(); ++it)
	{
		HandleMgr::DeleteHandle(it->second);
	}
	settingMap.clear();
}

void initSetting(const char* name, Config::SettingBase* setting)
{
	strcpy_s(setting->Name, Config::SettingBase::NAME_MAX_LEN, name);
	settingMap[name] = HandleMgr::RegisterPtr(setting);
}

void reportTiXMLErr(I32 errCode, const String filename)
{
	if(errCode && errCode != tinyxml2::XML_ERROR_EMPTY_DOCUMENT)
	{
		//TODO: could maybe elaborate on this, but just note failure for now
		LogW(String("Failed to parse config file") + filename + "!");
		LogW(String("Error code: ") + errCode);
	}
}

void setValueElem(Config::SettingBase* setting,
				  tinyxml2::XMLText* valText)
{
	using namespace Config;
	switch(setting->Type)
	{
	case SettingBase::INT:
		valText->SetValue(StrFromVal(((IntSetting*)setting)->Value).c_str());
		break;
	case SettingBase::FLOAT:
		valText->SetValue(StrFromVal(((FloatSetting*)setting)->Value).c_str());
		break;
	case SettingBase::BOOL:
		valText->SetValue(StrFromVal(((BoolSetting*)setting)->Value).c_str());
		break;
	case SettingBase::STRING:
		valText->SetValue(((StrSetting*)setting)->Value);
		break;
	default:
		LogW(String("Couldn't save value for setting ") + setting->Name + "!");
		break;
	}
}

void attachNewSettingElem(Config::SettingBase* setting,
					  tinyxml2::XMLDocument* doc,
					  tinyxml2::XMLElement* settingElem)
{
	using namespace Config;
	tinyxml2::XMLElement* nameElem = doc->NewElement(NAME_ATTR_NAME);
	tinyxml2::XMLText* nameText = doc->NewText(setting->Name);
	nameElem->InsertEndChild(nameText);
	settingElem->InsertEndChild(nameElem);

	tinyxml2::XMLElement* typeElem = doc->NewElement(TYPE_ATTR_NAME);
	tinyxml2::XMLText* typeText = doc->NewText(StrFromVal(setting->Type).c_str());
	typeElem->InsertEndChild(typeText);
	settingElem->InsertEndChild(typeElem);

	tinyxml2::XMLElement* valElem = doc->NewElement(VALUE_ATTR_NAME);
	tinyxml2::XMLText* valText = doc->NewText("");

	setValueElem(setting, valText);

	valElem->InsertEndChild(valText);
	settingElem->InsertEndChild(valElem);
}

const I32 Config::GetIntSetting(const char* name)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		//setting doesn't exist, init a new one
		IntSetting* res = CustomNew<IntSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		return res->Value;
	}
	L_ASSERT(setting->second->Type == SettingBase::INT);
	return ((IntSetting*)setting->second.Ptr())->Value;
}
const F32 Config::GetFloatSetting(const char* name)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		//setting doesn't exist, init a new one
		FloatSetting* res = CustomNew<FloatSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		return res->Value;
	}
	L_ASSERT(setting->second->Type == SettingBase::FLOAT);
	return ((FloatSetting*)setting->second.Ptr())->Value;
}
const bool Config::GetBoolSetting(const char* name)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		//setting doesn't exist, init a new one
		BoolSetting* res = CustomNew<BoolSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		return res->Value;
	}
	L_ASSERT(setting->second->Type == SettingBase::BOOL);
	return ((BoolSetting*)setting->second.Ptr())->Value;
}
const char* Config::GetStrSetting(const char* name)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		//setting doesn't exist, init a new one
		StrSetting* res = CustomNew<StrSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		return res->Value;
	}
	L_ASSERT(setting->second->Type == SettingBase::STRING);
	return ((StrSetting*)setting->second.Ptr())->Value;
}

void Config::SetIntSetting(const char* name, I32 val)
{
	//if it doesn't exist, make it
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		IntSetting* res = CustomNew<IntSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		res->Value = val;
		return;
	}
	//otherwise, search
	SettingBase* res = setting->second.Ptr();
	if(res->Type != SettingBase::INT)
	{
		LogW(String("Invalid value passed to setting \"") + name + "\"!");
		return;
	}
	((IntSetting*)res)->Value = val;
}
void Config::SetFloatSetting(const char* name, F32 val)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		FloatSetting* res = CustomNew<FloatSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		res->Value = val;
		return;
	}
	//otherwise, search
	SettingBase* res = setting->second.Ptr();
	if(res->Type != SettingBase::FLOAT)
	{
		LogW(String("Invalid value passed to setting \"") + name + "\"!");
		return;
	}
	((FloatSetting*)res)->Value = val;
}
void Config::SetBoolSetting(const char* name, bool val)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		BoolSetting* res = CustomNew<BoolSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		res->Value = val;
		return;
	}
	//otherwise, search
	SettingBase* res = setting->second.Ptr();
	if(res->Type != SettingBase::BOOL)
	{
		LogW(String("Invalid value passed to setting \"") + name + "\"!");
		return;
	}
	((BoolSetting*)res)->Value = val;
}
void Config::SetStrSetting(const char* name, const char* val)
{
	SettingMap::iterator setting = settingMap.find(name);
	if(setting == settingMap.end())
	{
		StrSetting* res = CustomNew<StrSetting>(CONFIG_ALLOC, "SettingInit");
		initSetting(name, res);
		//strs are different - COPY the new value.
		strcpy_s(res->Value, StrSetting::MAX_LEN, val);
		return;
	}
	//otherwise, search
	SettingBase* res = setting->second.Ptr();
	if(res->Type != SettingBase::STRING)
	{
		LogW(String("Invalid value passed to setting \"") + name + "\"!");
		return;
	}
	strcpy_s(((StrSetting*)res)->Value, StrSetting::MAX_LEN, val);
}

/**
*	Config format:
*	Each setting consists of a <setting> tag, 
*	which has a "name" attrib which is always a string (the setting's name),
*	a "type" attrib which is always an int (just the setting's enum),
*	and finally a "value" attrib which better match the type!
**/

bool Config::LoadCfgFile(const Path& path)
{
	//let's make sure the file exists
	if(Filesystem::Exists(path))
	{
		//if it does, parse and build the settings
		DataStream* file = Filesystem::OpenFileText(path);
		char* fileBuf = file->ReadAll();
		tinyxml2::XMLDocument* doc = CustomNew<tinyxml2::XMLDocument>(CONFIG_ALLOC, "XMLExport");
		doc->Parse(fileBuf);
		//get rid of the allocs ASAP
		CustomArrayDelete(fileBuf);
		//make sure doc's valid
		reportTiXMLErr(doc->ErrorID(), path.GetBaseName());
		//and make sure the doc has data
		//get a handle so we don't have to do null checks
		tinyxml2::XMLHandle docHnd(doc);
		tinyxml2::XMLElement* root = docHnd.FirstChildElement().ToElement();
		if(!root)
		{
			LogW(String("Config file") + path.GetBaseName() + "has no root element!");
			return false;
		}
		//otherwise, we're ready; get parsing
		for(tinyxml2::XMLElement* elem = root->FirstChildElement(SETTING_NAME); elem != NULL; elem = elem->NextSiblingElement(SETTING_NAME))
		{
			tinyxml2::XMLElement* nameElem = elem->FirstChildElement(NAME_ATTR_NAME);//Attribute(NAME_ATTR_NAME);
			//get the name
			if(!nameElem)
			{
				LogW("Couldn't get the name of a setting!");
				continue;
			}
			const char* name = nameElem->FirstChild()->ToText()->Value();
			I32 type = 0;// = elem->IntAttribute(TYPE_ATTR_NAME);
			tinyxml2::XMLElement* typeElem = elem->FirstChildElement(TYPE_ATTR_NAME);
			//then the type
			if(!typeElem)//elem->QueryIntAttribute(TYPE_ATTR_NAME, &type) != tinyxml2::XML_NO_ERROR)
			{
				LogW(String("Couldn't find type for setting ") + name + "!");
				continue;
			}
			if(typeElem->QueryIntText(&type) != tinyxml2::XML_NO_ERROR)
			{
				LogW(String("Couldn't parse type for setting ") + name + "!");
				continue;
			}
			//then the value
			tinyxml2::XMLElement* valElem = elem->FirstChildElement(VALUE_ATTR_NAME);
			if(!valElem)
			{
				LogW(String("Couldn't find value for setting ") + name + "!");
				continue;
			}
			//dumb to have blocks in the switch,
			//but need to keep final val in case's scope
			switch(type)
			{
			case SettingBase::INT:
				{
					I32 val;
					if(valElem->QueryIntText(&val) != tinyxml2::XML_NO_ERROR)
					{
						LogW(String("Couldn't get value for setting ") + name + "!");
						break;
					}
					SetIntSetting(name, val);
					break;
				}
			case SettingBase::FLOAT:
				{
					F32 val;
					if(valElem->QueryFloatText(&val) != tinyxml2::XML_NO_ERROR)
					{
						LogW(String("Couldn't get value for setting ") + name + "!");
						break;
					}
					SetFloatSetting(name, val);
					break;
				}
			case SettingBase::BOOL:
				{
					bool val;
					if(valElem->QueryBoolText(&val) != tinyxml2::XML_NO_ERROR)
					{
						LogW(String("Couldn't get value for setting ") + name + "!");
						break;
					}
					SetBoolSetting(name, val);
					break;
				}
			case SettingBase::STRING:
				{
					const char* val = valElem->GetText();//elem->Attribute(VALUE_ATTR_NAME);
					if(!val)
					{
						LogW(String("Couldn't get value for setting ") + name + "!");
						break;
					}
					SetStrSetting(name, val);
					break;
				}
			default:
				{
					LogW(String("Couldn't set value for setting ") + name + "!");
					break;
				}
			}
		}
		CustomDelete(doc);
		Filesystem::CloseFile(file);
		//and we're done!
		LogD(String("Loaded settings from file ") + path.GetBaseName());
		return true;
	}
	LogW(String("Could not open config file ") + path.GetBaseName() + "!");
	return false;
}

tinyxml2::XMLElement* findSettingInFile(tinyxml2::XMLElement* first, const char* stngName)
{
	for(tinyxml2::XMLElement* elem = first; elem != NULL; elem = elem->NextSiblingElement(SETTING_NAME))
	{
		//const char* name = elem->Attribute(NAME_ATTR_NAME, stngName);
		tinyxml2::XMLElement* nameElem = elem->FirstChildElement(NAME_ATTR_NAME);
		//get the name
		if(nameElem)
		{
			if(strcmp(nameElem->GetText(), stngName) == 0)
			{
				return elem;
			}
		}
	}
	return NULL;
}

bool Config::SaveCfgFile(const Path& path)
{
	//behavior depends on the file's existence
	//if it DOESN'T exist (or the file's missing a root node implying the same), 
	//we can just build from the setting list and save the file.
	//parse and build the settings
	LogV(String("Saving config file ") + path.GetBaseName());
	tinyxml2::XMLDocument* doc = CustomNew<tinyxml2::XMLDocument>(CONFIG_ALLOC, "XMLExport");
	DataStream* file = Filesystem::OpenFileText(path);
	char* fileBuf = file->ReadAll();
	doc->Parse(fileBuf);
	reportTiXMLErr(doc->ErrorID(), path.GetBaseName());
	//get rid of the buffer, but do NOT close the file yet, we'll probably write something to it.
	CustomArrayDelete(fileBuf);
	tinyxml2::XMLHandle docHnd(doc);
	tinyxml2::XMLElement* possRoot = docHnd.FirstChildElement().ToElement();
	if(!possRoot)
	{
		#pragma region New File
		tinyxml2::XMLPrinter printer;
		tinyxml2::XMLElement* root = doc->NewElement(ROOT_NAME);
		//root->SetName(ROOT_NAME);
		doc->InsertEndChild(root);
		HashMap<tinyxml2::XMLElement*> elemMap;
		for(SettingMap::iterator i = settingMap.begin(); i != settingMap.end(); ++i)
		{
			tinyxml2::XMLElement* elem = doc->NewElement(SETTING_NAME);
			elemMap[i->first] = elem;
			SettingBase* setting = i->second.Ptr();
			L_ASSERT(setting);

			//and attach the element
			attachNewSettingElem(setting, doc, elem);
			root->InsertEndChild(elem);
		}
		//now save the file
		//need to flush the document to the printer first
		doc->Print(&printer);
		file->Write(printer.CStr());
		Filesystem::CloseFile(file);
		//really iffy...
		CustomDelete(doc);
		/*
		//free the hashmap's pointers just to be sure
		for(HashMap<tinyxml2::XMLElement*>::iterator i = elemMap.begin(); i != elemMap.end(); ++i)
		{
			CustomDelete(i->second);
		}
		CustomDelete(root);*/
		return true;
		#pragma endregion
	}
	//if it DOES, we have a much more complicated situation:
	//	* for each setting, search the file for the setting.
	//		* if it exists, write new value iff types match
	//		* else write the setting
	else
	{		
		#pragma region Existing File
		tinyxml2::XMLPrinter printer;
		possRoot->SetName(ROOT_NAME);
		HashMap<tinyxml2::XMLElement*> elemMap;
		for(SettingMap::iterator i = settingMap.begin(); i != settingMap.end(); ++i)
		{
			//try to find the element in the file
			SettingBase* setting = i->second.Ptr();
			L_ASSERT(setting);
			bool isNew = false;
			tinyxml2::XMLElement* elem = findSettingInFile(possRoot->FirstChildElement(SETTING_NAME), setting->Name);
			//otherwise make it if it doesn't exist
			if(!elem)
			{
				isNew = true;
				elem = doc->NewElement(SETTING_NAME);
				attachNewSettingElem(setting, doc, elem);
				possRoot->LinkEndChild(elem);
			}
			else
			{
				tinyxml2::XMLElement* valElem = elem->FirstChildElement(VALUE_ATTR_NAME);
				tinyxml2::XMLText* valText = valElem->FirstChild()->ToText();
				if(!valText)
				{
					valText = doc->NewText("");
					valElem->LinkEndChild(valText);
				}
				//elem->SetAttribute(NAME_ATTR_NAME, setting->Name);
				//elem->SetAttribute(TYPE_ATTR_NAME, setting->Type);
				//now this part's tougher
				setValueElem(setting, valText);
			}
			elemMap[i->first] = elem;
		}
		//now save the file
		//need to flush the document to the printer first
		doc->Print(&printer);
		
		//since this is a straight dump, we'll write the new file,
		//delete the old one, and rename the new file to the proper name
		Path newPath = path.ToString() + "~";
		if(Filesystem::Exists(newPath))
		{
			Filesystem::RemoveFile(newPath);
		}
		DataStream* newFile = Filesystem::OpenFileText(newPath);
		newFile->Write(printer.CStr());
		Filesystem::CloseFile(newFile);
		Filesystem::CloseFile(file);
		//now rename the file
		Filesystem::Rename(newPath, path);
		//really iffy...
		CustomDelete(doc);
		return true;
		#pragma endregion
	}
}