#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, CrossingBidOrder_GeneratesTradeAndConsumesRestingAsk) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, CrossingAskOrder_GeneratesTradeAndConsumesRestingBid) {
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
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, PartialFill_RestsAggressorRemainderOnBidSide) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 15});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, PartialFill_RestsAggressorRemainderOnAskSide) {
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
                                          .price = 10000,
                                          .quantity = 15});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest,
       PartialFill_DecrementsRestingQuantityWithoutChangingPrice) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 4});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 4);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 6);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, FullFill_RemovesBestLevelAndPromotesNextAsk) {
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
                      .price = 10100,
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 7);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, FullFill_RemovesBestLevelAndPromotesNextBid) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});
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
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 9900), 7);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, MultiLevelSweep_MatchesBestPriceFirst) {
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
                                          .price = 10100,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10100);
  EXPECT_EQ(trades[1].quantity, 5);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10100), 2);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, SamePriceLevel_UsesFifoForRestingAsks) {
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
                      .price = 10000,
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 6});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 6);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, ExactTouchBidPrice_TradesAgainstBestAsk) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 5});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, ExactTouchAskPrice_TradesAgainstBestBid) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 5});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}
