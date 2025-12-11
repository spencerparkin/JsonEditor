#include "JsonDataViewModel.h"
#include <format>

using namespace ParseParty;

JsonDataViewModel::JsonDataViewModel()
{
	this->jsonRootValue = nullptr;
}

/*virtual*/ JsonDataViewModel::~JsonDataViewModel()
{
	delete this->jsonRootValue;
}

void JsonDataViewModel::SetJsonRootValue(ParseParty::JsonValue* jsonRootValue)
{
	delete this->jsonRootValue;
	this->jsonRootValue = jsonRootValue;
	this->metaDataMap.clear();
}

ParseParty::JsonValue* JsonDataViewModel::GetJsonRootValue()
{
	return this->jsonRootValue;
}

/*virtual*/ void JsonDataViewModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	const JsonValue* jsonValue = static_cast<const JsonValue*>(item.m_pItem);

	switch (col)
	{
		case COL_NAME:
		{
			auto iter = this->metaDataMap.find((JsonValue*)jsonValue);
			if (iter == this->metaDataMap.end())
				variant = wxVariant(wxT("?"));
			else
			{
				const JsonValueMetaData& metaData = iter->second;
				variant = wxVariant(wxString(metaData.name));
			}
			break;
		}
		case COL_TYPE:
		{
			if (dynamic_cast<const JsonObject*>(jsonValue))
				variant = wxVariant(wxString(wxT("Object")));
			else if (dynamic_cast<const JsonArray*>(jsonValue))
				variant = wxVariant(wxString(wxT("Array")));
			else if (dynamic_cast<const JsonString*>(jsonValue))
				variant = wxVariant(wxString(wxT("String")));
			else if (dynamic_cast<const JsonInt*>(jsonValue))
				variant = wxVariant(wxString(wxT("Integer")));
			else if (dynamic_cast<const JsonBool*>(jsonValue))
				variant = wxVariant(wxString(wxT("Boolean")));
			else
				variant = wxVariant(wxString(wxT("???")));

			break;
		}
		case COL_VALUE:
		{
			wxString value;

			auto jsonString = dynamic_cast<const JsonString*>(jsonValue);
			if (jsonString)
				value = jsonString->GetValue();

			auto jsonInt = dynamic_cast<const JsonInt*>(jsonValue);
			if (jsonInt)
				value = wxString::Format("%d", jsonInt->GetValue());

			auto jsonBool = dynamic_cast<const JsonBool*>(jsonValue);
			if (jsonBool)
				value = jsonBool->GetValue() ? wxT("TRUE") : wxT("FALSE");

			variant = wxVariant(value);

			break;
		}
	}
}

/*virtual*/ bool JsonDataViewModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);

	return false;
}

/*virtual*/ wxDataViewItem JsonDataViewModel::GetParent(const wxDataViewItem& item) const
{
	wxDataViewItem parentItem;

	return parentItem;
}

/*virtual*/ bool JsonDataViewModel::IsContainer(const wxDataViewItem& item) const
{
	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);

	if (!jsonValue)
		jsonValue = this->jsonRootValue;

	if (dynamic_cast<JsonObject*>(jsonValue) || dynamic_cast<JsonArray*>(jsonValue))
		return true;

	return false;
}

/*virtual*/ unsigned int JsonDataViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);

	if (!jsonValue)
		jsonValue = this->jsonRootValue;

	JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonValue);
	if (jsonObject)
	{
		for (auto pair : *jsonObject)
		{
			JsonValueMetaData metaData;
			metaData.jsonParentValue = jsonObject;
			metaData.name = pair.first;
			this->metaDataMap.insert(std::pair(pair.second, metaData));

			wxDataViewItem childItem;
			childItem.m_pItem = pair.second;
			children.Add(childItem);
		}

		return jsonObject->GetSize();
	}

	JsonArray* jsonArray = dynamic_cast<JsonArray*>(jsonValue);
	if (jsonArray)
	{
		for (unsigned int i = 0; i < jsonArray->GetSize(); i++)
		{
			JsonValueMetaData metaData;
			metaData.jsonParentValue = jsonArray;
			metaData.name = std::format("{}", i);
			this->metaDataMap.insert(std::pair(jsonArray->GetValue(i), metaData));

			wxDataViewItem childItem;
			childItem.m_pItem = jsonArray->GetValue(i);
			children.Add(childItem);
		}

		return jsonArray->GetSize();
	}

	return 0;
}

/*virtual*/ bool JsonDataViewModel::HasContainerColumns(const wxDataViewItem& item) const
{
	return true;
}