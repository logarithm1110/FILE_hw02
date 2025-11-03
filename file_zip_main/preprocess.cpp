#include "preprocess.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

preprocessor::preprocessor() {
	initialize_replacements();
}

void preprocessor::initialize_replacements(){
	vector<pair<string, string>>patterns = {
		//日志头部信息√
        {"#Software: Hllpoj Server 1.0.1 / Logger 1.0.0 built 0001","§SW§"},
		{"#Version: 1.0","§V1§"},
        {"#Fields: date time s-ip cs-method cs-uri-stem cs-uri-query s-port cs-username c-ip cs(User-Agent) cs(Referer) sc-status sc-substatus sc-win32-status time-taken", "§F1§"},

        //User-Agent√
        {"Mozilla/5.0+(Linux;+U;+Android+8.0.0;+en-us;+MIX+2+Build/OPR1.170623.027)+AppleWebKit/537.36+(KHTML,+like+Gecko)+Version/4.0+Chrome/61.0.3163.128+Mobile+Safari/537.36+XiaoMi/MiuiBrowser/10.5.2", "§UA1§"},
        {"Mozilla/5.0+(Windows+NT+6.3;+Win64;+x64)+AppleWebKit/537.36+(KHTML,+like+Gecko)+Chrome/72.0.3626.121+Safari/537.36", "§UA2§"},
        {"Mozilla/5.0+(Macintosh;+Intel+Mac+OS+X+10_13_6)+AppleWebKit/605.1.15+(KHTML,+like+Gecko)+Version/12.0.3+Safari/605.1.15", "§UA3§"},

        //日期√
        {"2019-03-13 ", "§DATE§"},

        //IP√
        {"***.***.***.***", "§IP§"},  

        //URL√
        {"/login.php", "§LOGIN§"},
        {"/default/******.php", "§DEFPHP§"},
        {"/default/******.htm", "§DEFHTM§"},
        {"/files/assets/", "§ASSETS§"},
        {"/files/bower_components/", "§BOWER§"},

        //HTTP√
        {" POST ", "§POST§"},
        {" GET ", "§GET§"},
        {" 200 ", "§200§"}, 
        {" 304 ", "§304§"}, 
        {" 404 ", "§404§"},

        //查询参数√
        {"problemid=7_1", "§PID71§"},
        {"problemid=7_2", "§PID72§"},
        {"problemid=7_3", "§PID73§"},
        {"problemid=7_2_f", "§PID72F§"},

        //响应时间√
        {" time-taken", "§TIME§"},
    };

    for (size_t i = 0; i < patterns.size(); ++i) {
        const auto& p = patterns[i];
		replacement_entry entry(p.first, p.second);
		replacements_list.push_back(entry);
		pattern_to_token[p.first] = p.second;
		token_to_pattern[p.second] = p.first;
    }
}

string preprocessor::generate_token(int index) {
    return "§T" + to_string(index) + "§";
}

void  preprocessor::replace_all(string& text, const string& from, const string& to) {
    if (from.empty())return;

    size_t pos = 0;
    while ((pos = text.find(from, pos)) != string::npos) {
        text.replace(pos, from.length(), to);
        pos += to.length();
    }
}

string preprocessor::preprocess(const string& input) {
    string result = input;

    for (const auto& entry : replacements_list) {
        replace_all(result, entry.pattern, entry.token);
    }

    return result;
}

string preprocessor::restore(const string& processed) {
    string result = processed;

    for (auto it = replacements_list.rbegin(); it != replacements_list.rend(); ++it) {
        replace_all(result, it->token, it->pattern);
    }

    return result;
}

bool preprocessor::serialize_table(ofstream& out) const {
    if (!out.good()) return false;

    uint32_t count = static_cast<uint32_t>(replacements_list.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& entry : replacements_list) {
        // 写入 pattern 长度和内容
        uint32_t pattern_len = static_cast<uint32_t>(entry.pattern.length());
        out.write(reinterpret_cast<const char*>(&pattern_len), sizeof(pattern_len));
        out.write(entry.pattern.c_str(), pattern_len);

        // 写入 token 长度和内容
        uint32_t token_len = static_cast<uint32_t>(entry.token.length());
        out.write(reinterpret_cast<const char*>(&token_len), sizeof(token_len));
        out.write(entry.token.c_str(), token_len);
    }

    return out.good();
}

bool preprocessor::deserialize_table(ifstream& in) {
    if (!in.good()) return false;

    clear();

    // 读取替换表条目数量
    uint32_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in.good()) return false;

    replacements_list.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        // 读取 pattern
        uint32_t pattern_len;
        in.read(reinterpret_cast<char*>(&pattern_len), sizeof(pattern_len));
        if (!in.good()) return false;

        string pattern(pattern_len, '\0');
        in.read(&pattern[0], pattern_len);
        if (!in.good()) return false;

        // 读取 token
        uint32_t token_len;
        in.read(reinterpret_cast<char*>(&token_len), sizeof(token_len));
        if (!in.good()) return false;

        std::string token(token_len, '\0');
        in.read(&token[0], token_len);
        if (!in.good()) return false;

        // 添加到替换表
        replacement_entry entry(pattern, token);
        replacements_list.push_back(entry);
        pattern_to_token[pattern] = token;
        token_to_pattern[token] = pattern;
    }

    return true;
}

size_t preprocessor::get_replacement_count() const {
    size_t size = sizeof(uint32_t); // count
    for (const auto& entry : replacements_list) {
        size += sizeof(uint32_t) * 2; // pattern_len + token_len
        size += entry.pattern.length() + entry.token.length();
    }
    return size;
}

void preprocessor::clear() {
    replacements_list.clear();
    pattern_to_token.clear();
    token_to_pattern.clear();
}