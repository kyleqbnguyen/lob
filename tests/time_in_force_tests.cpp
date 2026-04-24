#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocRemainder_whenOrderPartiallyMatches) {
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
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocWithoutTrade_whenOrderDoesNotCross) {
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
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteFokCompletely_whenOrderIsFullyFillable) {
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
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteFokAgainstSingleLevel_whenExactLiquidityExists) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelFokCompletely_whenLiquidityIsInsufficient) {
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
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest, addOrder_shouldCancelFokWithoutTrading_whenBookIsEmpty) {
  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocAskRemainder_whenAskPartiallyMatches) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10000,
                                          .quantity = 15});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelIocAskWithoutTrade_whenAskOrderDoesNotCross) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 9900,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10000,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 10);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldSweepAndCancelRemainder_whenIocAskCrossesMultipleLevels) {
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
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10000,
                                          .quantity = 12});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldExecuteFokAsk_whenSufficientBidLiquidityExists) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelFokAsk_whenBidLiquidityIsInsufficient) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldCancelFokAskWithoutTrading_whenBookIsEmpty) {
  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest, addOrder_shouldRestDtcBid_whenBidDoesNotCross) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::DTC,
                                          .price = 10000,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest, addOrder_shouldRestDtcAsk_whenAskDoesNotCross) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 9900,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::DTC,
                                          .price = 10000,
                                          .quantity = 5});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest, addOrder_shouldMatchDtcBid_whenBidCrossesBook) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::DTC,
                                          .price = 10000,
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

TEST_F(OrderBookTest, addOrder_shouldRestDtcRemainder_whenPartialMatchOccurs) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 5});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::DTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldCancelFokWithoutTrading_whenLiquidityExistsOnlyBeyondLimitPrice) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10100,
                      .quantity = 10});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10000,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 10);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldLeaveBookUnchanged_whenFokCannotFillAcrossMultipleLevels) {
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
                      .quantity = 3});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::FOK,
                                          .price = 10100,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.depth(Side::Ask), 2);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 3);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldSweepAvailableLiquidityAndCancelRemainder_whenIocCrossesMultipleLevels) {
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
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::IOC,
                                          .price = 10100,
                                          .quantity = 12});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].quantity, 4);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.orderCount(), 0);
}
