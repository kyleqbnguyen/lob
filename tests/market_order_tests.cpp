#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest,
       addOrder_shouldFillAtPassivePrice_whenMarketBidCrossesBook) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldSweepAllAskLevels_whenMarketBidEntersBook) {
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
                      .quantity = 4});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 0,
                                          .quantity = 9});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].price, 10100);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldFillAtPassivePrice_whenMarketAskCrossesBook) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldSweepAllBidLevels_whenMarketAskEntersBook) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 5});
  orderBook.addOrder({.id = 101,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 4});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 0,
                                          .quantity = 9});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].price, 10000);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketBidExhaustsLiquidity) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketBidEntersEmptyBook) {
  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 0,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketAskExhaustsLiquidity) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldNotRestUnfilledQuantity_whenGtcMarketAskEntersEmptyBook) {
  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 0,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteMarketFokBid_whenSufficientLiquidityExists) {
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
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldCancelMarketFokBidWithoutTrading_whenLiquidityIsInsufficient) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 0,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteMarketFokAsk_whenSufficientLiquidityExists) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 5});
  orderBook.addOrder({.id = 101,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 0,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldCancelMarketFokAskWithoutTrading_whenLiquidityIsInsufficient) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Market,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 0,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}
