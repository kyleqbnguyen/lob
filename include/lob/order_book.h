#pragma once

#include "lob/level_info.h"
#include "lob/types.h"

#include <map>
#include <optional>

using Book = std::map<Price, LevelInfo>;

class OrderBook {
public:
  Trades addOrder(Order order);

  bool cancelOrder(OrderId orderId);

  bool modifyOrder(OrderId orderId, Quantity quantity);

  std::optional<Price> bestBid();

  std::optional<Price> bestAsk();

private:
  Book bids_;
  Book asks_;
};
