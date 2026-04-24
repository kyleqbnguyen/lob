#pragma once

#include "lob/types.h"

#include <cstddef>
#include <map>
#include <optional>
#include <unordered_map>

class MapOrderBook {
public:
  Trades addOrder(Order order);

  bool cancelOrder(OrderId orderId);

  bool modifyOrder(OrderId orderId, Quantity quantity);

  std::optional<Price> bestBid() const;

  std::optional<Price> bestAsk() const;

  std::size_t getBookDepth(Side side) const;

  Quantity getQuantityAt(Side side, Price price) const;

  std::size_t getOrderBookSize() const;

private:
  void matchOrder(Order& order, Trades& trades);

  void insertResting(const Order& order);

  bool canFillFully(const Order& order);

  bool canCross(const Order& order, Price bestResting);

  std::map<Price, LevelInfo> bids_;
  std::map<Price, LevelInfo> asks_;

  std::unordered_map<OrderId, Order> orders_;
};
