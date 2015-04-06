

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

#include <set>
#include <memory>
#include <vector>
#include <iterator>
#include "fcmm.hpp"

using std::unique_ptr;
using UserID = std::uint32_t;

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

    };

    struct KeyHash {
        std::size_t operator()(const Key& key) const {
            const std::size_t prime = 92821;
            std::size_t hash = 486187739;
            hash = (hash * prime) ^ key.UserID;
            hash = (hash * prime) ^ key.AdID;
            hash = (hash * prime) ^ key.QueryID;
            hash = (hash * prime) ^ key.Position;
            hash = (hash * prime) ^ key.Dept;
            return hash;
        }
    };

    
    struct Value {
        std::uint16_t click;
        std::uint16_t impression;

        void update( const Value & User::Value rhs ){
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
    using Information = vector< unique_ptr<AdInfo> >;
    using ClickThroughTable = std::map< uint32_t,unique_ptr<ClickThrough> >;
    struct Ad {
        uint32_t advertiserID; 
        Information information;
        // UserID --> clickThrough
        ClickThroughTable clickThroughTable;

        void updateClickTable( uint32_t userID , const Value& value ){
           auto record = clickThroughTable.find( userID );
           if( record == clickThroughTable.end() ){
               clickThroughTable.emplace( 
                   userID,  new ClickThrough( value->click,value->impression,0.0 )
               );
           }
           else{
               ClickThrough &entry = clickThroughTable[ userID ];
               entry.impressionCount += value.impression;
               if( value.click )
                   entry.clickCount += value.click;
           }

        }
        void calculateClickThroughEach(){
           for( ClickThroughTable::iterator entry = table.begin(); entry != table.end(); entry++ ) {
                if( entry->second->clickCount )
                    entry->second->rate = (double)entry->second->clickCount / entry->second->impressionCount;
           }
        }
    };
    struct AdInfo {
        uint64_t displayURL; 
        int keywordID; 
        int titleID; 
        int descriptionID;
    }
    // AdID --> Ad
    using Map = std::unordered_map< uint32_t, unique_ptr<Ad::Ad> >; 

    void calculateClickThroughRate( Map::iterator it, Map::iterator until ){
        for( ; it != until ; ++it )
            it->calculateClickThroughEach();
    }
}

namespace User {

    using Ads    = std::set< uint32_t >; // A set of AdIDs
    using Clicks = std::vector< Key* >; // Ideally, there won't be duplicated elements
    struct User {
        Ads impressions;  // All ads on which the user has at least one impression
        Clicks clicks;   // All queries on which the user has at least one click
    };
    
    using Map   = std::unordered_map< uint32_t, unique_ptr<User::User> >; // UserID --> User

    void buildClicks( Data::Map::const_iterator from, Data::Map::const_iterator until ){
    }

}

