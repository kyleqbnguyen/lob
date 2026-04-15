#include "order_book_test.h"

#include <optional>

TEST_F(OrderBookTest, EmptyBook_ReportsNoLiquidity) {
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, SingleBid_OnlyReportsLiquidityOnBidSide) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, SingleAsk_OnlyReportsLiquidityOnAskSide) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestAsk(), 10000);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, BestBid_ReturnsHighestRestingBid) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 10);
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.bestBid(), 10000);
}

TEST_F(OrderBookTest, BestAsk_ReturnsLowestRestingAsk) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 10);

  EXPECT_EQ(orderBook.bestAsk(), 5000);
}

TEST_F(OrderBookTest, BidDepth_Unaffected_WhenOrdersAddedAtSamePriceLevel) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
}

TEST_F(OrderBookTest, AskDepth_Unaffected_WhenOrdersAddedAtSamePriceLevel) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
}

TEST_F(OrderBookTest, BidDepth_Increases_WhenOrderAddedOnNewLevel) {
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 2);
}

TEST_F(OrderBookTest, AskDepth_Increases_WhenOrderAddedOnNewLevel) {
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 2);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsSumOfBidQuantities_AtSamePriceLevel) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000,
               firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000,
               secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), sum);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsSumOfAskQuantities_AtSamePriceLevel) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000,
               firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000,
               secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), sum);
}

TEST_F(OrderBookTest, QuantityAt_IgnoresOtherBidLevels) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 5000), 5);
}

TEST_F(OrderBookTest, QuantityAt_IgnoresOtherAskLevels) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 10000), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 5000), 5);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsZero_ForEmptyLevelInNonEmptyBidBook) {
  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 5000), 0);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsZero_ForEmptyLevelInNonEmptyAskBook) {
  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 5000), 0);
}

TEST_F(OrderBookTest, OrderBookSize_CountsBidsOn_SameLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsAsksOn_SameLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsBidsOn_NewPriceLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsAsksOn_NewPriceLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsOrdersOn_BothSides) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::Limit, TimeInForce::GTC, 10000, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::Limit, TimeInForce::GTC, 5000, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

