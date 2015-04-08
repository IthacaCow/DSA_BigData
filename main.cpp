#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdio>
#include <set>
#include <string>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include "bigData.hpp"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::minutes minutes;

const int MAX_NUM_ENTRIES = 149639105;
const int MAX_NUM_USERS   = 22023547;
const int MAX_NUM_ADS     = 641707;
const int MAX_NUM_THREADS = 4;

const std::string command_quit      = "quit";
const std::string command_clicked   = "clicked";
const std::string command_get       = "get";
const std::string command_impressed = "impressed";
const std::string command_profit    = "profit";

Data::Map dataMap; // Key --> Value 
User::Map userMap; // UserID --> User
Ad::Map   adMap;   // AdID --> Ad

void cleanUp(){
    for( auto&& it: userMap ){
        delete it.second;
    }
    for( auto&& it: adMap ){
        delete it.second;
    }
}
void get( Data::Key& key ){
   const Data::Value& v = dataMap[ key ]; 
   printf("%d %d\n", v.click, v.impression );
}
// output all (AdID, QueryID) pairs that user u has made at least one click
void clicked( uint32_t UserID ){
    User::Clicks& clicks = userMap[ UserID ]->clicks;
    for( User::Clicks::iterator c = clicks.begin(); c != clicks.end(); c++ ){
        printf("%d %d\n", (*c)->adID, (*c)->userID);
    }
}
//
// output the sorted (AdID), line by line, and its associated properties
// (DisplayURL), (AdvertiserID), (KeywordID), (TitleID), (DescriptionID) 
// that both users u1 and u2 has at least one impression on
//
void impressed( uint32_t UserID_1 , uint32_t UserID_2 ){
    User::Ads& user1 = userMap[ UserID_1 ]->impressions;
    User::Ads& user2 = userMap[ UserID_2 ]->impressions;

    User::Ads common;
    std::set_intersection(user1.begin(),user1.end(),user2.begin(),user2.end(),
                          std::inserter(common,common.begin()));
    for( User::Ads::iterator id = common.begin(); id != common.end(); id++ ){
        printf("%d\n",*id);
        auto& ad = adMap[ *id ];
        for( Ad::Information::iterator info = (ad->information).begin(); info != (ad->information).end(); info++ ){
            printf("%llu %d %d %d %d\n",(*info)->displayURL,
                                             ad->advertiserID,
                                        (*info)->keywordID,
                                        (*info)->titleID,
                                        (*info)->descriptionID);
        }
    }
}
// output the sorted (UserID), line by line, whose click-through-rate 
// (total click / total impression) on `AdID` is greater than or equal to `clickThroughRateLowerBound`.
void profit( uint32_t adID, double lowerBound ){
   Ad::ClickThroughTable& table = adMap[ adID ]->clickThroughTable;
   for( Ad::ClickThroughTable::iterator entry = table.begin(); entry != table.end(); entry++ ) {
        if( entry->second->rate >= lowerBound )
            printf("%d\n", entry->first);
   }
}

void buildClickThroughTable(){

    int numThreads = 2;
    std::vector<std::thread> threads;

    int segLength = dataMap.size() / numThreads;
    Data::Map::iterator from = dataMap.begin();
    Data::Map::iterator until = std::next(from,segLength);
    for (int i = 0;;) {
        std::thread thread(User::buildClicks, from, until);
        threads.push_back(std::move(thread));

        if( ++i == numThreads )
            break;

        from = until;
        advance(until,segLength);
    }

    if( until != dataMap.end() ){
        User::buildClicks( until, dataMap.end() );
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

}
void calculateRate(){

    int numThreads = 2;
    std::vector<std::thread> threads;

    int segLength = adMap.size() / numThreads;
    Ad::Map::iterator from = adMap.begin();
    Ad::Map::iterator until = next(from,segLength);
    for (int i = 0;;) {
        std::thread thread(Ad::calculateClickThroughRate, from, until);
        threads.push_back(std::move(thread));

        if( ++i == numThreads )
            break;

        from = until;
        advance(until,segLength);
    }

    if( until != adMap.end() ){
        Ad::calculateClickThroughRate( until , adMap.end() );
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

template <class int_type>
inline int_type strToInt( char** str ){
    int_type x = 0;
    while( isspace(**str) )(*str)++;
    while( !isspace(**str) ){
        x = x*10 + **str - '0';
        (*str)++;
    }
    return x;
}
inline size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;   
}
void read_data(const char* fileName){
    size_t filesize = getFilesize(fileName);
    int fd = open(fileName, O_RDONLY, 0);
    assert(fd != -1);

    auto mmap_t0 = Clock::now();

    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    assert(mmappedData != NULL);

    char *p = (char*)mmappedData;

    auto mmap_t1 = Clock::now();

    minutes m = std::chrono::duration_cast<minutes>(mmap_t1 - mmap_t0);
    std::cout <<"mmap: time elapsed: "<< m.count() << "min\n";

    for (int i = 0 ; i < MAX_NUM_ENTRIES ; i++) {
       auto adInfo = new Ad::AdInfo();
       auto ad     = new Ad::Ad();
       Data::Key   key;
       Data::Value value;

       value.click           = strToInt<uint16_t>(&p);
       value.impression      = strToInt<uint32_t>(&p);
       adInfo->displayURL    = strToInt<ullint_t>(&p);
       key.adID              = strToInt<uint32_t>(&p);
       ad->advertiserID      = strToInt<ushint_t>(&p);
       key.dept              = strToInt<uint8_t >(&p);
       key.position          = strToInt<uint8_t >(&p);
       key.queryID           = strToInt<uint32_t>(&p);
       adInfo->keywordID     = strToInt<uint32_t>(&p);
       adInfo->titleID       = strToInt<uint32_t>(&p);
       adInfo->descriptionID = strToInt<uint32_t>(&p);
       key.userID            = strToInt<uint32_t>(&p);

       auto insertedData = dataMap.emplace( key, value ) ;
       // The key is already in map 
       if( !insertedData.second )
           insertedData.first->second.update( value );

       auto insertedAdContent = adMap.emplace( key.adID, ad ).first->second;
       insertedAdContent->information.push_back( adInfo ); 
       insertedAdContent->updateClickTable( key.userID, value );

       /* add entry to userMap */
       auto insertedUser = userMap.emplace( key.userID, new User::User() ).first->second;
       insertedUser->impressions.insert( key.adID ); // The user has at least one impression
       if( value.click ) // If there's at least one click
           insertedUser->clicks.push_back( &(insertedData.first->first) );

    }
}
int main(int argc, char *argv[])
{
    dataMap.reserve( MAX_NUM_ENTRIES ); 
    userMap.reserve( MAX_NUM_USERS );
      adMap.reserve( MAX_NUM_ADS );

    auto t0 = Clock::now();
    read_data(argv[1]);
    auto t1 = Clock::now();

    minutes m = std::chrono::duration_cast<minutes>(t1 - t0);
    std::cout <<"Read data: time elapsed: "<< m.count() << "min\n";


    std::string command;

    
    uint32_t u,u1,a,q;
    uint8_t p,d;
    double theta;
    while( std::cin >> command && command != command_quit ){
        std::cout<<"********************\n";
        if( command == command_get ){
            std::cin >> u >> a >> q >> p >> d;
            Data::Key key( u,a,q,p,d );
            get( key );
        }
        else if( command == command_clicked ){
            std::cin >> u;
            clicked( u );
        }
        else if( command == command_impressed ){
            std::cin >> u >> u1;
            impressed( u , u1 );
        }
        else if( command == command_profit ){
            std::cin >> a >> theta;
            profit( a , theta );
        }
        std::cout<<"********************\n";
    }

    cleanUp();
    return 0;
}
