#ifndef BIGDATA_H
#define BIGDATA_H

#include <set>
#include <boost/functional/hash.hpp>
#include <vector>
#include <map>
#include <unordered_map>
#include <iterator>
using ullint_t = unsigned long long int;
using ushint_t = unsigned short;
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

        // Key(){}
        // Key(uint32_t u, uint32_t a, uint32_t q, uint8_t p, uint8_t d):
            // userID(u),adID(a),queryID(q),position(p),dept(d){}

    };

    struct KeyHash {
        std::size_t operator()(const Key& key) const {
            using boost::hash_value;
            using boost::hash_combine;

            std::size_t hash = 0;
            hash_combine(hash,hash_value(key.userID));
            hash_combine(hash,hash_value(key.adID));
            hash_combine(hash,hash_value(key.queryID));
            hash_combine(hash,hash_value(key.position));
            hash_combine(hash,hash_value(key.dept));
            return hash;
        }
    };
    // struct KeyHash {
        // std::size_t operator()(const Key& key) const {
            // const std::size_t prime = 92821;
            // std::size_t hash = 486187739;
            // hash = (hash * prime) ^ key.userID;
            // hash = (hash * prime) ^ key.adID;
            // hash = (hash * prime) ^ key.queryID;
            // hash = (hash * prime) ^ key.position;
            // hash = (hash * prime) ^ key.dept;
            // return hash;
        // }
    // };

    
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
        uint32_t clickCount = 0;
        uint32_t impressionCount = 0;

        void add( Data::Value& value ){
            if( value.click )clickCount += value.click;
            impressionCount += value.impression;
        }
    };
    struct AdInfo {
        ushint_t advertiserID; 
        ullint_t displayURL; 
        uint32_t keywordID; 
        uint32_t titleID; 
        uint32_t descriptionID;

    };
    struct InfoComparator {
        bool operator() ( const AdInfo* lhs, const AdInfo* rhs ) {
            if( lhs->keywordID < rhs->keywordID )  
                return true;
            if( lhs->keywordID > rhs->keywordID )  
                return false;
            if( lhs->advertiserID < rhs->advertiserID )  
                return true;
            if( lhs->advertiserID > rhs->advertiserID )  
                return false;
            if( lhs->titleID < rhs->titleID )  
                return true;
            if( lhs->titleID > rhs->titleID )  
                return false;
            if( lhs->descriptionID < rhs->descriptionID )  
                return true;
            if( lhs->descriptionID > rhs->descriptionID )  
                return false;
            if( lhs->displayURL < rhs->displayURL )  
                return true;
            if( lhs->displayURL > rhs->displayURL )  
                return false;
            return false;
        }
    };
    using ClickThroughTable = std::map< uint32_t,ClickThrough >;
    struct Ad {
        // UserID --> clickThrough
        ClickThroughTable clickThroughTable;
    };
    // AdID --> Ad
    using Map = std::unordered_map< std::uint32_t, Ad* >; 
    using InfoTable = std::set< AdInfo*, InfoComparator >;

}

namespace User {

    struct ClickComparator; 
    // using InfoPair = std::pair<uint32_t,Ad::InfoTable::iterator>;
    using InfoPair = std::pair<uint32_t,Ad::AdInfo*>;
    using Ads    = std::set< InfoPair >; // A set of AdIDs + Info
    using Clicks = std::vector< std::pair<uint32_t,uint32_t> >; 
    // TODO: Check duplicate

    struct ClickComparator {
        bool operator() ( const Data::Key* lhs, const Data::Key* rhs ) {
            return (lhs->adID < rhs->adID) || ( lhs->adID == rhs->adID && lhs->queryID < rhs->queryID);
        }
    };
    // struct InfoPairComparator{
        // bool operator() ( const InfoPair& lhs, const InfoPair& rhs ) {
            // return (lhs.first < rhs.first) || ( lhs.first == rhs.first && *(lhs.second) < *(rhs.second));
        // }
    // };
    struct User {
        Ads impressions;  // All ads on which the user has at least one impression
        Clicks clicks;   // All queries on which the user has at least one click

    };
    

}



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

#endif
