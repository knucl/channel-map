#include <channel_map_simple.hpp>
#include <channel_map_simple_item.hpp>
#include <string>
#include <iostream>
#include <iomanip>

#include <chrono>

/*
mapdata.csvのファイルパスを与えるとchannel-map-simpleの動作テストをする
*/
int main(int argc, char* argv[]) {
    std::string input_file_path = argv[1];
    chmap::ChannelMapSimple& channel_map_simple = chmap::ChannelMapSimple::get_instance();
    channel_map_simple.initialize(input_file_path);

    std::cout << "\n[in simple_skeleton.cpp] ChannelMapSimple initialized." << std::endl;
    // channel_map_simple.printAllItemsFE();
    // channel_map_simple.printAllItemsDET();
    channel_map_simple.checkDuplicateFEIDs();

    uint8_t test_ip3rd_T1right = 0x02;
    uint8_t test_ip4th_T1right = 0xAA;
    uint16_t test_ch_T1right = 12;

    std::cout << "\n[in simple_skeleton.cpp] Testing getDETItem for FE id composed of ip3rd=" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(test_ip3rd_T1right) << ", ip4th=" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(test_ip4th_T1right) << ", ch=" << std::dec << test_ch_T1right  << std::endl;
    chmap::ChannelMapSimpleItem_DET& det_item_T1right = channel_map_simple.getDETItem(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right);
    std::cout << "\tDET item for FE id: " << std::hex << std::setw(8) << std::setfill('0') << ((static_cast<uint32_t>(test_ip3rd_T1right) << 16) | (static_cast<uint32_t>(test_ip4th_T1right) << 8) | test_ch_T1right) << std::dec << std::endl;
    std::cout << "  name: " << std::hex << std::setw(8) << std::setfill('0') << det_item_T1right.name << ",which is \"" << static_cast<char>((det_item_T1right.name >> 24) & 0xFF) << static_cast<char>((det_item_T1right.name >> 16) & 0xFF) << static_cast<char>((det_item_T1right.name >> 8) & 0xFF) << static_cast<char>(det_item_T1right.name & 0xFF) << "\" in char"<< std::endl;
    std::cout << "  plane: " << std::hex << std::setw(4) << std::setfill('0') << det_item_T1right.plane << ",which is \"" << static_cast<char>((det_item_T1right.plane >> 8) & 0xFF) << static_cast<char>(det_item_T1right.plane & 0xFF) << "\" in char" << std::endl;
    std::cout << "  segment: " << static_cast<uint32_t>(det_item_T1right.segment) << std::endl;
    std::cout << "  channel: " << std::hex << std::setw(8) << std::setfill('0') << det_item_T1right.channel << ",which is \"" << static_cast<char>((det_item_T1right.channel >> 24) & 0xFF) << static_cast<char>((det_item_T1right.channel >> 16) & 0xFF) << static_cast<char>((det_item_T1right.channel >> 8) & 0xFF) << static_cast<char>(det_item_T1right.channel & 0xFF) << "\" in char" << std::endl;

    auto t0 = std::chrono::high_resolution_clock::now();
    const int n_trials = 1000000;
    uint32_t det_name;
    uint16_t det_plane;
    uint8_t det_segment;
    uint32_t det_channel;
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelMapSimpleItem_DET& det_item = channel_map_simple.getDETItem(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right);
        det_name = det_item.name;
        det_plane = det_item.plane;
        det_segment = det_item.segment;
        det_channel = det_item.channel;
    }
    auto t1 =  std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = t1 - t0;
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of getDETItem in " << elapsed.count() << " microseconds." << std::endl;
    std::cout << "Average time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;
    return 0;
}