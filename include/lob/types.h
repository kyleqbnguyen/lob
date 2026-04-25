#pragma once

#include <cstdint>
#include <deque>
#include <vector>

using OrderId = std::uint64_t;
using Price = std::int32_t;
using Quantity = std::uint32_t;

enum class Side : std::uint8_t { Bid, Ask };

enum class OrderType : std::uint8_t { Limit, Market };

enum class TimeInForce : std::uint8_t {
  GTC, // Good-Till-Cancel
  IOC, // Immediate-Or-Cancel
  FOK, // Fill-Or-Kill
};

struct Order {
  OrderId id;
  Side side;
  OrderType type;
  TimeInForce timeInForce;
  Price price;
  Quantity quantity;
};

struct Trade {
  OrderId aggressorId;
  OrderId passiveId;
  Price price;
  Quantity quantity;
};

using Trades = std::vector<Trade>;

struct LevelInfo {
  Quantity quantity{0};
  std::deque<OrderId> orderIds;
};
