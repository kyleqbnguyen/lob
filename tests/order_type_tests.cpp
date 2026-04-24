#include "fixture.h"

TEST_F(OrderBookTest,
       addOrder_shouldStopAtLimitPrice_whenLimitBidCannotSweepHigherAskLevels) {
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
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 7);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest,
       addOrder_shouldStopAtLimitPrice_whenLimitAskCannotSweepLowerBidLevels) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});
  orderBook.addOrder({.id = 101,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 9900,
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 7);
  EXPECT_EQ(orderBook.orderCount(), 2);
}
