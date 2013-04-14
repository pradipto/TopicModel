#ifndef _CUTILITIES_H_
#define _CUTILITIES_H_

#include "includes.h"

typedef std::pair<double, int> pair_double_int;
typedef std::pair<std::string, double> pair_string_double;

class CUtilities
{
	struct SToLower	{
		char operator() (char c) const  { return std::tolower(c); }
	};

	// some static variables
public:
	static std::string _file_path_separator;

public:
	CUtilities(void);

	// utility functions
	static void print_command_help();

	static void delete_trailing_forward_slashes(std::string& path);
	static void trim(std::string& str);
	static void trim_left_right(std::string& str);
	static void delete_dashes(std::string& str);	// deprecated
	static void delete_character(std::string& str, char c);
	static void delete_non_alphanum_left_right(std::string& str);
	static void to_lower(std::string& str);
	static void tokenize(std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);

	static bool is_valid_directory(const std::string& dir);
	static bool make_directory(const std::string& dir);

	static bool compareFirstPredDescending_pairDoubleInt(const pair_double_int& left, const pair_double_int& right);
	static bool compareFirstPredAscending_pairDoubleInt(const pair_double_int& left, const pair_double_int& right);

	static bool compareSecondPredDescending_pairStringDouble(const pair_string_double& left, const pair_string_double& right);
	static bool compareSecondPredAscending_pairStringDouble(const pair_string_double& left, const pair_string_double& right);

	/**
	 * This method converts a std::string to the templatized primitive datatype
	 */
	template <class T>
	static bool string_to_number(T& d, std::string const& s, std::ios_base& (*f)(std::ios_base&) = std::dec) {
		std::stringstream ss(s);
		bool ret = true;
		ret = !(ss >> f >> d).fail();
		return ret;
	}

	/**
	 * This method converts a templatized primitive datatype to std::string
	 */
	template <class T>
	static std::string number_to_string(T& d) {
		std::string s;
		std::stringstream ss;
		bool ret = true;
		ret = !(ss << d).fail();

		s = ret ? ss.str() : "";
		return s;
	}

	/*
	 * These subsequent methods are part of AToM-beta toolbox and should be moved to somewhere else
	 */
	/**
	 * @return an integer representing a topic with highest probability mass
	 */
	static int get_max_topic_index(std::vector<int>& topic_assignment_vec) {
		std::map<int, int> topic_num_count_map;
		std::map<int, int>::iterator m_itr;
		size_t topic_vec_size = topic_assignment_vec.size();
		for ( size_t i = 0; i < topic_vec_size; ++i ) {
			m_itr = topic_num_count_map.find(topic_assignment_vec[i]);
			if ( m_itr == topic_num_count_map.end() )
				topic_num_count_map.insert( std::make_pair(topic_assignment_vec[i], 1) );
			else m_itr->second = m_itr->second + 1;
		}
		int max_topic = -1;
		int max_topic_count = 0;
		for ( m_itr = topic_num_count_map.begin(); m_itr != topic_num_count_map.end(); ++m_itr )
			if ( m_itr->second > max_topic_count ) {
				max_topic_count = m_itr->second;
				max_topic = m_itr->first;
			}
		return max_topic;
	}

	/**
	 * @return an int which is the index i in vector v such that v[i] = value
	 */
	template<class T>
	static int find_index_in_vector(std::vector<T>& v, T& value) {
		size_t length = v.size();
		int index = -1;
		for ( size_t i = 0; i < length; ++i )
			if ( v.at(i) == value ) {
				index = i;
				break;
			}
		return index;
	}

public:
	~CUtilities(void);
};

#endif
