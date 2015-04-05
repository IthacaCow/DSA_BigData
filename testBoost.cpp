#include <boost/spirit/include/qi.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <chrono>

namespace qi = boost::spirit::qi;

typedef std::vector<uint32_t> data_t;

using hrclock = std::chrono::high_resolution_clock;

int main(int argc, char** argv) {
    if (argc<2) return 255;
    data_t data;
    data.reserve(4392580288);   // for the  45 GiB file benchmark
    // data.reserve(402653284); // for the 4.5 GiB file benchmark

    boost::iostreams::mapped_file mmap(argv[1], boost::iostreams::mapped_file::readonly);
    auto f = mmap.const_data();
    auto l = f + mmap.size();

    using namespace qi;

    auto start_parse = hrclock::now();
    bool ok = phrase_parse(f,l,int_parser<uint32_t, 10>() % eol, blank, data);
    auto stop_time = hrclock::now();

    if (ok)   
        std::cout << "Parse success\n";
    else 
        std::cerr << "Parse failed at #" << std::distance(mmap.const_data(), f) << " around '" << std::string(f,f+50) << "'\n";

    if (f!=l) 
        std::cerr << "Trailing unparsed: " << std::distance(f,l) << " characters\n";

    std::cout << "Data.size():   " << data.size() << "\n";
    std::cout << "Time taken by parsing: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time-start_parse).count() / 1000.0 << "s\n";
}
