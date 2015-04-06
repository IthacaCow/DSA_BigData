#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
#include <cstdio>
#include <set>
#include <unordered_map>
#include "bigData.hpp"

const int MAX_NUM_ENTRIES       = 149639105;
const int MAX_NUM_USERS         = 22023547;
const int MAX_NUM_ADS           = 641707;
const int MAX_NUM_THREADS       = 4;
const int DATA_FILE_NUM_OF_LINE = 149639105;

const string command_quit      = "quit";
const string command_clicked   = "clicked";
const string command_get       = "get";
const string command_impressed = "impressed";
const string command_profit    = "profit";

Data::Map dataMap; // Key --> Value 
User::Map userMap; // UserID --> User
Ad::Map   adMap;   // AdID --> Ad

void get( Data::Key key ){
   const Data::Value& v = dataMap[ key ]; 
   printf("%d %d\n", v.click, v.impression );
}
// output all (AdID, QueryID) pairs that user u has made at least one click
void clicked( uint32_t UserID ){
    User::Clicks& clicks = userMap[ UserID ]->clicks;
    for( User::Clicks::iterator c = clicks.begin(); c != clicks.end(); c++ ){
        printf("%d %d\n", c->adID, c->userID);
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
    for( Ads::iterator ad = common.begin(); ad != common.bend(); ad++ ){
        printf("%d\n",ad->id);
        for( Information::iterator info = (ad->information).begin(); info != (ad->information).end(); info++ ){
            printf("%llu %d %d %d %d\n",info->displayURL,
                                          ad->advertiserID,
                                        info->keywordID,
                                        info->titleID,
                                        info->descriptionID);
        }
    }
}
// output the sorted (UserID), line by line, whose click-through-rate 
// (total click / total impression) on `AdID` is greater than or equal to `clickThroughRateLowerBound`.
void profit( uint32_t AdID, double lowerBound ){
   Ad::ClickThroughTable& table = adMap[ adID ]->clickThroughTable;
   for( ClickThroughTable::iterator entry = table.begin(); entry != table.end(); entry++ ) {
        if( entry->second->rate >= lowerBound )
            printf("%d\n", entry->first);
   }
}

void buildClickThroughTable(){

    int numThreads = 2;
    std::vector<std::thread> threads;

    int segLength = dataMap.size() / numThreads;
    Data::Map::const_iterator from = dataMap.cbegin()
    Data::Map::const_iterator until = next(from,segLength);
    for (int i = 0;;) {
        std::thread thread(User::buildClicks, from, until);
        threads.push_back(std::move(thread));

        if( ++i == numThreads )
            break;

        from = until;
        advance(until,segLength);
    }

    if( end != dataMap.end() ){
        User::buildClicks( from, dataMap.end() );
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

}
void calculateRate(){

    int numThreads = 2;
    std::vector<std::thread> threads;

    int segLength = adMap.size() / numThreads;
    Ad::Map::iterator from = adMap.cbegin()
    Ad::Map::iterator until = next(from,segLength);
    for (int i = 0;;) {
        std::thread thread(Ad::calculateClickThroughRate, from, until);
        threads.push_back(std::move(thread));

        if( ++i == numThreads )
            break;

        from = until;
        advance(until,segLength);
    }

    if( end != adMap.end() ){
        Ad::calculateClickThroughRate( end , adMap.end() );
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
    size_t filesize = getFilesize(argv[1]);
    int fd = open(argv[1], O_RDONLY, 0);
    assert(fd != -1);

    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    assert(mmappedData != NULL);

    char *p = (char*)mmappedData;


    for (int i = 0 ; i < DATA_FILE_NUM_OF_LINE ; i++) {
       std::unique_ptr<Ad::AdInfo> adInfo(new Ad::AdInfo());
       std::unique_ptr<Ad::Ad>     ad(new Ad::Ad());
       Data::Key   key;
       Data::Value value;

       value.click           = strToInt(&p);
       value.impression      = strToInt(&p);
       adInfo->displayURL    = strToInt(&p);
       key.adID              = strToInt(&p);
       ad->advertiserID      = strToInt(&p);
       key.dept              = strToInt(&p);
       key.position          = strToInt(&p);
       key.queryID           = strToInt(&p);
       adInfo->keywordID     = strToInt(&p);
       adInfo->titleID       = strToInt(&p);
       adInfo->descriptionID = strToInt(&p);
       key.userID            = strToInt(&p);

       auto insertedData = dataMap.emplace( key, value ) ;
       // The key is already in map 
       if( !insertedData.second )
           insertedData.first->second->update( value );

       auto insertedAdContent = adMap.emplace( key.adID, std::move(ad) ).first->second;
       insertedAdContent->information.push_back( std::move(adInfo) ); 
       insertedAdContent->updateClickTable( key.userID, value );

       /* add entry to userMap */
       auto insertedUser = userMap.emplace( key.userID, unique_ptr<User::User>(new User::User()) ).first->second;
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

    read_data(argv[1]);

    string command;
    while( cin >> command && command != command_quit ){
        cout<<"********************\n";
        if( command == command_get ){
            cin >> u >> a >> q >> p >> d;
            Data::Key key( u,a,q,p,d );
            get( key );
        }
        else if( command == command_clicked ){
            cin >> UserID;
            clicked( UserID );
        }
        else if( command == command_impressed ){
            cin >> UserID_1 >> UserID_2;
            impressed( UserID_1 , UserID_2 );
        }
        else if( command == command_profit ){
            cin >> AdID >> clickThroughRateLowerBound;
            profit( AdID , clickThroughRateLowerBound );
        }
        cout<<"********************\n";
    }
    return 0;
}
