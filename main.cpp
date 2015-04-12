#include <iostream>
#include <chrono>
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
#include "debug.hpp"
#include "bigData.hpp"

using namespace std;

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::minutes minutes;

const int MAX_NUM_ENTRIES = 149639105;
// const int MAX_NUM_ENTRIES = 11;
const int MAX_NUM_USERS   = 22023547;
const int MAX_NUM_ADS     = 641707;
const int MAX_NUM_THREADS = 4;

const std::string command_quit      = "quit";
const std::string command_clicked   = "clicked";
const std::string command_get       = "get";
const std::string command_impressed = "impressed";
const std::string command_profit    = "profit";

Data::Map  dataMap; // Key --> Value
Ad::Map    adMap;   // AdID --> Ad
User::User userTable[ MAX_USER_ID ];

void calculateRate(Ad::ClickThroughTable& table);

void cleanUp(){
    for( auto&& it: adMap ){
        delete it.second;
    }
}
void get( Data::Key& key ){
   const Data::Value& v = dataMap[ key ]; 
   printf("%d %d\n", v.click, v.impression );
}
// output all (AdID, QueryID) pairs that user u has made at least one click
void licked( uint32_t UserID ){
    User::Clicks& clicks = userTable[ UserID ].clicks;
    for( User::Clicks::iterator c = clicks.begin(); c != clicks.end(); c++ ){
        printf("%d %d\n", c->first, c->second); // AdID , QueryID
    }
}
//
// output the sorted (AdID), line by line, and its associated properties
// (DisplayURL), (AdvertiserID), (KeywordID), (TitleID), (DescriptionID) 
// that both users u1 and u2 has at least one impression on
//
void impressed( uint32_t UserID_1 , uint32_t UserID_2 ){
    User::Ads& user1 = userTable[ UserID_1 ].impressions;
    User::Ads& user2 = userTable[ UserID_2 ].impressions;

    User::Ads common;
    std::set_intersection(user1.begin(),user1.end(),user2.begin(),user2.end(),
                          std::inserter(common,common.begin()));
    for( User::Ads::iterator id = common.begin(); id != common.end(); id++ ){
        printf("%d\n",*id);
        auto& ad = adMap[ *id ];

        /* TODO Eliminate duplicate */
        for( Ad::Information::iterator info = (ad->information).begin(); info != (ad->information).end(); info++ ){
            printf("%llu %d %d %d %d\n",(*info)->displayURL,
                                        (*info)->advertiserID,
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
        if( entry->second->clickCount >= (double)entry->second->impressionCount * lowerBound )
            printf("%d\n", entry->first);
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

    auto mmap_t0 = Clock::now();

    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

    if( mmappedData == MAP_FAILED ){
        cout<<"mmap failed\n";
        exit(1);
    }

    char *p = (char*)mmappedData;

    auto mmap_t1 = Clock::now();

    minutes m = std::chrono::duration_cast<minutes>(mmap_t1 - mmap_t0);
    std::cout <<"mmap: time elapsed: "<< m.count() << "min\n";

    std::cout<<"Click Impress URL_ID Ad_ID AdverID Dept Pos QueryID KeyWordID TitleID DescriptID UserID"<<std::endl;
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

  // std::cout<< value.click << " "
           // << value.impression<< " "
           // << adInfo->displayURL<< " "
           // << key.adID<< " "
           // << adInfo->advertiserID<< " "
           // << (int)key.dept<< " "
           // << (int)key.position<< " "
           // << key.queryID<< " "
           // << adInfo->keywordID<< " "
           // << adInfo->titleID<< " "
           // << adInfo->descriptionID<< " "
           // << key.userID<< " \n" ;

       /* If duplicate found? */
       /* Implement a duplicate finder at request time */
       auto insertedAd = adMap.emplace( key.adID, ad );
       if( !insertedAd.second ){
           // std::cout<<" Ad Entry exist! \n";
           delete ad;
       }
       else{
           insertedAd.first->second->information.push_back( adInfo );
       }

       /* add entry to userTable */
       userTable[ key.userID ].impressions.insert( key.adID );

       // printImpression( userTable[ key.userID ].impressions );

       auto insertedData = dataMap.emplace( key, value ) ;
       // The key is already in map 
       if( !insertedData.second ){
           // std::cout<<"Data Entry exist! \n";
           insertedData.first->second.update( value );
           if( value.click && userTable[ key.userID ].clicks.empty() ){
               // cout<<" At least one click"<<std::endl;
               userTable[ key.userID ].clicks.push_back( std::pair<uint32_t,uint32_t>(key.adID,key.queryID) ); 
               // Insert AdID, QueryID pair
               
               // printClick( userTable[ key.userID ].clicks );

           }
           continue;
       }

       if( value.click ){ // If there's at least one click
           // cout<<" At least one click"<<std::endl;
           userTable[ key.userID ].clicks.push_back( std::pair<uint32_t,uint32_t>(key.adID,key.queryID) ); 
           // Insert AdID, QueryID pair
           
           // printClick( userTable[ key.userID ].clicks );
       }

       if( i % 100000 == 0 ) {
           printf("%d\n",i);
       }

    }
}
int main(int argc, char *argv[])
{
    dataMap.reserve( MAX_NUM_ENTRIES ); 
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
