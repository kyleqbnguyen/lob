#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, modifyOrder_shouldReturnTrue_whenOrderExists) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  EXPECT_TRUE(orderBook.modifyOrder(100, 5));
}

TEST_F(OrderBookTest, modifyOrder_shouldReturnFalse_whenOrderDoesNotExist) {
  EXPECT_FALSE(orderBook.modifyOrder(999, 5));
}

TEST_F(OrderBookTest,
       modifyOrder_shouldUpdateLevelQuantity_whenBidQuantityReduced) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.modifyOrder(100, 4);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 4);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldUpdateLevelQuantity_whenAskQuantityReduced) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.modifyOrder(100, 4);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 4);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldUpdateOnlyTargetOrder_whenMultipleOrdersAtLevel) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});
  orderBook.addOrder({.id = 101,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 6});

  orderBook.modifyOrder(100, 3);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 9);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldPreserveTimePriority_whenQuantityReduced) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});
  orderBook.addOrder({.id = 101,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  orderBook.modifyOrder(100, 3);

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 4});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 3);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldNotAffectBestPrice_whenQuantityReduced) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.modifyOrder(100, 4);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest,
       modifyOrder_shouldNotAffectOppositeSide_whenBidIsModified) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});
  orderBook.addOrder({.id = 101,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 5});

  orderBook.modifyOrder(100, 3);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 5);
}
