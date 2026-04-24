#pragma once

#include "lob/map_order_book.h"
#include "lob/types.h"

#include <gtest/gtest.h>

class OrderBookTest : public ::testing::Test {
protected:
  MapOrderBook orderBook{};
  OrderId id{1};

  void addTestOrder(Side side, OrderType type, TimeInForce timeInForce,
                    Price price, Quantity quantity) {
    orderBook.addOrder({.id = id++,
                        .side = side,
                        .type = type,
                        .timeInForce = timeInForce,
                        .price = price,
                        .quantity = quantity});
  }
};
