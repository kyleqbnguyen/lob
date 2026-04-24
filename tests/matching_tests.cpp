#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, addOrder_shouldConsumeRestingAsk_whenBidCrossesBook) {
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
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest, addOrder_shouldConsumeRestingBid_whenAskCrossesBook) {
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
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       addOrder_shouldRestBidRemainder_whenBidPartiallyFillsAsk) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldRestAskRemainder_whenAskPartiallyFillsBid) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldDecrementRestingQuantity_whenPartialFillOccursAtBestPrice) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 4);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 6);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest, addOrder_shouldPromoteNextAsk_whenBestAskIsFullyFilled) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 7);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest, addOrder_shouldPromoteNextBid_whenBestBidIsFullyFilled) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 7);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldMatchBestPriceFirst_whenSweepingMultipleLevels) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10100);
  EXPECT_EQ(trades[1].quantity, 5);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 2);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldMatchHighestBidFirst_whenAskSweepsMultipleBidLevels) {
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
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 10});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10000);
  EXPECT_EQ(trades[1].quantity, 5);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 2);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldUseFifo_whenMatchingRestingAsksAtSamePrice) {
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 6);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldUseFifo_whenMatchingRestingBidsAtSamePrice) {
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
                      .price = 10000,
                      .quantity = 7});

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 6});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].quantity, 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 6);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldFullyConsumeFrontAsk_whenMatchEqualsFrontOrderQuantity) {
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
                                          .quantity = 5});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 7);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldRestAggressorRemainder_whenSweepExhaustsAvailableLiquidity) {
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
                                          .quantity = 15});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].passiveId, 100);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(trades[1].aggressorId, 200);
  EXPECT_EQ(trades[1].passiveId, 101);
  EXPECT_EQ(trades[1].price, 10100);
  EXPECT_EQ(trades[1].quantity, 7);

  EXPECT_EQ(orderBook.bestBid(), 10100);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10100), 3);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 0);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       addOrder_shouldTradeAtPassivePrice_whenAggressorImprovesOnRestingAsk) {
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
                                          .price = 10500,
                                          .quantity = 5});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(trades[0].price, 10000);
  EXPECT_EQ(trades[0].quantity, 5);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(
    OrderBookTest,
    addOrder_shouldLeaveSameSideLiquidityUntouched_whenMatchingOppositeSide) {
  orderBook.addOrder({.id = 10,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 9900,
                      .quantity = 4});
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
  EXPECT_EQ(trades[0].aggressorId, 200);
  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 9900), 4);
  EXPECT_EQ(orderBook.orderCount(), 1);
}
