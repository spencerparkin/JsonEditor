#include "JsonDataViewModel.h"
#include <format>

using namespace ParseParty;

JsonDataViewModel::JsonDataViewModel()
{
}

/*virtual*/ JsonDataViewModel::~JsonDataViewModel()
{
}

void JsonDataViewModel::SetJsonRootValue(std::shared_ptr<ParseParty::JsonValue> jsonRootValue)
{
	this->jsonRootValue = jsonRootValue;
	this->metaDataMap.clear();
}

std::shared_ptr<ParseParty::JsonValue> JsonDataViewModel::GetJsonRootValue()
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

	switch (col)
	{
		case COL_NAME:
		{
			wxString newName = variant.GetString();

			auto iter = this->metaDataMap.find((JsonValue*)jsonValue);
			if (iter != this->metaDataMap.end())
			{
				JsonValueMetaData& metaData = iter->second;
				
				std::shared_ptr<JsonValue> jsonParentValue = metaData.jsonParentValueWeakPtr.lock();

				auto jsonObject = dynamic_cast<JsonObject*>(jsonParentValue.get());
				if (jsonObject && jsonObject->GetValue(metaData.name).get() == jsonValue)
				{
					if (jsonObject->GetValue(newName.ToStdString()) == nullptr)
					{
						metaData.name = newName;
						jsonObject->SetValue(metaData.name, jsonValue->shared_from_this());
						return true;
					}
				}
			}

			return false;
		}
		case COL_VALUE:
		{
			wxString newValue = variant.GetString();

			auto jsonString = dynamic_cast<JsonString*>(jsonValue);
			if (jsonString)
			{
				jsonString->SetValue(newValue.ToStdString());
				return true;
			}

			auto jsonInt = dynamic_cast<JsonInt*>(jsonValue);
			if (jsonInt)
			{
				long intValue = 0;
				if (!newValue.ToLong(&intValue))
					return false;

				jsonInt->SetValue(intValue);
				return true;
			}

			auto jsonBool = dynamic_cast<JsonBool*>(jsonValue);
			if (jsonBool)
			{
				if (newValue.Upper() == wxT("TRUE"))
				{
					jsonBool->SetValue(true);
					return true;
				}
				else if (newValue.Upper() == wxT("FALSE"))
				{
					jsonBool->SetValue(false);
					return true;
				}
			}

			return false;
		}
		case COL_TYPE:
		{
			return false;
		}
	}

	return true;
}

/*virtual*/ wxDataViewItem JsonDataViewModel::GetParent(const wxDataViewItem& item) const
{
	wxDataViewItem parentItem;

	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);
	auto iter = this->metaDataMap.find((JsonValue*)jsonValue);
	if (iter != this->metaDataMap.end())
	{
		JsonValueMetaData& metaData = iter->second;
		std::shared_ptr<JsonValue> jsonParentValue = metaData.jsonParentValueWeakPtr.lock();
		if (jsonParentValue.get() != this->jsonRootValue.get())
			parentItem.m_pItem = jsonParentValue.get();
	}

	return parentItem;
}

/*virtual*/ bool JsonDataViewModel::IsContainer(const wxDataViewItem& item) const
{
	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);

	if (!jsonValue)
		jsonValue = this->jsonRootValue.get();

	if (dynamic_cast<JsonObject*>(jsonValue) || dynamic_cast<JsonArray*>(jsonValue))
		return true;

	return false;
}

/*virtual*/ unsigned int JsonDataViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	JsonValue* jsonValue = static_cast<JsonValue*>(item.m_pItem);

	if (!jsonValue)
		jsonValue = this->jsonRootValue.get();

	JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonValue);
	if (jsonObject)
	{
		for (auto pair : *jsonObject)
		{
			JsonValueMetaData metaData;
			metaData.jsonParentValueWeakPtr = jsonObject->shared_from_this();
			metaData.name = pair.first;
			this->metaDataMap.insert(std::pair(pair.second.get(), metaData));

			wxDataViewItem childItem;
			childItem.m_pItem = pair.second.get();
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
			metaData.jsonParentValueWeakPtr = jsonArray->shared_from_this();
			metaData.name = std::format("{}", i);
			this->metaDataMap.insert(std::pair(jsonArray->GetValue(i).get(), metaData));

			wxDataViewItem childItem;
			childItem.m_pItem = jsonArray->GetValue(i).get();
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