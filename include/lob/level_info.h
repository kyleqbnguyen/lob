#pragma once

#include "lob/types.h"

#include <deque>

struct LevelInfo {
  Price price;
  Quantity quantity{0};
  std::deque<Order> orders;
};
