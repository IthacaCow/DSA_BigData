

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
        std::uint8_t position;
        std::uint8_t dept;
        Key(uint32_t _UserID, uint32_t _AdID, uint32_t _QueryID, uint32_t _Position, uint32_t _Dept):
            UserID(_UserID),AdID(_AdID),Query(_QueryID),Position(_Position),Dept(_Dept){}
        bool operator==(const Key& rhs) const {
            return rhs.userID == userID && rhs.adID == adID && rhs.queryID == queryID &&
                   rhs.position == position && rhs.dept == dept;
        }
        bool operator!=(const Key& rhs) const {
            return !operator==(rhs);
        }

    };

    struct KeyHash1 {
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

    std::hash<std::uint32_t> hash_fn;
    struct KeyHash2 {
        std::size_t operator()(const Key& key) const {
            std::size_t hash = 1610612741;
            hash = hash_fn( hash ^ key.UserID)
            hash = hash_fn( hash ^ key.AdID)
            hash = hash_fn( hash ^ key.QueryID)
            hash = hash_fn( hash ^ key.Position)
            hash = hash_fn( hash ^ key.Dept)
            return hash;
        }
    };
    
    struct Value {
        std::uint16_t click;
        std::uint16_t impression;

        bool operator==(const Value& rhs) const {
            return rhs.click == click && rhs.impression == impression ;
        }
        bool operator!=(const Value& rhs) const {
            return !operator==(rhs);
        }
        
    };

    using Entry = std::pair<Key,Value>;
    using Map   = fcmm::Fcmm<Key, unique_ptr<Value>, KeyHash1, KeyHash2> ;
}

namespace Ad {

    struct ClickThrough {
        uint32_t clickCount;
        uint32_t impressionCount;
        double rate;  // totalClicks / totalImpressions
    };
    struct Ad {
        uint32_t advertiserID; 
        vector< unique_ptr<AdInfo> > information;
        std::map< UserID,unique_ptr<ClickThrough> > clickThroughTable; // Use default hash function
    };
    struct AdInfo {
        uint64_t displayURL; 
        int keywordID; 
        int titleID; 
        int descriptionID;
    }
    using Map = fcmm::Fcmm<AdID, unique_ptr<Ad> >; // Use default hash function
}

namespace User {

    struct Query {
        int id;
        Ad* ad;
    };
    using Ads     = std::set< uint32_t >; // A set of AdIDs
    using Queries = std::vector< unique_ptr<User::Query> >;
    struct User {
        Ads impressions;  // All ads on which the user has at least one impression
        Queries clicks;   // All queris on which the user has at least one click
    };
    using Entry = std::pair< UserID,unique_ptr<User> >;
    using Map   = fcmm::Fcmm< UserID, unique_ptr<User> >; // Using default key hash

}

