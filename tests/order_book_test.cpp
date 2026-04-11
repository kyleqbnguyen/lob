#include "lob/order_book.h"
#include "lob/types.h"

#include <gtest/gtest.h>
#include <optional>

class OrderBookTest : public ::testing::Test {
protected:
  OrderBook orderBook{};
  OrderId id{1};

  void addTestOrder(Side side, OrderType type, Price price, Quantity quantity) {
    orderBook.addOrder({.id = id++,
                        .side = side,
                        .type = type,
                        .price = price,
                        .quantity = quantity});
  }
};

// State Query Behavior
TEST_F(OrderBookTest, EmptyBook_ReportsNoLiquidity) {
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, SingleBid_OnlyReportsLiquidityOnBidSide) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.bestBid(), 100);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

TEST_F(OrderBookTest, SingleAsk_OnlyReportsLiquidityOnAskSide) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.bestAsk(), 100);
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);

  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 0);

  EXPECT_EQ(orderBook.getOrderBookSize(), 1);
}

// Method Specific Behavior
TEST_F(OrderBookTest, BestBid_ReturnsHighestRestingBid) {
  addTestOrder(Side::Bid, OrderType::GTC, 50, 10);
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.bestBid(), 100);
}

TEST_F(OrderBookTest, BestAsk_ReturnsLowestRestingAsk) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  addTestOrder(Side::Ask, OrderType::GTC, 50, 10);

  EXPECT_EQ(orderBook.bestAsk(), 50);
}
