#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "exchange.hpp"
#include "useraccount.hpp"
#include "utility.hpp"

using std::string;
using std::ostream;
using std::ostringstream;
using std::cout;
using std::endl;


Order::Order(const Order &order){
    username = order.username;
    side = order.side;
    asset = order.asset;
    amount = order.amount;
    price = order.price;
};

Order& Order::operator=(const Order &order){
    username = order.username;
    side = order.side;
    asset = order.asset;
    amount = order.amount;
    price = order.price;
    return *this;
};

// << operator overload
ostream& operator<<(ostream &out, const Order &order){
    out<<order.side<<" "<<order.amount<<" "<<order.asset<<" at "<<order.price<<" USD by "<<order.username;
    return out;
};

ostream& operator<<(ostream &out, const Trade &trade){
    out<<trade.buyer_username<<" Bought "<<trade.amount<<" of "<<trade.asset<<" From "<<trade.seller_username<<" for "
    <<trade.price<<" USD";
   return out;
};

// == operator overload
bool operator==(const Order &order1, const Order &order2){
    if(order1.amount == order2.amount && order1.asset == order2.asset && order1.side == order2.side && 
    order1.username == order2.username && order1.price == order2.price)
        return true;
    return false;
}







