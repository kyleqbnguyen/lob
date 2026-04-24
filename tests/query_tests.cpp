#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, queries_shouldReportNoLiquidity_whenBookIsEmpty) {
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       queries_shouldReportLiquidityOnlyOnBidSide_whenBookHasSingleBid) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       bestBid_shouldReturnHighestRestingBid_whenBookHasMultipleBids) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 10);
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest, bestBid_shouldRemainHighest_whenLowerBidAdded) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest,
       bestAsk_shouldReturnLowestRestingAsk_whenBookHasMultipleAsks) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 10);

  EXPECT_EQ(orderBook.bestAsk(), 5000);
}

TEST_F(OrderBookTest, bestAsk_shouldRemainLowest_whenHigherAskAdded) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestAsk(), 5000);
}

TEST_F(OrderBookTest, depth_shouldNotIncrease_whenBidAddedAtSamePriceLevel) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
}

TEST_F(OrderBookTest, depth_shouldNotIncrease_whenAskAddedAtSamePriceLevel) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
}

TEST_F(OrderBookTest, depth_shouldIncrease_whenBidAddedOnNewLevel) {
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.depth(Side::Bid), 2);
}

TEST_F(OrderBookTest, depth_shouldIncrease_whenAskAddedOnNewLevel) {
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.depth(Side::Ask), 2);
}

TEST_F(OrderBookTest,
       depth_shouldRemainIndependent_whenBothSidesHaveLiquidity) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10100, 5);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnSumOfBidQuantities_whenLevelHasMultipleOrders) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000,
               firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000,
               secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), sum);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnSumOfAskQuantities_whenLevelHasMultipleOrders) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000,
               firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000,
               secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), sum);
}

TEST_F(OrderBookTest,
       quantityAt_shouldRemainIsolatedPerBidLevel_whenBookHasMultipleLevels) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 5000), 5);
}

TEST_F(OrderBookTest,
       quantityAt_shouldRemainIsolatedPerAskLevel_whenBookHasMultipleLevels) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 10);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 5000), 5);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnZero_whenBidLevelIsEmptyInNonEmptyBook) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 5000), 0);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnZero_whenAskLevelIsEmptyInNonEmptyBook) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 5000), 0);
}

TEST_F(OrderBookTest,
       quantityAt_shouldReturnZero_whenLevelExistsOnOppositeSide) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
}

TEST_F(OrderBookTest,
       orderCount_shouldCountMultipleOrders_whenTheySharePriceLevel) {
  EXPECT_EQ(orderBook.orderCount(), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.orderCount(), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(OrderBookTest, orderCount_shouldCountOrders_whenTheyRestOnBothSides) {
  EXPECT_EQ(orderBook.orderCount(), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.orderCount(), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.orderCount(), 2);
}

TEST_F(
    OrderBookTest,
    queries_shouldUpdateAfterTradeRemovesBestLevels_whenBookTransitionsToEmpty) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 5);

  const auto trades = orderBook.addOrder({.id = id++,
                                          .side = Side::Ask,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 5});

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}
