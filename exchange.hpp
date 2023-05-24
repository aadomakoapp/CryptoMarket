#pragma once
#include <iostream>
#include <string>
#include "exchange.hpp"
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "useraccount.hpp"
#include "utility.hpp"
using std::string;
using std::vector;
using std::map;
using std::set;



struct Exchange {
// data member
  map< string, map<string, int> > account;
  
  vector<Order> open_orders;
  vector<Order> filled_orders;
  vector<Trade> all_trades;


// struct member functions
  void MakeDeposit(const std::string &username, const std::string &asset,
                   int amount);
  void PrintUserPortfolios(std::ostream &os);
  bool MakeWithdrawal(const std::string &username, const std::string &asset,
                      int amount);
  bool AddOrder(const Order &order);
  void PrintUsersOrders(std::ostream &os);
  void PrintTradeHistory(std::ostream &os);
  void PrintBidAskSpread(std::ostream &os);


  // Helper functions
  void BuyTransaction(Order buy_order, vector<Order> market_orders);
  void RemoveOrderFromOpenOrders(Order order, vector<Order> &open_orders);
  void SellTransaction(Order sell_order, vector<Order> open_market);
  set<string> GetAssetsPresent(const vector<Order> &open_orders);

};
