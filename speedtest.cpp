#include <cstdlib>
#include <iostream>
#include <chrono>

#include <channel_map.hpp>

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

    /*
        test use for mapdata.csv
    */
    if (input_csv == "mapdata.csv") {
    {
        chmap::ChannelTuple det("utof", 0, 0, "left", 0); // Left readout of UTOF


        auto t0 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
            const auto& fe = channel_map.get("fe", det);
        }
        const auto& fe = channel_map.get("fe", det);
        auto t1 = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::micro>(t1 - t0).count();


        auto t2 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
            const auto& utof_left_det = channel_map.get("detector", fe);
        }
        auto t3 = std::chrono::high_resolution_clock::now();
        double elapsed_inv = std::chrono::duration<double, std::micro>(t3 - t2).count();
        const auto& utof_left_det = channel_map.get("detector", fe);


        auto t4 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
        uint64_t femId = std::get<chmap::number_t>(fe.at("id"));
        }
        auto t5 = std::chrono::high_resolution_clock::now();
        double elapsed_access = std::chrono::duration<double, std::micro>(t5 - t4).count();


        auto t6 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
            uint64_t femId = std::get<chmap::number_t>(fe.at(0));
        }
        auto t7 = std::chrono::high_resolution_clock::now();
        double elapsed_access_index = std::chrono::duration<double, std::micro>(t7 - t6).count();


        auto t8 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
            std::string det_name = std::get<std::string>(utof_left_det.at("id"));
        }
        auto t9 = std::chrono::high_resolution_clock::now();
        double elapsed_access_det = std::chrono::duration<double, std::micro>(t9 - t8).count();


        auto t10 = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000000; ++i){
            std::string det_name = std::get<std::string>(utof_left_det.at(0));
        }
        auto t11 = std::chrono::high_resolution_clock::now();
        double elapsed_access_det_index = std::chrono::duration<double, std::micro>(t11 - t10).count();

        std::cout << "Results of 1,000,000 accesses:\n"
                  << "\tget fe from det: " << elapsed/1000000.0 << " us / access\n"
                  << "\tget det from fe: " << elapsed_inv/1000000.0 << " us / access\n"
                  << "\taccess fe.at(\"id\"): " << elapsed_access/1000000.0 << " us / access\n"
                  << "\taccess fe.at(0): " << elapsed_access_index/1000000.0 << " us / access\n"
                  << "\taccess det.at(\"id\"): " << elapsed_access_det/1000000.0 << " us / access\n"
                  << "\taccess det.at(0): " << elapsed_access_det_index/1000000.0 << " us / access\n"
                  << std::endl;


      const auto& utof_left_det = channel_map.get("detector", fe);
      for(const auto& e : utof_left_det){
        std::cout << "element : " << e << std::endl;
      }

      std::cout << "det = " << det << std::endl
                << "-> fe = " << std::hex << fe << std::endl;

      for (const auto& e : fe) {
        std::cout << "element : " << e << std::endl;
      }


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
