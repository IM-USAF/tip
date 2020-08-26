#ifndef PARQUET_CONTEXT_H
#define PARQUET_CONTEXT_H

#include <vector>
#include <cstdint>
#include <stdio.h>
#include <map>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>
#include <typeinfo>

#include "column_data.h"

/*

!!! NOTICE !!!

The preprocessor macros ARROW_STATIC and PARQUET_STATIC must be defined
if linking to the library statically.

!!! NOTICE !!!

*/

class ParquetContext
{
private:
	std::string host_;
	std::string user_;
	std::string path_;
	std::vector<std::string> temp_string_vec_;
	int port_;
	bool truncate_;
	arrow::Status st_;
	bool have_created_writer_;
	bool have_created_schema_;
	arrow::MemoryPool* pool_;
	bool parquet_stop_;
	std::unique_ptr<parquet::arrow::FileWriter> writer_;
	int ROW_GROUP_COUNT_;
	uint8_t ret_;
	int append_row_count_;
	bool have_created_table_;
	std::map<std::string, ColumnData> column_data_map_;
	std::shared_ptr<arrow::Schema> schema_;
	std::shared_ptr<arrow::io::FileOutputStream> ostream_;
	std::shared_ptr<parquet::WriterProperties> props_;
	std::shared_ptr<arrow::io::ReadableFile> input_file_;
	std::vector<std::shared_ptr<arrow::Field>> fields_;
	std::vector<uint8_t> cast_vec_;

	std::unique_ptr<arrow::ArrayBuilder> 
		GetBuilderFromDataType(
			const std::shared_ptr<arrow::DataType> dtype,
			const bool& is_list_builder);

	bool AppendColumn(ColumnData& dataInfo, 
		const int& rows, 
		const int offset = 0);

	template<typename T, typename A>
	void Append(const bool& isList,
		const bool& castRequired,
		const int& listCount,
		const int& offset, 
		const ColumnData& columnData);

	void CreateBuilders();

	std::vector<int32_t> GetOffsetsVector(const int& n_rows, 
		const int& elements_per_row, 
		const int offset = 0);

	void WriteColsIfReady();

	void FillStringVec(std::vector<std::string>* str_data_vec_ptr, 
		const int& count,
		const int offset = 0);	

	bool IsUnsigned(const std::shared_ptr<arrow::DataType> type);

	std::string GetTypeIDFromArrowType(const std::shared_ptr<arrow::DataType> type,
		int& byteSize);

	template<typename castToType> 
	void CastTo(const void const* data,
		const CastFromType castFrom,
		const int& size,
		const int& offset);
	

public:
	ParquetContext();
	ParquetContext(int rgSize);
	~ParquetContext();

	void Close();

	uint8_t OpenForWrite(const std::string& path,
		const bool& truncate);

	void AddField(const std::shared_ptr<arrow::DataType> type, 
		const std::string& fieldName, 
		const int listSize=NULL);

	bool WriteColumns(const int& rows, const int offset=0);
	
	bool WriteColumns();

	template<typename NativeType> 
	bool SetMemoryLocation(std::vector<NativeType>& data, 
		const std::string&, 
		uint8_t* boolField=nullptr);

	template<typename NativeType>
	bool SetMemoryLocation(std::vector<std::string>& strVec, 
		const std::string& fieldName, 
		uint8_t* boolField=nullptr);
};

template<typename T, typename A>
void ParquetContext::Append(const bool& isList, 
	const bool& castRequired, 
	const int& listCount, 
	const int& offset,  
	const ColumnData& columnData)
{
	if (isList)
	{
		// Get the relevant builder for the data type.
		std::shared_ptr<arrow::ListBuilder> bldr =
			std::dynamic_pointer_cast<arrow::ListBuilder>(columnData.builder_);

		// Resize array to allocate space and append data.
		bldr->Resize(append_row_count_);

		// Resize the global cast vector to the minimum size needed
		// in bytes
		if (cast_vec_.size() < (append_row_count_ * listCount * sizeof(T)))
			cast_vec_.resize(append_row_count_ * listCount * sizeof(T));

		if (castRequired)
		{
			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);

			A* sub_bldr =
				static_cast<A*>(bldr->value_builder());			

			CastTo<T>(columnData.data_,
				columnData.cast_from_,
				append_row_count_ * listCount,
				offset * listCount);

			sub_bldr->AppendValues((T*)cast_vec_.data(), 
				append_row_count_ * listCount);
		}
		else
		{
			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);
			A* sub_bldr =
				static_cast<A*>(bldr->value_builder());
			sub_bldr->AppendValues((T*)columnData.data_ + (offset * listCount), 
				append_row_count_ * listCount);
		}
	}
	else 
	{
		std::shared_ptr<A> bldr =
			std::dynamic_pointer_cast<A>(columnData.builder_);

		// Resize array to allocate space and append data.
		bldr->Resize(append_row_count_);

		// Resize the global cast vector to the minimum size needed
		// in bytes
		if (cast_vec_.size() < (append_row_count_ * sizeof(T)))
			cast_vec_.resize(append_row_count_ * sizeof(T));

		if (columnData.null_values_ == nullptr) 
		{
			if (castRequired) 
			{
				CastTo<T>(columnData.data_,
					columnData.cast_from_,
					append_row_count_,
					offset);

				bldr->AppendValues((T*)cast_vec_.data(),
					append_row_count_);
			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset, 
					append_row_count_);
		}
		else
		{
			if (castRequired) 
			{
				CastTo<T>(columnData.data_,
					columnData.cast_from_,
					append_row_count_,
					offset);

				bldr->AppendValues((T*)cast_vec_.data(),
					append_row_count_,
					columnData.null_values_);

			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset,
					append_row_count_, 
					columnData.null_values_);
		}
	}
}

template<typename castToType>
void ParquetContext::CastTo(const void const* data,
	const CastFromType castFrom,
	const int& size,
	const int& offset)
{
	switch (castFrom)
	{
	case INT8:
		std::copy((int8_t*)data + offset,
			(int8_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT8:
		std::copy((uint8_t*)data + offset,
			(uint8_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT16:
		std::copy((int16_t*)data + offset,
			(int16_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT16:
		std::copy((uint16_t*)data + offset,
			(uint16_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT32:
		std::copy((int32_t*)data + offset,
			(int32_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;
	
	case UINT32:
		std::copy((uint32_t*)data + offset,
			(uint32_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT64:
		std::copy((int64_t*)data + offset,
			(int64_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT64:
		std::copy((uint64_t*)data + offset,
			(uint64_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;
	}
}

template<typename NativeType> 
bool ParquetContext::SetMemoryLocation(std::vector<NativeType>& data, 
	const std::string& fieldName, 
	uint8_t* boolField)
{

	for (std::map<std::string, ColumnData>::iterator 
		it = column_data_map_.begin(); 
		it != column_data_map_.end(); 
		++it) {

		if (it->first == fieldName) 
		{
			NativeType a;

			// If it is a list and boolField is defined
			// make sure to reset boolField to null since
			// null lists aren't available
			if (it->second.is_list_)
			{
				if (boolField != nullptr)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Null fields for lists"
						"are currently unavailable: %s\n",
						fieldName.c_str());
#endif
#endif
					boolField = nullptr;
				}

				if (data.size() % it->second.list_size_ != 0)
				{
					printf("Error!!!!!!!!!!!!  list size specified (%d)"
						" is not a multiple of total data length (%d) "
						"for column: %s\n",
						it->second.list_size_,
						data.size(),
						fieldName);
					parquet_stop_ = true;
					return false;
				}				
			}

			// Check if ptr is already set
			if (it->second.pointer_set_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, ptr"
						"is already set for: %s\n", 
					fieldName.c_str());
#endif
#endif
			}

			// If casting is required			
			if (typeid(NativeType).name() != it->second.type_ID_) 
			{
				// Check if other types are being written
				// to or from a string or to boolean from 
				// anything but uint8_t 
				// NativeType is they type being cast FROM and second.type_->id() 
				// is the type being cast TO which is the original arrow type passed 
				// to AddField 
				// Note: It is impossible to stop boolean from being cast
				// up to a larger type, since NativeType for boolean is uint8_t.
				// The only way to stop boolean from being cast
				// up would be to stop all uint8_t from being cast up.
				// Also note that it->second.type_ID_ is originally retrieved from
				// ParquetContext::GetTypeIDFromArrowType and every type is as 
				// expected except boolean. Boolean arrow types will result in 
				// uint8_t being assigned to it->second.type_ID_
				if (it->second.type_->id() == arrow::StringType::type_id ||
					typeid(std::string).name() == typeid(NativeType).name() ||
					it->second.type_->id() == arrow::BooleanType::type_id)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  can't cast from other data"
							"type to string or bool for: %s\n", 
						fieldName.c_str());
#endif
#endif
					parquet_stop_ = true;
					return false;
				}
				// Cast to larger datatype check
				else if (it->second.byte_size_ < sizeof(a)) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Intended datatype to be cast"
							"is smaller than the given datatype for: %s\n", 
						fieldName.c_str());
#endif
#endif
					parquet_stop_ = true;
					return false;
				}
				// Check if floating point casting is happening
				else if (it->second.type_ID_ == typeid(float).name() ||
					it->second.type_ID_ == typeid(double).name() || 
					typeid(NativeType).name() == typeid(float).name() || 
					typeid(NativeType).name() == typeid(double).name()) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Can't cast floating"
							"point data types: %s, \n", 
						fieldName.c_str());
#endif
#endif
					parquet_stop_ = true;
					return false;
				}
				// Equal size datatypes check
				else if (it->second.byte_size_ == sizeof(a)) 
				{
					it->second.SetColumnData(data.data(), 
						fieldName, 
						typeid(NativeType).name(),
						boolField,
						data.size());
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Intended datatype to "
							"be cast is equal to the casting type for: %s\n", 
						fieldName.c_str());
#endif
#endif
				}
				else {
					it->second.SetColumnData(data.data(), 
						fieldName, 
						typeid(NativeType).name(), 
						boolField, 
						data.size());
#ifdef DEBUG
#if DEBUG > 1
					printf("Cast from %s planned for: %s, \n", 
						typeid(NativeType).name(), 
						fieldName.c_str());
#endif
#endif
				}
			}
			// Data types are the same and no casting required
			else
			{
				it->second.SetColumnData(data.data(), 
					fieldName, 
					"", 
					boolField,
					data.size());
			}

			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist: %s\n", 
		fieldName.c_str());

	parquet_stop_ = true;
	return false;
}

template<typename NativeType>
bool ParquetContext::SetMemoryLocation(std::vector<std::string>& data,
	const std::string& fieldName,
	uint8_t* boolField)
{
	for (std::map<std::string, ColumnData>::iterator
		it = column_data_map_.begin();
		it != column_data_map_.end();
		++it)
	{
		if (it->first == fieldName)
		{
			if (typeid(std::string).name() != it->second.type_ID_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, "
					"can't cast from string to other types: %s\n",
					fieldName);
#endif
#endif
				parquet_stop_ = true;
				return false;
			}
			else if (it->second.pointer_set_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, "
					"ptr is already set for: %s\n",
					fieldName);
#endif
#endif
			}
			if (it->second.is_list_)
			{
				if (boolField != nullptr)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Null fields for lists"
						"are currently unavailable: %s\n",
						fieldName);
#endif
#endif
					boolField = nullptr;
				}
				if (data.size() % it->second.list_size_ != 0)
				{
					printf("Error!!!!!!!!!!!!  list size specified (%d)"
							" is not a multiple of total data length (%d) "
							"for column: %s\n", 
						it->second.list_size_,
						data.size(),
						fieldName);
					parquet_stop_ = true;
					return false;
				}
			}
#ifdef DEBUG
#if DEBUG > 1
			printf("setting field info for %s\n",
				fieldName.c_str());
#endif
#endif
			it->second.SetColumnData(data, fieldName, boolField, data.size());
			
			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist:%s\n",
		fieldName.c_str());
	parquet_stop_ = true;
	return false;
}


#endif