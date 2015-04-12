#include "bigData.hpp"
#include <cstdio>
#include <iostream>
using namespace std;
void printClick( User::Clicks& v ){
    cout<<"Click table: \n";
    for( auto& p: v){
        cout<<"(AdId: "<<p.first<<" ,QueryID: "<<p.second<<" ) ";
    }
    cout<<endl<<endl;
}

void printImpression( User::Ads& s ){
    cout<<"Impression set:\n";
    for( auto info = s.begin(); info != s.end() ; ++info ){
        cout<<" Id: "<< info->first <<endl;
        auto id = info->first;
        while( info->first == id ){
            printf("\t%llu %d %d %d %d\n",info->second->displayURL,
                                          info->second->advertiserID,
                                          info->second->keywordID,
                                          info->second->titleID,
                                          info->second->descriptionID);
            if( ++info == s.end() ){
                cout<<endl<<endl;
                return;
            }
        }
    }
}

void printDataMap( Data::Map& dataMap ){
    for( auto it = dataMap.begin(); it != dataMap.end(); it++ ){
        auto k = it->first;
        cout<<"Key: "<<k.userID<<" "<<k.adID<<" "<<k.queryID<<" "<<(int)k.position<<" "<<(int)k.dept<<endl;
        cout<<"click: "<<it->second.click<<" ,impression: "<<it->second.impression<<endl;
    }
    
}
