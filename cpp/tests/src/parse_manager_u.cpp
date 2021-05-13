#include "gtest/gtest.h"
#include "parse_manager.h"
#include "parser_config_params.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

class ParseManagerTest : public ::testing::Test
{
protected:

	ParserConfigParams config;
	std::string input_path = "";
	std::string output_path = "";
	ParseManager pm = ParseManager(input_path, output_path, &config);
	std::string filename = "_TMATS.txt";
	std::ifstream file;
	std::string line;
	ManagedPath base_output_dir_;
	ManagedPath base_name_;
	ManagedPath full_output_dir_;
	bool result_;
	std::map<Ch10PacketType, bool> pkt_enabled_map_;
	std::map<Ch10PacketType, std::string> append_str_map_;
	std::map<Ch10PacketType, ManagedPath> output_dir_map_;

	ParseManagerTest() : result_(false) {}

	void RemoveFile()
	{
		// delete previous file if it exists
		file.open(filename);
		if (file.good())
		{
			file.close();
			remove(filename.c_str());
		}
		file.close();
	}

	~ParseManagerTest()
	{

	}

	template <typename Map>
	bool map_compare(Map const& lhs, Map const& rhs) {
		return lhs.size() == rhs.size()
			&& std::equal(lhs.begin(), lhs.end(),
				rhs.begin());
	}

};

TEST_F(ParseManagerTest, NoTMATSLeftFromPriorTests)
{
	file.open(filename);
	ASSERT_FALSE(file.good());
	RemoveFile();
}

TEST_F(ParseManagerTest, NoTMATSPresent)
{
	std::vector<std::string> tmats;
	pm.ProcessTMATsTest(tmats);
	file.open(filename);
	// file shouldn't exist if tmats did
	// not exist
	ASSERT_FALSE(file.good());
	RemoveFile();
}

TEST_F(ParseManagerTest, TMATSWritten)
{
	std::vector<std::string> tmats = { "line1\nline2\n", "line3\nline4\n" };
	pm.ProcessTMATsTest(tmats);
	file.open(filename);
	ASSERT_TRUE(file.good());
	std::ostringstream ss;
	ss << file.rdbuf(); 
	std::string test = ss.str();
	ASSERT_EQ(test, "line1\nline2\nline3\nline4\n");
	RemoveFile();
}

TEST_F(ParseManagerTest, TMATSParsed)
{
	// R-x\TK1-n:channelID
	// R-x\DSI-n:Source
	// R-x\CDT-n:Type
	std::vector<std::string> tmats = { "line2;\n\n", 
										"R-1\\TK1-1:1;\n\n",
										"R-2\\TK1-2:2;\n\n",
										"R-2\\DSI-2:Bus2;\n\n",
										"R-1\\DSI-1:Bus1;\n",
										"R-3\\TK1-3:3;\n\n",
										"R-1\\CDT-1:type1;\n\n",
										"R-2\\CDT-2:type2;\n\n",
										"R-3\\CDT-3:type3;\n",
										"R-3\\DSI-3:Bus3;\n\n",
										"comment;\n",
										"Junk-1\\Junk1-1;\n\n",
									 };
	pm.ProcessTMATsTest(tmats);
	std::map<std::string,std::string> source_map_test = 
		pm.GetTMATsChannelIDToSourceMap();

	std::map<std::string, std::string> type_map_test =
		pm.GetTMATsChannelIDToTypeMap();

	std::map<std::string, std::string> source_map_truth = 
	{ {"1" , "Bus1"}, {"2" , "Bus2"}, {"3" , "Bus3"} };

	std::map<std::string, std::string> type_map_truth = 
	{ {"1" , "type1"}, {"2" , "type2"}, {"3" , "type3"} };

	ASSERT_TRUE(map_compare(source_map_test, source_map_truth));
	ASSERT_TRUE(map_compare(type_map_test, type_map_truth));
	RemoveFile();
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapEmptyMap)
{
	std::map<std::string, std::string> input_map;
	std::map<Ch10PacketType, bool> output_map;
	bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
	EXPECT_FALSE(res);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapInvalidPacketName)
{
	std::map<std::string, std::string> input_map = {
		{"MILSTD1553_FORMAT1", "true"},
		{"VIDEO_FORMAT", "true"} // VIDEO_FORMAT0 is possible, not without trailing "0"
	};
	std::map<Ch10PacketType, bool> output_map;
	bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
	EXPECT_FALSE(res);
	EXPECT_EQ(output_map.size(), 0);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapInvalidBooleanString)
{
	std::map<std::string, std::string> input_map = {
		{"MILSTD1553_FORMAT1", "tru"}, // "tru" is not a valid boolean string
		{"VIDEO_FORMAT0", "true"}
	};
	std::map<Ch10PacketType, bool> output_map;
	bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
	EXPECT_FALSE(res);
	EXPECT_EQ(output_map.size(), 0);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapCorrectMapping)
{
	std::map<std::string, std::string> input_map = {
		{"MILSTD1553_FORMAT1", "false"},
		{"VIDEO_FORMAT0", "true"}
	};
	std::map<Ch10PacketType, bool> output_map;
	bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
	EXPECT_TRUE(res);
	EXPECT_EQ(output_map.count(Ch10PacketType::MILSTD1553_F1), 1);
	EXPECT_EQ(output_map.count(Ch10PacketType::VIDEO_DATA_F0), 1);

	EXPECT_EQ(output_map.at(Ch10PacketType::MILSTD1553_F1), false);
	EXPECT_EQ(output_map.at(Ch10PacketType::VIDEO_DATA_F0), true);

	input_map["MILSTD1553_FORMAT1"] = "True";
	input_map["VIDEO_FORMAT0"] = "fAlse";
	output_map.clear();
	res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
	EXPECT_TRUE(res);
	EXPECT_EQ(output_map.count(Ch10PacketType::MILSTD1553_F1), 1);
	EXPECT_EQ(output_map.count(Ch10PacketType::VIDEO_DATA_F0), 1);

	EXPECT_EQ(output_map.at(Ch10PacketType::MILSTD1553_F1), true);
	EXPECT_EQ(output_map.at(Ch10PacketType::VIDEO_DATA_F0), false);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirObjectEmptyPaths)
{
	std::string append_str;
	// Set base_output_dir to current dir, leave base_name_ empty.
	base_output_dir_ = ManagedPath();
	base_name_ = ManagedPath(std::string(""));
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_FALSE(result_);

	// Flip, set base_output_dir empty, base_nam_ to current dir
	base_output_dir_ = ManagedPath(std::string(""));
	base_name_ = ManagedPath();
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirObjectEmptyAppendStr)
{
	std::string append_str;
	std::string expect;

	// base file name is just a file, not a path
	base_output_dir_ = ManagedPath();
	base_name_ = ManagedPath(std::string("my_file"));
	expect = (base_output_dir_ / base_name_).RawString();
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_TRUE(result_);
	EXPECT_EQ(expect, full_output_dir_.RawString());

	// base file name is a path with file name
	base_name_ = ManagedPath(std::string("data"));
	base_name_ /= ManagedPath(std::string("my_file.txt"));
	expect = (base_output_dir_ / base_name_.filename()).RawString();
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_TRUE(result_);
	EXPECT_EQ(expect, full_output_dir_.RawString());
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirObjectNonEmptyAppendStr)
{
	std::string append_str = ".pq";
	std::string expect;

	// base file name is just a file, not a path
	base_output_dir_ = ManagedPath();
	base_name_ = ManagedPath(std::string("my_file"));
	expect = (base_output_dir_ / base_name_).RawString() + ".pq";
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_TRUE(result_);
	EXPECT_EQ(expect, full_output_dir_.RawString());

	// base file name is a path with file name
	base_name_ = ManagedPath(std::string("data"));
	base_name_ /= ManagedPath(std::string("my_file.txt"));
	expect = (base_output_dir_ / base_name_.stem()).RawString() + ".pq";
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, false);
	EXPECT_TRUE(result_);
	EXPECT_EQ(expect, full_output_dir_.RawString());
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirCreateDir)
{
	std::string append_str = ".pq";

	base_output_dir_ = ManagedPath();
	base_name_ = ManagedPath(std::string("my_file"));
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, true);
	EXPECT_TRUE(result_);
	EXPECT_TRUE(full_output_dir_.is_directory());
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirCreateDirFail)
{
	std::string append_str = ".pq";

	// base dir does not exist, so dir creation will fail
	base_output_dir_ = ManagedPath() / "test" / "data";
	base_name_ = ManagedPath(std::string("my_file"));
	result_ = pm.CreateCh10PacketOutputDirObject(base_output_dir_, base_name_,
		append_str, full_output_dir_, true);
	EXPECT_FALSE(result_);
}

std::map<Ch10PacketType, bool> pkt_enabled_map_;
std::map<Ch10PacketType, std::string> append_str_map_;
std::map<Ch10PacketType, ManagedPath> output_dir_map_;

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsMissingAppendStr)
{
	pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
	pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = false;
	
	// No append string entry for 1553
	append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";

	result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
		pkt_enabled_map_, append_str_map_, output_dir_map_, false);
	EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsEmptyOutputOnFailure)
{
	pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
	pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

	// No append string entry for 1553
	append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";
	append_str_map_[Ch10PacketType::MILSTD1553_F1] = "_1553.parquet";

	// Create empty base_output_dir_ to evoke failure.
	base_output_dir_ = ManagedPath(std::string(""));
	result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
		pkt_enabled_map_, append_str_map_, output_dir_map_, false);
	EXPECT_FALSE(result_);
	EXPECT_EQ(output_dir_map_.size(), 0);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsCorrectDirs)
{
	pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
	pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

	// No append string entry for 1553
	append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";
	append_str_map_[Ch10PacketType::MILSTD1553_F1] = "_1553.parquet";

	base_name_ = ManagedPath(std::string("my_data"));
	std::string expected_video = (base_output_dir_ / base_name_).RawString() +
		append_str_map_.at(Ch10PacketType::VIDEO_DATA_F0);
	std::string expected_1553 = (base_output_dir_ / base_name_).RawString() +
		append_str_map_.at(Ch10PacketType::MILSTD1553_F1);
	result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
		pkt_enabled_map_, append_str_map_, output_dir_map_, false);
	EXPECT_TRUE(result_);
	EXPECT_EQ(output_dir_map_.size(), 2);
	EXPECT_EQ(expected_video, output_dir_map_.at(Ch10PacketType::VIDEO_DATA_F0));
	EXPECT_EQ(expected_1553, output_dir_map_.at(Ch10PacketType::MILSTD1553_F1));
}