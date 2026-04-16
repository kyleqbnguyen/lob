#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, IocOrder_MatchesAvailableQuantityAndCancelsRemainder) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10000,
                                          .quantity = 15});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, IocOrder_CancelsWithoutTrade_WhenNonCrossing) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10000,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, FokOrder_ExecutesCompletely_WhenFullyFillable) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});
  orderBook.addOrder({.id = 101,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10100,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].quantity + trades[1].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, FokOrder_CancelsCompletely_WhenInsufficientLiquidity) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}
