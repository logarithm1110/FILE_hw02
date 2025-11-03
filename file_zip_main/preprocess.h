#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <string>
#include <vector>
#include <unordered_map>
#include<fstream>

struct replacement_entry {
	std::string pattern;//原始模式
	std::string token;//替换标记

	replacement_entry(const std::string& p, const std::string& t) : pattern(p), token(t) {}
};

class preprocessor{
public:
	preprocessor();

	std::string preprocess(const std::string& input);
	std::string restore(const std::string& processed);

	bool serialize_table(std::ofstream& out) const;
	bool deserialize_table(std::ifstream& in);

	size_t get_replacement_count() const;

	void clear();
private:
	std::vector<replacement_entry> replacements_list;
	std::unordered_map<std::string, std::string> pattern_to_token;
	std::unordered_map<std::string, std::string> token_to_pattern;
	
	void initialize_replacements();
	std::string generate_token(int index);
	void replace_all(std::string& text, const std::string& from, const std::string& to);
};

#endif