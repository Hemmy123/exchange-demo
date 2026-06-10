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

  OrderParams order1{.id = orderId++, .price = 100, .qty = 532};
  OrderParams order2{.id = orderId++, .price = 100, .qty = 431};
  OrderParams order3{.id = orderId++, .price = 400, .qty = 130};
  OrderParams order4{.id = orderId++, .price = 500, .qty = 432};
  OrderParams order5{.id = orderId++, .price = 400, .qty = 230};
  OrderParams order6{.id = orderId++, .price = 100, .qty = 312};

  OrderParams bidOrder1{.id = orderId++, .price = 100, .qty = 100};
  OrderParams bidOrder2{.id = orderId++, .price = 200, .qty = 320};
  OrderParams bidOrder3{.id = orderId++, .price = 200, .qty = 102};
  OrderParams bidOrder4{.id = orderId++, .price = 400, .qty = 132};
  OrderParams bidOrder5{.id = orderId++, .price = 400, .qty = 134};
  OrderParams bidOrder6{.id = orderId++, .price = 100, .qty = 243};

  orderBook.PlaceOrder(Side::Ask, order1);
  orderBook.PlaceOrder(Side::Ask, order2);
  orderBook.PlaceOrder(Side::Ask, order3);
  orderBook.PlaceOrder(Side::Ask, order4);
  orderBook.PlaceOrder(Side::Ask, order5);
  orderBook.PlaceOrder(Side::Ask, order6);

  orderBook.PlaceOrder(Side::Bid, bidOrder1);
  orderBook.PlaceOrder(Side::Bid, bidOrder2);
  orderBook.PlaceOrder(Side::Bid, bidOrder3);
  orderBook.PlaceOrder(Side::Bid, bidOrder4);
  orderBook.PlaceOrder(Side::Bid, bidOrder5);
  orderBook.PlaceOrder(Side::Bid, bidOrder6);

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
  std::print("----------------\n");

  orderBook.Modify(5, 400, 1232131);
  orderBook.Print();
}

int main() {
  OrderBookTest();
  return 0;
}
