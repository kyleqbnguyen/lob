#pragma once

#include <cstdint>
#include <deque>
#include <map>
#include <vector>

using OrderId = std::uint64_t;
using Price = std::int32_t;
using Quantity = std::uint32_t;

enum class Side : std::uint8_t { Bid, Ask };

enum class OrderType : std::uint8_t {
  GTC, // Good-Till-Cancel
  IOC, // Immediate-Or-Cancel
  FOK, // Fill-Or-Kill
  DTC  // Day-Till-Cancel
};

struct Order {
  OrderId id;
  Side side;
  OrderType type;
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
  Price price;
  Quantity quantity{0};
  std::deque<Order> orders;
};

using Book = std::map<Price, LevelInfo>;
