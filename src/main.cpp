#include <cstdio>
#include <iostream>
#include <print>

#include <vector>

// TODO: Figure out how to get this.
// #include <inplace_vector>

#include "OrderBook.h"
#include "Types.h"

using namespace std;

void OrderBookTest() {
  InstrumentId app = 1;
  OrderBook orderBook(app);

  OrderId orderId = 0;

  AskOrderParams order1{.id = orderId++, .price = 100, .qty = 532};
  AskOrderParams order2{.id = orderId++, .price = 100, .qty = 431};
  AskOrderParams order3{.id = orderId++, .price = 400, .qty = 130};
  AskOrderParams order4{.id = orderId++, .price = 500, .qty = 432};
  AskOrderParams order5{.id = orderId++, .price = 400, .qty = 230};
  AskOrderParams order6{.id = orderId++, .price = 100, .qty = 312};

  BidOrderParams bidOrder1{.id = orderId++, .price = 100, .qty = 100};
  BidOrderParams bidOrder2{.id = orderId++, .price = 200, .qty = 320};
  BidOrderParams bidOrder3{.id = orderId++, .price = 200, .qty = 102};
  BidOrderParams bidOrder4{.id = orderId++, .price = 400, .qty = 132};
  BidOrderParams bidOrder5{.id = orderId++, .price = 400, .qty = 134};
  BidOrderParams bidOrder6{.id = orderId++, .price = 100, .qty = 243};

  orderBook.ProcessOperation<Operation::Ask>(order1);
  orderBook.ProcessOperation<Operation::Ask>(order2);
  orderBook.ProcessOperation<Operation::Ask>(order3);
  orderBook.ProcessOperation<Operation::Ask>(order4);
  orderBook.ProcessOperation<Operation::Ask>(order5);
  orderBook.ProcessOperation<Operation::Ask>(order6);

  orderBook.ProcessOperation<Operation::Bid>(bidOrder1);
  orderBook.ProcessOperation<Operation::Bid>(bidOrder2);
  orderBook.ProcessOperation<Operation::Bid>(bidOrder3);
  orderBook.ProcessOperation<Operation::Bid>(bidOrder4);
  orderBook.ProcessOperation<Operation::Bid>(bidOrder5);
  orderBook.ProcessOperation<Operation::Bid>(bidOrder6);

  orderBook.Print();

  std::print("----------------\n");
  auto bestAsk = orderBook.BestAsk();
  auto bestBid = orderBook.BestBid();

  std::print("Best Ask: {} \n", *bestAsk);
  std::print("Best Bid: {} \n", *bestBid);

  auto spread = orderBook.Spread();
  if (spread.has_value()) {

    std::print("Spread: {} \n", spread.value());
  }
}

void testFunc() {
  vector<int> myVec;
  myVec.push_back(20);
}

int main() {
  OrderBookTest();
  cout << "hello world" << '\n';
  return 0;
}
