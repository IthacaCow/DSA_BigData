#include <boost/spirit/include/qi.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>

int main()
{
    using namespace boost::lambda;
    typedef std::istream_iterator<int> in;
    boost::iostreams::mapped_file mmap("abc", boost::iostreams::mapped_file::readonly);

    std::for_each(
        in(std::cin), in(), std::cout << (_1 * 3) << " " );
}
