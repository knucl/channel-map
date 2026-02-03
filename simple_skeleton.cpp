#include <channel_map_simple.hpp>
#include <channel_map_simple_item.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <channel_map.hpp>
#include <debug_print.hpp>
#include <channel_tuple.hpp>
#include <element.hpp>
#include <cstdlib>
#include <variant>

#include <chrono>

#define general_chmap 1

/*
mapdata.csvのファイルパスを与えるとchannel-map-simpleの動作テストをする
*/
int main(int argc, char* argv[]) {
    std::string input_file_path = argv[1];
    chmap::ChannelMapSimple& channel_map_simple = chmap::ChannelMapSimple::get_instance();
    channel_map_simple.initialize(input_file_path);
    #if general_chmap
    chmap::ChannelMap& channel_map = chmap::ChannelMap::get_instance();
    channel_map.initialize(input_file_path);
    chmap::ChannelTuple test_det1("utof", 0, 0, "right", 0);
    chmap::ChannelTuple test_det2("t0", 0, 0, "left", 0);
    chmap::ChannelTuple test_det3("bdc", "X", 1, 1, 0);
    const auto& fe1 = channel_map.get("fe", test_det1);
    const auto& fe2 = channel_map.get("fe", test_det2);
    const auto& fe3 = channel_map.get("fe", test_det3);
    std::cout << "\n[in simple_skeleton.cpp] General ChannelMap results:" << std::endl;
    std::cout << "  for det: " << test_det1 << ", fe: " << fe1 << std::endl;
    std::cout << "  for det: " << test_det2 << ", fe: " << fe2 << std::endl;
    std::cout << "  for det: " << test_det3 << ", fe: " << fe3 << std::endl;
    uint64_t fe1_id = static_cast<uint64_t>(std::get<chmap::number_t>(fe1["id"]));
    uint16_t fe1_channel = static_cast<uint16_t>(std::get<chmap::number_t>(fe1["channel"]));
    uint64_t fe2_id = static_cast<uint64_t>(std::get<chmap::number_t>(fe2["id"]));
    uint16_t fe2_channel = static_cast<uint16_t>(std::get<chmap::number_t>(fe2["channel"]));
    uint64_t fe3_id = static_cast<uint64_t>(std::get<chmap::number_t>(fe3["id"]));
    uint16_t fe3_channel = static_cast<uint16_t>(std::get<chmap::number_t>(fe3["channel"]));
    uint8_t fe1_ip4th = static_cast<uint8_t>((fe1_id) & 0xFF);
    uint8_t fe1_ip3rd = static_cast<uint8_t>((fe1_id >> 8) & 0xFF);
    uint8_t fe2_ip4th = static_cast<uint8_t>((fe2_id) & 0xFF);
    uint8_t fe2_ip3rd = static_cast<uint8_t>((fe2_id >> 8) & 0xFF);
    uint8_t fe3_ip4th = static_cast<uint8_t>((fe3_id) & 0xFF);
    uint8_t fe3_ip3rd = static_cast<uint8_t>((fe3_id >> 8) & 0xFF);
    #endif

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
    std::cout << "\tAverage time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;

    #if general_chmap
    std::cout << "\n[in simple_skeleton.cpp] Now testing getDETItem for several FE ids obtained from General ChannelMap." << std::endl;
    t0 = std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelMapSimpleItem_DET& det_item = channel_map_simple.getDETItem(fe1_ip3rd, fe1_ip4th, fe1_channel);
        det_name = det_item.name;
        det_plane = det_item.plane;
        det_segment = det_item.segment;
        det_channel = det_item.channel;
    }
    t1 =  std::chrono::high_resolution_clock::now();
    elapsed = t1 - t0;
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of getDETItem (using General ChannelMap) in " << elapsed.count() << " microseconds." << std::endl;
    std::cout << "\tAverage time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;

    t0 = std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelMapSimpleItem_DET& det_item = channel_map_simple.getDETItem(fe2_ip3rd, fe2_ip4th, fe2_channel);
        det_name = det_item.name;
        det_plane = det_item.plane;
        det_segment = det_item.segment;
        det_channel = det_item.channel;
    }
    t1 =  std::chrono::high_resolution_clock::now();
    elapsed = t1 - t0;
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of getDETItem (using General ChannelMap) in " << elapsed.count() << " microseconds." << std::endl;
    std::cout << "\tAverage time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;

    t0 = std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelMapSimpleItem_DET& det_item = channel_map_simple.getDETItem(fe3_ip3rd, fe3_ip4th, fe3_channel);
        det_name = det_item.name;
        det_plane = det_item.plane;
        det_segment = det_item.segment;
        det_channel = det_item.channel;
    }
    t1 =  std::chrono::high_resolution_clock::now();
    elapsed = t1 - t0;
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of getDETItem (using General ChannelMap) in " << elapsed.count() << " microseconds." << std::endl;
    std::cout << "\tAverage time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;

    t0 = std::chrono::high_resolution_clock::now();
    for(int i=0; i<n_trials; i++) {
        chmap::ChannelMapSimpleItem_DET& det_item = channel_map_simple.getDETItem(0xFF, 0xFF, 0xFFFF);
        det_name = det_item.name;
        det_plane = det_item.plane;
        det_segment = det_item.segment;
        det_channel = det_item.channel;
    }
    t1 =  std::chrono::high_resolution_clock::now();
    elapsed = t1 - t0;
    std::cout << "\n[in simple_skeleton.cpp] Performed " << n_trials << " trials of getDETItem (using General ChannelMap) in " << elapsed.count() << " microseconds." << std::endl;
    std::cout << "\tAverage time per getDETItem call: " << (elapsed.count() / n_trials) << " microseconds." << std::endl;

    #endif
    return 0;
}