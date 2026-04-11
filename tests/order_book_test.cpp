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

TEST_F(OrderBookTest, BidDepth_Unaffected_WhenOrdersAddedAtSamePriceLevel) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
}

TEST_F(OrderBookTest, AskDepth_Unaffected_WhenOrdersAddedAtSamePriceLevel) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 100, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
}

TEST_F(OrderBookTest, BidDepth_Increases_WhenOrderAddedOnNewLevel) {
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);

  addTestOrder(Side::Bid, OrderType::GTC, 50, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 2);
}

TEST_F(OrderBookTest, AskDepth_Increases_WhenOrderAddedOnNewLevel) {
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 50, 5);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 2);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsSumOfBidQuantities_AtSamePriceLevel) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Bid, OrderType::GTC, 100, firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Bid, OrderType::GTC, 100, secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), sum);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsSumOfAskQuantities_AtSamePriceLevel) {
  constexpr Quantity firstOrderQuantity{10};
  addTestOrder(Side::Ask, OrderType::GTC, 100, firstOrderQuantity);

  constexpr Quantity secondOrderQuantity{5};
  addTestOrder(Side::Ask, OrderType::GTC, 100, secondOrderQuantity);

  constexpr Quantity sum{firstOrderQuantity + secondOrderQuantity};
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), sum);
}

TEST_F(OrderBookTest, QuantityAt_IgnoresOtherBidLevels) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  addTestOrder(Side::Bid, OrderType::GTC, 50, 5);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 50), 5);
}

TEST_F(OrderBookTest, QuantityAt_IgnoresOtherAskLevels) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  addTestOrder(Side::Ask, OrderType::GTC, 50, 5);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 50), 5);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsZero_ForEmptyLevelInNonEmptyBidBook) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 50), 0);
}

TEST_F(OrderBookTest, QuantityAt_ReturnsZero_ForEmptyLevelInNonEmptyAskBook) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 50), 0);
}

TEST_F(OrderBookTest, OrderBookSize_CountsBidsOn_SameLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsAsksOn_SameLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 100, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsBidsOn_DiffLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Bid, OrderType::GTC, 50, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsAsksOn_DiffLevel) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 50, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

TEST_F(OrderBookTest, OrderBookSize_CountsOrdersOn_BothSides) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 50, 5);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}

// Non-Mutation Tests
TEST_F(OrderBookTest, BestBid_DoesNotMutate_EmptyBook) {
  EXPECT_EQ(orderBook.bestBid(), std::nullopt);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, BestAsk_DoesNotMutate_EmptyBook) {
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, BestBid_DoesNotMutate_NonEmptyBook) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  addTestOrder(Side::Bid, OrderType::GTC, 50, 5);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();
  const auto initialQtyAt100 = orderBook.getQuantityAt(Side::Bid, 100);
  const auto initialQtyAt50 = orderBook.getQuantityAt(Side::Bid, 50);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), initialQtyAt100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 50), initialQtyAt50);
}

TEST_F(OrderBookTest, BestAsk_DoesNotMutate_NonEmptyBook) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);
  addTestOrder(Side::Ask, OrderType::GTC, 150, 5);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();
  const auto initialQtyAt100 = orderBook.getQuantityAt(Side::Ask, 100);
  const auto initialQtyAt150 = orderBook.getQuantityAt(Side::Ask, 150);

  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), initialQtyAt100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 150), initialQtyAt150);
}

TEST_F(OrderBookTest, GetBookDepth_DoesNotMutate_EmptyBook) {
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, GetBookDepth_DoesNotMutate_NonEmptyBook) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  addTestOrder(Side::Bid, OrderType::GTC, 50, 5);
  addTestOrder(Side::Ask, OrderType::GTC, 150, 7);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();
  const auto initialBidQty100 = orderBook.getQuantityAt(Side::Bid, 100);
  const auto initialBidQty50 = orderBook.getQuantityAt(Side::Bid, 50);
  const auto initialAskQty150 = orderBook.getQuantityAt(Side::Ask, 150);

  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), initialBidQty100);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 50), initialBidQty50);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 150), initialAskQty150);
}

TEST_F(OrderBookTest, QuantityAt_DoesNotMutate_ExistingBidLevel) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 10);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
}

TEST_F(OrderBookTest, QuantityAt_DoesNotMutate_ExistingAskLevel) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 10);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
}

TEST_F(OrderBookTest, QuantityAt_DoesNot_InsertMissingBidLevel) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();

  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 50), 0);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
}

TEST_F(OrderBookTest, QuantityAt_DoesNot_InsertMissingAskLevel) {
  addTestOrder(Side::Ask, OrderType::GTC, 100, 10);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();

  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 50), 0);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
}

TEST_F(OrderBookTest, OrderBookSize_DoesNotMutateEmptyBook) {
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 0);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 0);
}

TEST_F(OrderBookTest, OrderBookSize_DoesNotMutate_NonEmptyBook) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);
  addTestOrder(Side::Ask, OrderType::GTC, 150, 5);

  const auto initialBestBid = orderBook.bestBid();
  const auto initialBestAsk = orderBook.bestAsk();
  const auto initialBidDepth = orderBook.getBookDepth(Side::Bid);
  const auto initialAskDepth = orderBook.getBookDepth(Side::Ask);
  const auto initialSize = orderBook.getOrderBookSize();
  const auto initialBidQty = orderBook.getQuantityAt(Side::Bid, 100);
  const auto initialAskQty = orderBook.getQuantityAt(Side::Ask, 150);

  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);

  EXPECT_EQ(orderBook.bestBid(), initialBestBid);
  EXPECT_EQ(orderBook.bestAsk(), initialBestAsk);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), initialBidDepth);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), initialAskDepth);
  EXPECT_EQ(orderBook.getOrderBookSize(), initialSize);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), initialBidQty);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 150), initialAskQty);
}
