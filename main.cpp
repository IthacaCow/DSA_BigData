#include <iostream>
#include <parallel/algorithm>
#include <vector>
#include <iterator>
#include <cstdio>
#include <set>
#include <parallel/algorithm>
#include <string>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bigData.hpp"

#ifdef DEBUG
#include "debug.hpp"
#endif
#ifdef BENCHMARK 
#include <chrono>
#endif

using namespace std;

#ifdef BENCHMARK 
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::seconds seconds;
#endif

const int MAX_NUM_ENTRIES = 149639105;
// const int MAX_NUM_ENTRIES = 11;
const int MAX_NUM_USERS   = 22023547;
const int MAX_NUM_ADS     = 641707;

const std::string command_quit      = "quit";
const std::string command_clicked   = "clicked";
const std::string command_get       = "get";
const std::string command_impressed = "impressed";
const std::string command_profit    = "profit";

Data::Map  dataMap; // Key --> Value
Ad::Map    adMap;   // AdID --> Ad
Ad::InfoTable infoTable;
User::User userTable[ MAX_USER_ID ];
std::set<uint32_t> clickedQueryTable;

void cleanUp(){
    for( auto&& it: adMap ){
        delete it.second;
    }
    for( auto&& it: infoTable ){
        delete it;
    }
}
void get(void){
   Data::Key k;
   scanf("%d%d%d%d%d",&k.userID,&k.adID,&k.queryID,&k.position,&k.dept);
   const Data::Value& v = dataMap[k]; 
   printf("%d %d\n", v.click, v.impression );
}

bool clickedComparator( const std::pair<uint32_t,uint32_t>& lhs, const std::pair<uint32_t,uint32_t>& rhs ){
    return ( lhs.first < rhs.first ) || ( lhs.first == rhs.first && lhs.second < rhs.second );
}
// output all (AdID, QueryID) pairs that user u has made at least one click
void clicked(void){
    uint32_t UserID;
    scanf("%d",&UserID);

    User::Clicks& clicks = userTable[ UserID ].clicks;
    if(clickedQueryTable.find(UserID) == clickedQueryTable.end()){
        __gnu_parallel::sort( clicks.begin(),clicks.end(),clickedComparator );   
        clickedQueryTable.insert( UserID );
    } 
    auto it = clicks.begin();
    User::Clicks::iterator head;
    while (it != clicks.end()){
        printf("%d %d\n", it->first, it->second); // AdID , QueryID
        head = it+1;
        while (head != clicks.end() && it->first == head->first && it->second == head->second)
           ++head; 
        it = head;
    }
}
//
// output the sorted (AdID), line by line, and its associated properties
// (DisplayURL), (AdvertiserID), (KeywordID), (TitleID), (DescriptionID) 
// that both users u1 and u2 has at least one impression on
//
void impressed(void){
    uint32_t UserID_1,UserID_2;
    scanf("%d%d",&UserID_1,&UserID_2);
    auto& user1 = userTable[ UserID_1 ].impressions;
    auto& user2 = userTable[ UserID_2 ].impressions;
    /* iterator point to pair< adID , adInfo > */
    auto  it1 = user1.begin(); auto end1 = user1.end();
    auto  it2 = user2.begin(); auto end2 = user2.end();
    
    while( it1 != end1 && it2 != end2 ){
        if( it1->first < it2->first ) ++it1;
        else if( it2->first < it1->first ) ++it2;
        else{
            auto id = it1->first;
            printf("%d\n",id);

            set< Ad::AdInfo*, Ad::InfoComparator > infoUnion;
            while( it1 != end1 && it1->first == id ){
                infoUnion.insert( it1->second );
                ++it1;
            }
            while( it2 != end1 && it2->first == id ){
                infoUnion.insert( it2->second );
                ++it2;
            }

            for( auto it = infoUnion.begin(); it != infoUnion.end() ; ++it )
                printf("\t%llu %d %d %d %d\n",(*it)->displayURL,
                                              (*it)->advertiserID,
                                              (*it)->keywordID,
                                              (*it)->titleID,
                                              (*it)->descriptionID);
        }
    }
    
}
// output the sorted (UserID), line by line, whose click-through-rate 
// (total click / total impression) on `AdID` is greater than or equal to `clickThroughRateLowerBound`.
void profit(void){
   uint32_t adID;
   double lowerBound;
   scanf("%d%lf",&adID,&lowerBound);

   auto it = adMap.find( adID );
   if( it == adMap.end() ){
        printf("%d\n", entry->first); // UserID
   }
   else{
       auto& table = it->clickThroughTable;
       for( auto entry = table.begin(); entry != table.end(); ++entry ) {
            if( entry->second.clickCount >= (double)entry->second.impressionCount * lowerBound ){
                if( entry->second.impressionCount == 0 && lowerBound > 0.0 )
                    continue;
                printf("%d\n", entry->first); // UserID
            }
       }
   }
}

template <class int_type>
inline void strToInt( int_type& x, char** str ){
    x = 0;
    while( isspace(**str) )(*str)++;
    while( !isspace(**str) ){
        x = x*10 + **str - '0';
        (*str)++;
    }
}
inline size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;   
}
void read_data(const char* fileName){
    size_t filesize = getFilesize(fileName);
    int fd = open(fileName, O_RDONLY, 0);
    if( fd == -1 ){
        cout<<"Fail to read data input\n";
        exit(1);
    }

#ifdef DEBUG
    auto mmap_t0 = Clock::now();
#endif

    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

    if( mmappedData == MAP_FAILED ){
        cout<<"mmap failed\n";
        exit(1);
    }

    char *p = (char*)mmappedData;


#ifdef DEBUG
    auto mmap_t1 = Clock::now();

    seconds m = std::chrono::duration_cast<seconds>(mmap_t1 - mmap_t0);
    std::cout <<"mmap: time elapsed: "<< m.count() << " seconds\n";
#endif

    for (int i = 0 ; i < MAX_NUM_ENTRIES ; i++) {
       auto adInfo = new Ad::AdInfo();
       auto ad     = new Ad::Ad();
       Data::Key   key;
       Data::Value value;

       strToInt<uint16_t>( value.click,           &p);
       strToInt<uint32_t>( value.impression,      &p);
       strToInt<ullint_t>( adInfo->displayURL,    &p);
       strToInt<uint32_t>( key.adID,              &p);
       strToInt<ushint_t>( adInfo->advertiserID,  &p);
       strToInt<uint8_t >( key.dept,              &p);
       strToInt<uint8_t >( key.position,          &p);
       strToInt<uint32_t>( key.queryID,           &p);
       strToInt<uint32_t>( adInfo->keywordID,     &p);
       strToInt<uint32_t>( adInfo->titleID,       &p);
       strToInt<uint32_t>( adInfo->descriptionID, &p);
       strToInt<uint32_t>( key.userID,            &p);

#ifdef DEBUG
  std::cout<< value.click << " "
           << value.impression<< " "
           << adInfo->displayURL<< " "
           << key.adID<< " "
           << adInfo->advertiserID<< " "
           << (int)key.dept<< " "
           << (int)key.position<< " "
           << key.queryID<< " "
           << adInfo->keywordID<< " "
           << adInfo->titleID<< " "
           << adInfo->descriptionID<< " "
           << key.userID<< " \n" ;
#endif

       auto insertedInfo = infoTable.insert( adInfo );
       if(!insertedInfo.second)
           delete adInfo;

       userTable[ key.userID ].impressions.insert( User::InfoPair( key.adID,*(insertedInfo.first)) );

#ifdef DEBUG
       printImpression( userTable[ key.userID ].impressions );
#endif

       auto insertedAd = adMap.emplace( key.adID, ad );
       insertedAd.first->second->clickThroughTable[ key.userID ].add( value );
       if( !insertedAd.second ){
           delete ad;
       }

       insertedAd.first->second->clickThroughTable.emplace( key.userID, Ad::ClickThrough() ).first->second.add( value );


       auto insertedData = dataMap.emplace( key, value ) ;
       // The key is already in map 
       if( !insertedData.second ){
           insertedData.first->second.update( value );

           if( value.click && userTable[ key.userID ].clicks.empty() ){
               userTable[ key.userID ].clicks.push_back( std::pair<uint32_t,uint32_t>(key.adID,key.queryID) ); 
               // Insert AdID, QueryID pair
           }
           continue;
       }

       if( value.click ) // If there's at least one click
           userTable[ key.userID ].clicks.push_back( std::pair<uint32_t,uint32_t>(key.adID,key.queryID) ); 

#ifdef DEBUG
           printClick( userTable[ key.userID ].clicks );
#endif

#ifdef BENCHMARK
       if( i % 1000000 == 0 ) {
           printf("%d % \n",i);
       }
#endif

       }

}


int main(int argc, char *argv[])
{
    dataMap.reserve( MAX_NUM_ENTRIES ); 
      adMap.reserve( MAX_NUM_ADS );

#ifdef BENCHMARK
    auto t0 = Clock::now();
#endif
    read_data(argv[1]);
#ifdef BENCHMARK
    auto t1 = Clock::now();
#endif

#ifdef BENCHMARK
    seconds m = std::chrono::duration_cast<seconds>(t1 - t0);
    std::cout <<"Read data: time elapsed: "<< m.count() << " seconds\n";
#endif


    std::string command;

    
#ifdef BENCHMARK
    auto t0_process = Clock::now();
#endif

    while( std::cin >> command && command != command_quit ){
        std::cout<<"********************\n";
        if( command == command_get ){
            get();
        }
        else if( command == command_clicked ){
            clicked();
        }
        else if( command == command_impressed ){
            impressed();
        }
        else if( command == command_profit ){
            profit();
        }
        std::cout<<"********************\n";
    }

#ifdef BENCHMARK
    auto t1_process = Clock::now();
    seconds m1 = std::chrono::duration_cast<seconds>(t1_process - t0_process);
    std::cout <<"Query data: time elapsed: "<< m1.count() << " seconds\n";
    std::cout <<"Total time elapsed: "<< m.count() + m1.count() << " seconds\n";
#endif

    cleanUp();
    return 0;
}
