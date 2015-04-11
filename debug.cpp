#include "bigData.hpp"
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
    cout<<"Impression set: (adId) \n";
    for( auto& p: s){
        cout<<p<<" ";
    }
    cout<<endl<<endl;
}
