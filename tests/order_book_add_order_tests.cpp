#include "order_book_test.h"

#include <optional>

TEST_F(OrderBookTest, AddOrder_ReturnsNoTrades_ForRestingInsert) {
  const auto trades = orderBook.addOrder({.id = id++,
                                          .side = Side::Bid,
                                          .type = OrderType::GTC,
                                          .price = 100,
                                          .quantity = 10});

  EXPECT_TRUE(trades.empty());
}

TEST_F(OrderBookTest,
       AddingOppositeSideLiquidity_DoesNotAffectExistingSideState) {
  addTestOrder(Side::Bid, OrderType::GTC, 100, 10);

  EXPECT_EQ(orderBook.bestBid(), 100);
  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 1);

  addTestOrder(Side::Ask, OrderType::GTC, 150, 7);

  EXPECT_EQ(orderBook.bestBid(), 100);
  EXPECT_EQ(orderBook.bestAsk(), 150);
  EXPECT_EQ(orderBook.getBookDepth(Side::Bid), 1);
  EXPECT_EQ(orderBook.getBookDepth(Side::Ask), 1);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 100), 10);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 150), 7);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Bid, 150), 0);
  EXPECT_EQ(orderBook.getQuantityAt(Side::Ask, 100), 0);
  EXPECT_EQ(orderBook.getOrderBookSize(), 2);
}
