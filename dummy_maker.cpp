#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

bool starts_with(const std::string& str, const std::string& prefix){
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

int main(int argc, char* argv[]){
    std::string original_file = argv[1];
    std::string dummy_file;
    // "original_file"の拡張子の前に"_dummy"を追加したファイル名を作成
    size_t dot_position = original_file.find_last_of('.');
    if (dot_position != std::string::npos) {
        dummy_file = original_file.substr(0, dot_position) + "_dummy" + original_file.substr(dot_position);
    }

    std::ifstream infile(original_file);
    std::ofstream outfile(dummy_file);

    std::string line;
    while(std::getline(infile, line)){
        // ヘッダはそのまま出力
        if(starts_with(line, "fe.id")){
            outfile << line << std::endl;
            continue;
        }
        // 一つ目の要素によって複製して出力
        std::stringstream ss(line);
        std::string first_element;
        std::getline(ss, first_element, ',');
        outfile << line << std::endl; // 元の行を出力
        const std::string original_prefix = "0xc0a802";
        const std::vector<std::string> dummy_prefix = {"0xc0a803","0xc0a804","0xc0a805","0xc0a806","0xc0a807","0xc0a808","0xc0a809"};
        for(const auto& e_prefix : dummy_prefix){
            if(starts_with(first_element, original_prefix)){
                std::string original_elder = first_element.substr(original_prefix.size());
                std::string dummy_first_element = e_prefix + original_elder;
                std::string new_dummy_line = dummy_first_element + line.substr(first_element.size());
                outfile << new_dummy_line << std::endl;
            }// endif(starts_with(first_element, original_prefix))
        }

    }// endwhile(std::getline(infile, line))
    return 0;
}