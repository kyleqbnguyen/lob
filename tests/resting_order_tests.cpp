#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, AddOrder_ReturnsNoTrades_ForRestingInsert) {
  const auto trades = orderBook.addOrder({.id = id++,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
}

TEST_F(OrderBookTest, OppositeSideOrder_DoesNotAffectExistingSideState) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 15000, 7);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 15000);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 15000), 7);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 15000), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, NonCrossingBidOrder_RestsWithoutTrades) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, NonCrossingAskOrder_RestsWithoutTrades) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10100,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}
