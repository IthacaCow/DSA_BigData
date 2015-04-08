

/*
 * Get(UserID,AdID,QueryID,Position,Depth)
 *
 * map[ Key ] --> Entry
 *
 * *************************************
 *
 * Clicked( UserID )
 *
 * map[ UserID ] --> User
 *
 * *************************************
 *
 * Impressed( UserID_1, UserID_2 )
 *
 * intersection( map[ UserID_1 ]-->impressions map[ UserID_2 ]-->impressions )
 *
 * *************************************
 *
 * Profit( AdID, lowerBound )
 *
 * filter( map[ AdID ]-->clickThroughRate >= lowerBound )
 *
 *
 * */

/*
 * Maximum Value:
 *
    Click = 39900
    Impression = 764404
    DisplayURL = 18,445,899,849,997,222,722
    AdID = 22238277
    AdvertiserID = 39191
    Depth = 3
    Position = 3
    QueryID = 26243605
    KeywordID = 1249783
    TitleID = 4051439
    DescriptionID = 3171828
    UserID = 23907634

 */

#include <set>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <iterator>
#include "fcmm.hpp"
using ullint_t = unsigned long long int;
using ushint_t = unsigned short;
using std::unique_ptr;
using UserID = std::uint32_t;

const int MAX_USER_ID = 23907635;
namespace Data {
    struct Key {
        std::uint32_t userID;
        std::uint32_t adID;
        std::uint32_t queryID;
        std::uint8_t  position;
        std::uint8_t  dept;
        bool operator==(const Key& rhs) const {
            return rhs.userID == userID && rhs.adID == adID && rhs.queryID == queryID &&
                   rhs.position == position && rhs.dept == dept;
        }
        bool operator!=(const Key& rhs) const {
            return !operator==(rhs);
        }

        Key(){}
        Key(uint32_t u, uint32_t a, uint32_t q, uint8_t p, uint8_t d):
            userID(u),adID(a),queryID(q),position(p),dept(d){}

    };

    struct KeyHash {
        std::size_t operator()(const Key& key) const {
            const std::size_t prime = 92821;
            std::size_t hash = 486187739;
            hash = (hash * prime) ^ key.userID;
            hash = (hash * prime) ^ key.adID;
            hash = (hash * prime) ^ key.queryID;
            hash = (hash * prime) ^ key.position;
            hash = (hash * prime) ^ key.dept;
            return hash;
        }
    };

    
    struct Value {
        std::uint16_t click;
        std::uint32_t impression;

        void update( const Value & rhs ){
            click += rhs.click;
            impression += rhs.impression;
        }
    };

    using Map   = std::unordered_map< Data::Key, Data::Value, KeyHash >; 
}

namespace Ad {

    struct ClickThrough {
        uint32_t clickCount;
        uint32_t impressionCount;
        double rate;  // totalClicks / totalImpressions

        ClickThrough( uint32_t _clickCount ,uint32_t _impressionCount ,double _rate ): 
            clickCount(_clickCount), impressionCount(_impressionCount), rate(_rate){} 
    };
    struct AdInfo {
        ullint_t displayURL; 
        uint32_t keywordID; 
        uint32_t titleID; 
        uint32_t descriptionID;
    };
    using Information = std::vector< AdInfo* >;
    using ClickThroughTable = std::map< uint32_t,ClickThrough* >;
    struct Ad {
        ushint_t advertiserID; 
        Information information;
        // UserID --> clickThrough
        ClickThroughTable clickThroughTable;
        void updateClickTable( uint32_t userID , const Data::Value& value ){
           auto record = clickThroughTable.find( userID );
           if( record == clickThroughTable.end() ){
               clickThroughTable.emplace( 
                   userID,  new ClickThrough( value.click,value.impression,0.0 )
               );
           }
           else{
               auto& entry = clickThroughTable[ userID ];
               entry->impressionCount += value.impression;
               if( value.click )
                   entry->clickCount += value.click;
           }

        }
        void calculateClickThroughEach(){
           for( auto& entry: clickThroughTable ) {
                if( entry.second->clickCount )
                    entry.second->rate = (double)entry.second->clickCount / entry.second->impressionCount;
           }
           // for( ClickThroughTable::iterator entry = table.begin(); entry != table.end(); entry++ ) {
                // if( entry->second->clickCount )
                    // entry->second->rate = (double)entry->second->clickCount / entry->second->impressionCount;
           // }
        }
        ~Ad(){
            for( auto info: information )
                delete info;
            for( auto&& it: clickThroughTable )
                delete it.second;
        }
    };
    // AdID --> Ad
    using Map = std::unordered_map< std::uint32_t, Ad* >; 

    void calculateClickThroughRate( Map::iterator it, Map::iterator until ){
        for( ; it != until ; ++it )
            it->second->calculateClickThroughEach();
    }
}

namespace User {

    using Ads    = std::set< std::uint32_t >; // A set of AdIDs
    using Clicks = std::vector< const Data::Key* >; // Ideally, there won't be duplicated elements
    struct User {
        Ads impressions;  // All ads on which the user has at least one impression
        Clicks clicks;   // All queries on which the user has at least one click
    };
    
    using Map   = std::unordered_map< uint32_t, User* >; // UserID --> User

    void buildClicks( Data::Map::iterator from, Data::Map::iterator until ){
    }

}

