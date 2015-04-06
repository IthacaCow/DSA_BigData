#include <iostream>
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

Data::Map dataMap(MAX_NUM_ENTRIES); // Key --> Value 
User::Map userMap(MAX_NUM_USERS);   // UserID --> User
Ad::Map adMap(MAX_NUM_ADS);         // AdID --> Ad

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
       std::unique_ptr<Data::Key>   key(new Data::Key);
       std::unique_ptr<Data::Value> key(new Data::Value);
       std::unique_ptr<Ad::AdInfo>  key(new Ad::AdInfo);
       std::unique_ptr<Ad::Ad>      key(new Ad::Ad);
//Click Impress URL_ID Ad_ID AdverID Dept Pos QueryID KeyWordID TitleID DescriptID UserID
       value->click          = strToInt(&p);
       value->impression     = strToInt(&p);
       adInfo->displayURL    = strToInt(&p);
       key->adID             = strToInt(&p);
       ad->advertiserID      = strToInt(&p);
       key->dept             = strToInt(&p);
       key->position         = strToInt(&p);
       key->queryID          = strToInt(&p);
       adInfo->keywordID     = strToInt(&p);
       adInfo->titleID       = strToInt(&p);
       adInfo->descriptionID = strToInt(&p);
       key->userID           = strToInt(&p);
       auto insertState = dataMap.insert( std::make_pair(std::move(Key),std::move(Value)) );
       // The key is already in map 
       if( !insertState.second ){
           insertState.first->second->update(

       }

       /* Add entry to adMap */
       // Ad* adBody = adMap.get( key.adID )
       // if( !adBody )
           // adMap.insert( std::make_pair( key->adID, ad ) ).first;
       Ad& adBody = adMap.insertAndGet( std::make_pair( key->adID, ad ) );

       adBody->information.push_back( adInfo ); 
       auto record = adBody->clickThroughTable.find( userID );
       if( record == adBody->clickThroughTable.end() )
           adBody->clickThroughTable.insert( 
               std::make_pair( userID, { value.click,value->impression,0,0 } )
           );
       else{
           if(value->click)
               adBody->clickThroughTable[ userID ].clickCount += value->click;
           adBody->clickThroughTable[ userID ].impressionCount += value->impression;
       }

       /* add entry to userMap */
       User* userBody = userMap.get( key->userID );
       if( !userBody )
           userMap.insert( std::make_pair( key->userID , unique_ptr<User::User>(new User::User()) );
       userBody->impressions.insert( key->adID );  // The user has at least one impression
    }
}
int main(int argc, char *argv[])
{
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
