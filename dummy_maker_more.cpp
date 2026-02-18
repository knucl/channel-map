#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

bool starts_with(const std::string& str, const std::string& prefix){
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}
// 引数は "./dummy_maker.out ./mapdata.csv 100"のように与えて、2つ目は元ファイル名、3つめは生成するダミーの数
int main(int argc, char* argv[]){
    if(argc == 1){
        std::cout << "Usage: " << argv[0] << " <original_file> <number_of_dummy_entries>" << std::endl;
        return 0;
    }
    std::string original_file = argv[1];
    if(argc > 2)// 2つ目は元ファイル名、3つめは生成するダミーの数
    #if 0
    std::cout << "Original file: " << original_file << std::endl;
    std::cout << "Number of dummy entries to generate: " << argv[2] << std::endl;
    #endif
    std::string dummy_file;
    // "original_file"の拡張子の前に"_dummy"を追加したファイル名を作成
    size_t dot_position = original_file.find_last_of('.');
    if (dot_position != std::string::npos) {
        dummy_file = original_file.substr(0, dot_position) + "_dummy" + original_file.substr(dot_position);
    }

    std::ifstream infile(original_file);
    std::ofstream outfile(dummy_file);

    std::string line;
    bool is_terminal_out = true;
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
        // const std::vector<std::string> dummy_prefix = {"0xc0a803","0xc0a804","0xc0a805","0xc0a806","0xc0a807","0xc0a808","0xc0a809"};
        std::vector<std::string> dummy_prefix;
        int nDummy = std::stoi(argv[2]);
        for(int i=0; i<nDummy; i++){
            std::string dummy_prefix_element;
            std::stringstream ss_dummy;
            ss_dummy << "0xc0a8" << std::setfill('0') << std::setw(2) << std::hex << (3+i);
            ss_dummy >> dummy_prefix_element;
            dummy_prefix.push_back(dummy_prefix_element);
            #if 0
            if(is_terminal_out){
                std::cout << "Generated dummy prefix: " << dummy_prefix_element << std::endl;
            }
            #endif
        }
        for(const auto& e_prefix : dummy_prefix){
            if(starts_with(first_element, original_prefix)){
                std::string original_elder = first_element.substr(original_prefix.size());
                std::string dummy_first_element = e_prefix + original_elder;
                std::string new_dummy_line = dummy_first_element + line.substr(first_element.size());
                outfile << new_dummy_line << std::endl;
            }// endif(starts_with(first_element, original_prefix))
        }
        is_terminal_out = false;
    }// endwhile(std::getline(infile, line))
    return 0;
}