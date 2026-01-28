#include <cstdlib>
#include <iostream>
#include <chrono>

#include <channel_map.hpp>
#include <debug_print.hpp>

#define SPEEDTEST 1

enum EArgs {
  kProcess, kInputCSV, kNArgs
};

int main(int argc, char* argv[]) {
    if (argc < kNArgs) {
    std::cout << "Usage: " << argv[kProcess] << " [InputCSV]" << std::endl;
    return EXIT_SUCCESS;
    }

    std::string input_csv(argv[kInputCSV]);

    auto& channel_map = chmap::ChannelMap::get_instance();
    channel_map.initialize(input_csv);

    chmap::SetLogLevel_thr(chmap::LogLevel::kERROR);
    /*
        test use for mapdata.csv
    */
    if (input_csv == "mapdata.csv") {
    {
        chmap::ChannelTuple det("utof", 0, 0, "left", 0); // Left readout of UTOF


        uint32_t numLoops = 1e6;
        auto t0 = std::chrono::high_resolution_clock::now();
        chmap::ChannelTuple fe;
        for(int i=0; i<numLoops; ++i){
            fe = channel_map.get("fe", det);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::micro>(t1 - t0).count();
        fe = channel_map.get("fe", det);


        auto t2 = std::chrono::high_resolution_clock::now();
        chmap::ChannelTuple utof_left_det;
        for(int i=0; i<numLoops; ++i){
            utof_left_det = channel_map.get("detector", fe);
        }
        auto t3 = std::chrono::high_resolution_clock::now();
        double elapsed_inv = std::chrono::duration<double, std::micro>(t3 - t2).count();
        utof_left_det = channel_map.get("detector", fe);


        chmap::ChannelTuple utof_right_det("utof", 0, 0, "right", 0); // Right readout of UTOF
        chmap::ChannelTuple utof_right_fe = channel_map.get("fe", utof_right_det);
        for(const auto& e : utof_right_fe){
            std::cout << "element : " << e << std::endl;
        }
        auto t12 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<numLoops; ++i){
            utof_right_fe = channel_map.get("fe", utof_right_det);
        }
        auto t13 = std::chrono::high_resolution_clock::now();
        double elapsed_utof_right = std::chrono::duration<double, std::micro>(t13 - t12).count();


        chmap::ChannelTuple utof_left_fe = channel_map.get("fe", utof_left_det);
        auto t14 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<numLoops; ++i){
            utof_left_det = channel_map.get("detector", utof_left_fe);
        }
        auto t15 = std::chrono::high_resolution_clock::now();
        double elapsed_utof_left = std::chrono::duration<double, std::micro>(t15 - t14).count();


        chmap::ChannelTuple t1_right_det("t1", 0, 0, "right", 0); // Right readout of T1
        chmap::ChannelTuple t1_right_fe = channel_map.get("fe", t1_right_det);
        auto t16 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<numLoops; ++i){
            t1_right_fe = channel_map.get("fe", t1_right_det);
        }
        auto t17 = std::chrono::high_resolution_clock::now();
        double elapsed_t1_right = std::chrono::duration<double, std::micro>(t17 - t16).count();


        chmap::ChannelTuple t1_left_det("t1", 0, 0, "left", 0); // Left readout of T1
        chmap::ChannelTuple t1_left_fe = channel_map.get("fe", t1_left_det);
        auto t18 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<numLoops; ++i){
            t1_left_fe = channel_map.get("fe", t1_left_det);
        }
        auto t19 = std::chrono::high_resolution_clock::now();
        double elapsed_t1_left = std::chrono::duration<double, std::micro>(t19 - t18).count();

        auto t4 = std::chrono::high_resolution_clock::now();
        uint64_t femId;
        for(int i=0; i<numLoops; ++i){
            femId = std::get<chmap::number_t>(fe.at("id"));
        }
        auto t5 = std::chrono::high_resolution_clock::now();
        double elapsed_access = std::chrono::duration<double, std::micro>(t5 - t4).count();


        auto t6 = std::chrono::high_resolution_clock::now();
        uint64_t femId_index;
        for(int i=0; i<numLoops; ++i){
            femId_index = std::get<chmap::number_t>(fe.at(0));
        }
        auto t7 = std::chrono::high_resolution_clock::now();
        double elapsed_access_index = std::chrono::duration<double, std::micro>(t7 - t6).count();


        auto t8 = std::chrono::high_resolution_clock::now();
        std::string det_name;
        for(int i=0; i<numLoops; ++i){
            det_name = std::get<std::string>(utof_left_det.at("id"));
        }
        auto t9 = std::chrono::high_resolution_clock::now();
        double elapsed_access_det = std::chrono::duration<double, std::micro>(t9 - t8).count();


        auto t10 = std::chrono::high_resolution_clock::now();
        std::string det_name_index;
        for(int i=0; i<numLoops; ++i){
            det_name_index = std::get<std::string>(utof_left_det.at(0));
        }
        auto t11 = std::chrono::high_resolution_clock::now();
        double elapsed_access_det_index = std::chrono::duration<double, std::micro>(t11 - t10).count();

        std::cout << "Results of " << numLoops << " accesses:\n"
                  << "\tget fe from det(utof left): " << elapsed/1000000.0 << " us / access\n"
                  << "\tget det(utof left) from fe: " << elapsed_inv/1000000.0 << " us / access\n"
                  << "\taccess fe.at(\"id\"): " << elapsed_access/1000000.0 << " us / access\n"
                  << "\taccess fe.at(0): " << elapsed_access_index/1000000.0 << " us / access\n"
                  << "\taccess det.at(\"id\"): " << elapsed_access_det/1000000.0 << " us / access\n"
                  << "\taccess det.at(0): " << elapsed_access_det_index/1000000.0 << " us / access\n"
                  << std::endl;

        std::cout << "UTOF right results of " << numLoops << " accesses:\n"
                  << "\tget det from fe: " << elapsed_utof_right/1000000.0 << " us / access\n"
                  << std::endl;
        std::cout << "UTOF left results of " << numLoops << " accesses:\n"
                  << "\tget fe from det: " << elapsed_utof_left/1000000.0 << " us / access\n"
                  << std::endl;
        std::cout << "T1 right results of " << numLoops << " accesses:\n"
                  << "\tget fe from det: " << elapsed_t1_right/1000000.0 << " us / access\n"
                  << std::endl;
        std::cout << "T1 left results of " << numLoops << " accesses:\n"
                  << "\tget fe from det: " << elapsed_t1_left/1000000.0 << " us / access\n"
                  << std::endl;


        for(const auto& e : utof_left_det){
        std::cout << "element : " << e << std::endl;
        }

        std::cout << "det = " << det << std::endl
                << "-> fe = " << std::hex << fe << std::endl;

        for (const auto& e : fe) {
        std::cout << "element : " << e << std::endl;
        }

        chmap::ChannelTuple void_fe(std::get<chmap::number_t>(utof_left_fe[0]), -1, std::get<chmap::number_t>(utof_left_fe[2]));
        std::cout << "void_fe = " << void_fe << std::endl;
        chmap::ChannelTuple void_det;
        auto t20 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<numLoops; ++i){
            void_det = channel_map.get("detector", void_fe);
        }
        auto t21 = std::chrono::high_resolution_clock::now();
        double elapsed_void_det = std::chrono::duration<double, std::micro>(t21 - t20).count();

        std::cout << "Void det results of " << numLoops << " accesses:\n"
                  << "\tget det from void fe: " << elapsed_void_det/1000000.0 << " us / access\n"
                  << std::endl;
      


      /*
        std::get<T> returns the value if the specified type T
        matches the current type of std::variant,
        otherwise it throws a std::bad_variant_access exception.
      */
      try {
        std::cout << std::string(80, '=') << std::endl
                  << "fe[0] : " << std::get<chmap::number_t>(fe[0])
                  << " fe[id] : " << std::get<chmap::number_t>(fe["id"])
                  << " fe[1] : " << std::get<chmap::number_t>(fe[1])
                  << " fe[channel] : " << std::get<chmap::number_t>(fe["channel"])
                  << " fe[2] : " << std::get<chmap::number_t>(fe[2])
                  << " fe[data] : " << std::get<chmap::number_t>(fe["data"])
                  << std::endl;
        auto value = std::get<chmap::number_t>(fe.at(0));
        std::cout << "fe[0] : " << value << std::endl;
      } catch (const std::bad_variant_access& e) {
        std::cerr << "Bad variant access : " << e.what() << std::endl;
      }

      /*
        std::get_if<T> returns a pointer to the value if the specified
        type T matches the current type of std::variant, nullptr otherwise.
      */
      std::cout << std::string(80, '=') << std::endl;
      for(int i=0; i<fe.size(); ++i) {
        if(auto p = std::get_if<chmap::number_t>(&fe.at(i))){
            std::cout << "fe[" << i << "] : " << *p << std::endl;
            } else if(auto p = std::get_if<std::string>(&fe.at(i))) {
            std::cout << "fe[" << i << "] : " << *p << std::endl;
            } else {
            std::cerr << "The variant does not hold number or string" << std::endl;
        }
      }


      if (auto value = std::get_if<chmap::number_t>(&fe.at(2))) {
        std::cout << "The value is number : " << *value << std::endl;
      } else if (auto value = std::get_if<std::string>(&fe.at(2))) {
        std::cout << "The value is string : " << *value << std::endl;
      } else {
        std::cerr << "The variant does not hold number or string" << std::endl;
      }
    }

    // /*
    //   If the key is missing, a null tuple is returned.
    //  */
    // {
    //   chmap::ChannelTuple det(0, 1, 2, 3);
    //   const auto& fe = channel_map.get("fe", det);
    //   std::cout << "fe = " << fe << std::endl;
    // }

    // {
    //   const auto& fe = channel_map.get("fe", {0, "1", 2, "3"});
    //   std::cout << "fe = " << fe << std::endl;
    // }
  }

  return EXIT_SUCCESS;
}
