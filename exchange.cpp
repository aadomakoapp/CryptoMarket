#include "exchange.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
using std::string;
using std::vector;
using std::map;
using std::endl;
using std::set;

// Takes a username, asset and amount. It deposists that into a user's account.
void Exchange::MakeDeposit(const string& username, const string& asset, int amount)
{
    // if the user doesn't exist
    if (account.count(username) == 0) {
        map<string, int> value;
        value[asset] = amount;
        account[username] = value;
    }

    // the user already exists but the asset does not exist
    else if (account[username].count(asset) == 0) {
        account[username][asset] = amount;
    }

    // the asset exists already
    else {
        account[username][asset] += amount;
    }
}

// Takes an output stream and populates the stream with user portfolios
void Exchange::PrintUserPortfolios(std::ostream& os)
{
    os << "User Portfolios (in alphabetical order):" << endl;
    for (auto x : account) {
        os << x.first << "'s Portfolio: ";
        for (auto port : x.second) {
            if (port.second > 0)
                os << port.second << " " << port.first << ", ";
        }
        os << endl;
    }
}

// Takes an output stream and populates the stream with trades
void Exchange::PrintTradeHistory(std::ostream& os)
{
    os << "Trade History (in chronological order):" << endl;
    for (Trade trade : all_trades) {
        os << trade << endl;
    }
}

// Takes an output stream and populates the stream with user's orders
void Exchange::PrintUsersOrders(std::ostream& os)
{
    os << "Users Orders (in alphabetical order):" << endl;
    for (auto user : account) {
        string name = user.first;
        os << name << "'s Open Orders (in chronological order):" << endl;
        for (auto order : open_orders) {
            if (order.username == name)
                os << order << endl;
        }
        os << name << "'s Filled Orders (in chronological order):" << endl;
        for (auto order : filled_orders) {
            if (order.username == name)
                os << order << endl;
        }
    }
}

// Takes a username, asset and amount. It withdraws that into a user's account.
bool Exchange::MakeWithdrawal(const std::string& username, const std::string& asset, int amount)
{
    // if the user doesn't exist
    if (account.count(username) == 0)
        return false;

    // the user already exists but the asset does not exist
    else if (account[username].count(asset) == 0) {
        return false;
    }

    // the asset exists
    else {
        if (account[username][asset] >= amount) {
            account[username][asset] -= amount;
            return true;
        }
        else {
            return false;
        }
    }
}

// Takes an order and processes the order.
bool Exchange::AddOrder(const Order& order)
{
    // Sell order
    if (order.side == "Sell") {
        // check if the asset is even present
        if (account[order.username].count(order.asset) > 0) {
            if (account[order.username][order.asset] >= order.amount) {
                account[order.username][order.asset] -= order.amount;
                open_orders.push_back(order);
                SellTransaction(order, open_orders);
                return true;
            }
            else {
                return false;
            }
        }
        else
            return false;
    }
    // order is Buy
    int needed_cash = order.amount * order.price;
    if (account[order.username].count("USD") > 0) {
        int current_user_cash = account[order.username]["USD"];
        if (current_user_cash >= needed_cash) {
            account[order.username]["USD"] -= needed_cash;
            open_orders.push_back(order);
            BuyTransaction(order, open_orders);
            return true;
        }
        else
            return false;
    }
    else
        return false;
};

void Exchange::SellTransaction(Order sell_order, vector<Order> open_market)
{
    // filter the open_market to get only buy orders for the seller's needed asset
    // filter and get rid of all buy orders with prices lower than the seller's request
    vector<Order> open_buy_market;
    copy_if(open_market.begin(), open_market.end(), back_inserter(open_buy_market), [sell_order](Order p) {
        if (p.side == "Buy" && p.asset == sell_order.asset && p.price >= sell_order.price)
            return true;
        return false;
    });

    // sort the filtered buy_market from max price to low price
    stable_sort(open_buy_market.begin(), open_buy_market.end(), [](Order p1, Order p2) {
        if (p1.price > p2.price)
            return true;
        return false;
    });

    // do actual transaction
    for (Order buyer_order : open_buy_market) {
        // Exact Price Complete Trades
        if (sell_order.price == buyer_order.price && sell_order.amount == buyer_order.amount) {
            MakeDeposit(buyer_order.username, buyer_order.asset, buyer_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.price * sell_order.amount);
            Trade t1{ buyer_order.username, sell_order.username, buyer_order.asset, buyer_order.amount, buyer_order.price };
            all_trades.push_back(t1);
            filled_orders.push_back(buyer_order);
            filled_orders.push_back(sell_order);
            RemoveOrderFromOpenOrders(buyer_order, open_orders);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            return;
        }
        // Exact Price Incomplete Trades
        else if (sell_order.price == buyer_order.price && sell_order.amount > buyer_order.amount) {
            MakeDeposit(buyer_order.username, buyer_order.asset, buyer_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.price * buyer_order.amount);
            Trade t1{ buyer_order.username, sell_order.username, buyer_order.asset, buyer_order.amount, sell_order.price };
            all_trades.push_back(t1);
            int remainder = sell_order.amount - buyer_order.amount;
            Order rem_sell_order{ sell_order.username, "Sell", sell_order.asset, remainder, sell_order.price };
            Order finished_sell_order{ sell_order.username, "Sell", sell_order.asset, buyer_order.amount, sell_order.price };
            filled_orders.push_back(buyer_order);
            filled_orders.push_back(finished_sell_order);
            open_orders.push_back(rem_sell_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buyer_order, open_orders);
        }
        // AddOrder (Different Taker and Maker Prices)
        else if (sell_order.price < buyer_order.price && sell_order.amount < buyer_order.amount) {
            MakeDeposit(buyer_order.username, buyer_order.asset, sell_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.amount * sell_order.price);
            Trade t1{ buyer_order.username, sell_order.username, buyer_order.asset, sell_order.amount, sell_order.price };
            all_trades.push_back(t1);
            int remainder = buyer_order.amount - sell_order.amount;
            Order rem_buy_order{ buyer_order.username, "Buy", buyer_order.asset, remainder, buyer_order.price };
            Order finished_sell_order{ sell_order.username, "Sell", sell_order.asset, sell_order.amount, sell_order.price };
            Order finished_buy_order{ buyer_order.username, "Buy", buyer_order.asset, sell_order.amount, sell_order.price };
            filled_orders.push_back(finished_sell_order);
            filled_orders.push_back(finished_buy_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buyer_order, open_orders);
            open_orders.push_back(rem_buy_order);
        }

        else if (sell_order.price < buyer_order.price && sell_order.amount > buyer_order.amount) {
            MakeDeposit(buyer_order.username, buyer_order.asset, buyer_order.amount);
            MakeDeposit(sell_order.username, "USD", buyer_order.amount * sell_order.price);
            Trade t1{ buyer_order.username, sell_order.username, buyer_order.asset, buyer_order.amount, sell_order.price };
            all_trades.push_back(t1);
            int remainder = sell_order.amount - buyer_order.amount;
            Order rem_sell_order{ sell_order.username, "Sell", sell_order.asset, remainder, sell_order.price };
            Order finished_sell_order{ sell_order.username, "Sell", sell_order.asset, buyer_order.amount, sell_order.price };
            Order finished_buy_order{ buyer_order.username, "Buy", buyer_order.asset, buyer_order.amount, sell_order.price };
            filled_orders.push_back(finished_buy_order);
            filled_orders.push_back(finished_sell_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buyer_order, open_orders);
            open_orders.push_back(rem_sell_order);

            // rem_sell_order needs to be processed
            sell_order = rem_sell_order;
        }
    }
}

void Exchange::BuyTransaction(Order buy_order, vector<Order> open_market)
{
    // filter the open_market to get only sell orders for the buyer's needed asset
    // filter and get rid of all sell orders with prices higher than the buyer's request
    vector<Order> open_sell_market;
    copy_if(open_market.begin(), open_market.end(), back_inserter(open_sell_market), [buy_order](Order p) {
        if (p.side == "Sell" && p.asset == buy_order.asset && p.price <= buy_order.price)
            return true;
        return false;
    });

    // sort the filtered sell_market
    stable_sort(open_sell_market.begin(), open_sell_market.end(), [](Order p1, Order p2) {
        if (p1.price < p2.price)
            return true;
        return false;
    });

    // do actual transaction

    for (Order sell_order : open_sell_market) {
        // Exact Price Complete Trades
        if (sell_order.price == buy_order.price && sell_order.amount == buy_order.amount) {
            MakeDeposit(buy_order.username, buy_order.asset, buy_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.price * sell_order.amount);
            Trade t1{ buy_order.username, sell_order.username, buy_order.asset, buy_order.amount, buy_order.price };
            all_trades.push_back(t1);
            filled_orders.push_back(buy_order);
            filled_orders.push_back(sell_order);
            RemoveOrderFromOpenOrders(buy_order, open_orders);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            return;
        }
        // Exact Price Incomplete Trades
        else if (sell_order.price == buy_order.price && sell_order.amount < buy_order.amount) {
            MakeDeposit(buy_order.username, buy_order.asset, sell_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.amount * sell_order.price);
            Trade t1{ buy_order.username, sell_order.username, buy_order.asset, sell_order.amount, sell_order.price };
            all_trades.push_back(t1);
            // there is some more assets to be bought
            int remainder = buy_order.amount - sell_order.amount;
            Order rem_buy_order{ buy_order.username, "Buy", buy_order.asset, remainder, buy_order.price };
            Order finished_buy_order{ buy_order.username, "Buy", buy_order.asset, sell_order.amount, buy_order.price };
            filled_orders.push_back(sell_order);
            filled_orders.push_back(finished_buy_order);
            open_orders.push_back(rem_buy_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buy_order, open_orders);
            buy_order = rem_buy_order;
        }

        // AddOrder (Different Taker and Maker Prices)
        else if (buy_order.price > sell_order.price && buy_order.amount > sell_order.amount) {
            MakeDeposit(buy_order.username, buy_order.asset, sell_order.amount);
            MakeDeposit(sell_order.username, "USD", sell_order.amount * buy_order.price);
            Trade t1{ buy_order.username, sell_order.username, buy_order.asset, sell_order.amount, buy_order.price };
            all_trades.push_back(t1);
            int remainder = buy_order.amount - sell_order.amount;
            Order rem_buy_order{ buy_order.username, "Buy", buy_order.asset, remainder, buy_order.price };
            Order finished_buy_order{ buy_order.username, "Buy", buy_order.asset, sell_order.amount, buy_order.price };
            Order actual_sell{ sell_order.username, "Sell", sell_order.asset, sell_order.amount, buy_order.price };
            filled_orders.push_back(actual_sell);
            filled_orders.push_back(finished_buy_order);
            open_orders.push_back(rem_buy_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buy_order, open_orders);
            // corrected line added.
            buy_order = rem_buy_order;
        }
        else if (buy_order.price > sell_order.price && buy_order.amount < sell_order.amount) {
            MakeDeposit(buy_order.username, buy_order.asset, buy_order.amount);
            MakeDeposit(sell_order.username, "USD", buy_order.amount * buy_order.price);
            Trade t1{ buy_order.username, sell_order.username, buy_order.asset, buy_order.amount, buy_order.price };
            all_trades.push_back(t1);
            int remainder = sell_order.amount - buy_order.amount;
            Order rem_sell_order{ sell_order.username, "Sell", sell_order.asset, remainder, sell_order.price };
            Order finished_buy_order{ buy_order.username, "Buy", buy_order.asset, buy_order.amount, buy_order.price };
            Order actual_sell{ sell_order.username, "Sell", sell_order.asset, buy_order.amount, buy_order.price };
            filled_orders.push_back(actual_sell);
            filled_orders.push_back(finished_buy_order);

            // go through the open_orders and replace the sell_order, if the sell order is the order with the rem_sell_order

            std::replace_if(open_orders.begin(), open_orders.end(), [sell_order](Order order) {
                if (order == sell_order)
                    return true;
                return false;

            }, rem_sell_order);
            // open_orders.push_back(rem_sell_order);
            RemoveOrderFromOpenOrders(sell_order, open_orders);
            RemoveOrderFromOpenOrders(buy_order, open_orders);

            return;
        }
    }
}

void Exchange::RemoveOrderFromOpenOrders(Order order, vector<Order>& open_orders)
{
    open_orders.erase(std::remove_if(open_orders.begin(), open_orders.end(), [order](Order ord) {
                          if (order == ord)
                              return true;
                          return false;
                      }), open_orders.end());
}

set<string> Exchange::GetAssetsPresent(const vector<Order> &open_orders)
{
    set<string> my_temp_set;
    for (Order order : open_orders) {
        my_temp_set.insert(order.asset);
    }
    return my_temp_set;
}

void Exchange::PrintBidAskSpread(std::ostream& os)
{
    set<string> asset_names = GetAssetsPresent(open_orders);
    os << "Asset Bid Ask Spread (in alphabetical order):" << endl;
    for (string asset : asset_names) {
        vector<int> buy_order_prices;
        vector<int> sell_order_prices;
        for (Order order : open_orders) {
            if (order.asset == asset && order.side == "Buy")
                buy_order_prices.push_back(order.price);
            else if (order.asset == asset && order.side == "Sell")
                sell_order_prices.push_back(order.price);
        }
        stable_sort(buy_order_prices.begin(), buy_order_prices.end());
        stable_sort(sell_order_prices.begin(), sell_order_prices.end());

        if (buy_order_prices.size() == 0 && sell_order_prices.size() == 0)
            os << asset << ": Highest Open Buy = "
               << "NA"
               << " USD and Lowest Open Sell = "
               << "NA"
               << " USD" << endl;

        else if (buy_order_prices.size() > 0 && sell_order_prices.size() == 0) {
            os << asset << ": Highest Open Buy = " << buy_order_prices.back() << " USD and Lowest Open Sell = NA USD" << endl;
        }
        else if (buy_order_prices.size() == 0 && sell_order_prices.size() > 0) {
            os << asset << ": Highest Open Buy = "
               << "NA"
               << " USD and Lowest Open Sell = " << sell_order_prices[0] << " USD" << endl;
        }
        else {
            os << asset << ": Highest Open Buy = " << buy_order_prices.back() << " USD and Lowest Open Sell = " << sell_order_prices[0] << " USD" << endl;
        }
    }
}
