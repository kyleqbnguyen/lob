#include "fixture.h"

#include <optional>

TEST_F(OrderBookTest, cancelOrder_shouldReturnTrue_whenOrderExists) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  EXPECT_TRUE(orderBook.cancelOrder(100));
}

TEST_F(OrderBookTest, cancelOrder_shouldReturnFalse_whenOrderDoesNotExist) {
  EXPECT_FALSE(orderBook.cancelOrder(999));
}

TEST_F(OrderBookTest, cancelOrder_shouldReturnFalse_whenOrderAlreadyCancelled) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.cancelOrder(100);

  EXPECT_FALSE(orderBook.cancelOrder(100));
}

TEST_F(OrderBookTest, cancelOrder_shouldEmptyBook_whenOnlyBidIsCancelled) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Bid,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestBid(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Bid), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest, cancelOrder_shouldEmptyBook_whenOnlyAskIsCancelled) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 10});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestAsk(), std::nullopt);
  EXPECT_EQ(orderBook.depth(Side::Ask), 0);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 0);
  EXPECT_EQ(orderBook.orderCount(), 0);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldUpdateBestBid_whenBestBidLevelIsCancelled) {
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
                      .quantity = 5});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestBid(), 9900);
  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldUpdateBestAsk_whenBestAskLevelIsCancelled) {
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
                      .quantity = 5});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.depth(Side::Ask), 1);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldDecrementDepth_whenLastOrderAtLevelIsCancelled) {
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
                      .quantity = 5});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldNotDecrementDepth_whenOtherOrdersRemainAtLevel) {
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
                      .price = 10000,
                      .quantity = 5});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.depth(Side::Bid), 1);
  EXPECT_EQ(orderBook.quantityAt(Side::Bid, 10000), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldReduceLevelQuantity_whenOneOfMultipleOrdersCancelled) {
  orderBook.addOrder({.id = 100,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 6});
  orderBook.addOrder({.id = 101,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 4});

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10000), 4);
  EXPECT_EQ(orderBook.orderCount(), 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldPreserveFifo_whenLeadingOrderAtLevelIsCancelled) {
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
  orderBook.addOrder({.id = 102,
                      .side = Side::Ask,
                      .type = OrderType::Limit,
                      .timeInForce = TimeInForce::GTC,
                      .price = 10000,
                      .quantity = 3});

  orderBook.cancelOrder(100);

  const auto trades = orderBook.addOrder({.id = 200,
                                          .side = Side::Bid,
                                          .type = OrderType::Limit,
                                          .timeInForce = TimeInForce::GTC,
                                          .price = 10000,
                                          .quantity = 8});

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].passiveId, 101);
  EXPECT_EQ(trades[0].quantity, 7);
  EXPECT_EQ(trades[1].passiveId, 102);
  EXPECT_EQ(trades[1].quantity, 1);
}

TEST_F(OrderBookTest,
       cancelOrder_shouldNotAffectOppositeSide_whenBidIsCancelled) {
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

  orderBook.cancelOrder(100);

  EXPECT_EQ(orderBook.bestAsk(), 10100);
  EXPECT_EQ(orderBook.quantityAt(Side::Ask, 10100), 5);
  EXPECT_EQ(orderBook.orderCount(), 1);
}
