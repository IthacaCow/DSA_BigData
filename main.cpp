#include <iostream>
#include <vector>
#include <iterator>
#include <cstdio>
#include <set>
#include "bigData.hpp"

const int MAX_NUM_ENTRIES = 149639105;
const int MAX_NUM_USERS   =  22023547;
const int MAX_NUM_ADS     =    641707; 
const int MAX_NUM_THREADS =         4;

const string command_quit      = "quit";
const string command_clicked   = "clicked";
const string command_get       = "get";
const string command_impressed = "impressed";
const string command_profit    = "profit";

Data::Map dataMap(MAX_NUM_ENTRIES);             
User::Map userMap(MAX_NUM_USERS);

void threadFunction(std::function<void(const Key& key)> find, std::function<void(const Key& key)> insert) {

    for (int i = 0; i < numOperationsPerThread; i++) {
        std::uint32_t a = randNum(generator);
        std::uint32_t b = randNum(generator);
        std::uint32_t c = randNum(generator);
        Key key(a, b, c);
        if (randDouble(generator) < insertOperationsPercent) {
            insert(key);
        } else {
            find(key);
        }
    }

}

template<typename ThreadFunction, typename... Args>
void runThreads(int numThreads, ThreadFunction threadFunction, Args&&... threadFunctionArgs) {

    std::vector<std::thread> threads;

    for (int threadNo = 0; threadNo < MAX_NUM_THREADS; threadNo++) {
        std::thread thread(threadFunction, threadNo, std::forward<Args>(threadFunctionArgs)...);
        threads.push_back(std::move(thread));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

}
void runFcmm(int expectedNumEntries, int numThreads) {

    FcmmType map(expectedNumEntries);

    auto find = [&map](const Key& key) {
        map.find(key);
    };

    auto insert = [&map](const Key& key) {
        map.insert(key, calculate);
    };

    runThreads(find, insert);


}
void get( Data::Key key ){
   const Data::Value& v = dataMap.at( key ); 
   printf("%d %d\n", v.click, v.impression );
}
// output all (AdID, QueryID) pairs that user u has made at least one click
void clicked( uint32_t UserID ){
    const Queries& queries = (userMap.at( UserID ))->clicks;
    for( Queries::iterator q = queries.begin(); q != queries.end(); q++ ){
        printf("%d %d\n", q->ad->id, q->id);
    }
}
//
// output the sorted (AdID), line by line, and its associated properties
// (DisplayURL), (AdvertiserID), (KeywordID), (TitleID), (DescriptionID) 
// that both users u1 and u2 has at least one impression on
//
void impressed( uint32_t UserID_1 , uint32_t UserID_2 ){
    const Ads& user1 = (userMap.at( UserID_1 ))->impressions;
    const Ads& user2 = (userMap.at( UserID_2 ))->impressions;

    Ads common;
    std::set_intersection(user1.begin(),user1.end(),user2.begin(),user2.end(),
                          std::inserter(common,common.begin()));
    for( Ads::iterator ad = common.begin(); ad != common.bend(); ad++ ){
        printf("%d\n",ad->id);
        for( vector<AdInfo>::iterator info = (ad->information).begin(); info != (ad->information).end(); info++ ){
            printf("%d %d %d %d %d\n",info->displayURL,
                                      ad->advertiserID,
                                      info->keywordID,
                                      info->titleID,
                                      info->descriptionID);
        }
    }
}
// output the sorted (UserID), line by line, whose click-through-rate 
// (total click / total impression) on `AdID` is greater than or equal to `clickThroughRateLowerBound`.
void profit( uint32_t AdID, double clickThroughRateLowerBound ){

}


int main(int argc, char *argv[])
{

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
