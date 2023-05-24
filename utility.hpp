#pragma once
#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "useraccount.hpp"

using std::string;
using std::ostream;
using std::ostringstream;
using std::cout;
using std::endl;

struct Order {
  string username;
  string side;  // Can be "Buy" or "Sell"
  string asset;
  int amount;
  int price;

Order(string name, string action, string currency, int amt, int prc): username(name), side(action), asset(currency), amount(amt),price(prc) {};

// copy constructer 
Order(const Order &order);

// = operator 
Order& operator=(const Order &order);

};

ostream& operator<<(ostream &out, const Order &order);
bool operator==(const Order &order1, const Order &order2);

// TRADE STRUCT
struct Trade {
  std::string buyer_username;
  std::string seller_username;
  std::string asset;
  int amount;
  int price;

  Trade(string buyer, string seller, string currency, int amt, int prc): buyer_username(buyer), seller_username(seller), asset(currency), amount(amt),price(prc) {};
};

ostream& operator<<(ostream &out, const Trade &trade);




