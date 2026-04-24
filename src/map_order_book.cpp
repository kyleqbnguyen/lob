#include "lob/map_order_book.h"

#include "lob/types.h"

#include <algorithm>
#include <iterator>
#include <optional>

Trades MapOrderBook::addOrder(Order order) {
  Trades trades{};

  switch (order.timeInForce) {
  case TimeInForce::GTC:
    matchOrder(order, trades);
    if (order.quantity > 0 && order.type == OrderType::Limit) {
      insertResting(order);
    }
    break;

  case TimeInForce::IOC:
    matchOrder(order, trades);
    break;

  case TimeInForce::FOK:
    if (canFillFully(order)) {
      matchOrder(order, trades);
    }
    break;

  // TODO
  case TimeInForce::DTC:
    matchOrder(order, trades);
    if (order.quantity > 0 && order.type == OrderType::Limit) {
      insertResting(order);
    }
    break;
  }

  return trades;
}

bool MapOrderBook::cancelOrder(OrderId orderId) {
  auto orderIt{orders_.find(orderId)};
  if (orderIt == orders_.end()) {
    return false;
  }

  const auto& orderInfo{orderIt->second};
  auto& book{(orderInfo.side == Side::Bid) ? bids_ : asks_};

  auto levelIt{book.find(orderInfo.price)};
  if (levelIt == book.end()) {
    return false;
  }

  auto& levelInfo{levelIt->second};
  auto& levelQueue{levelInfo.orderIds};

  auto dqIt{std::find(levelQueue.begin(), levelQueue.end(), orderId)};
  if (dqIt == levelQueue.end()) {
    return false;
  }

  levelQueue.erase(dqIt);
  levelInfo.quantity -= orderInfo.quantity;

  if (levelQueue.empty()) {
    book.erase(levelIt);
  }

  orders_.erase(orderIt);
  return true;
}

bool MapOrderBook::modifyOrder(OrderId orderId, Quantity quantity) {
  if (quantity == 0) {
    return false;
  }

  auto orderIt{orders_.find(orderId)};
  if (orderIt == orders_.end()) {
    return false;
  }

  auto& orderInfo{orderIt->second};
  if (orderInfo.quantity <= quantity) {
    return false;
  }

  auto& book{(orderInfo.side == Side::Bid) ? bids_ : asks_};
  auto levelIt{book.find(orderInfo.price)};
  if (levelIt == book.end()) {
    return false;
  }

  Quantity removed{orderInfo.quantity - quantity};

  auto& levelInfo{levelIt->second};
  orderInfo.quantity -= removed;
  levelInfo.quantity -= removed;

  return true;
}

std::optional<Price> MapOrderBook::bestBid() const {
  if (bids_.empty()) {
    return std::nullopt;
  }

  return bids_.rbegin()->first;
}

std::optional<Price> MapOrderBook::bestAsk() const {
  if (asks_.empty()) {
    return std::nullopt;
  }

  return asks_.begin()->first;
}

std::size_t MapOrderBook::depth(Side side) const {
  auto& book{(side == Side::Bid) ? bids_ : asks_};

  return book.size();
}

Quantity MapOrderBook::quantityAt(Side side, Price price) const {
  auto& book{(side == Side::Bid) ? bids_ : asks_};
  auto it = book.find(price);

  if (it == book.end()) {
    return 0;
  }

  return it->second.quantity;
}

std::size_t MapOrderBook::orderCount() const { return orders_.size(); }

void MapOrderBook::matchOrder(Order& order, Trades& trades) {
  auto& oppBook{(order.side == Side::Bid) ? asks_ : bids_};

  while (order.quantity != 0 && !oppBook.empty()) {
    auto bestIt{(order.side == Side::Bid) ? oppBook.begin()
                                          : std::prev(oppBook.end())};

    if (!canCross(order, bestIt->first)) {
      break;
    }

    auto& level{bestIt->second};
    auto& queue{level.orderIds};

    while (order.quantity > 0 && !queue.empty()) {
      auto restingIt = orders_.find(queue.front());

      Quantity filled{std::min(order.quantity, restingIt->second.quantity)};

      trades.push_back(
          {order.id, queue.front(), restingIt->second.price, filled});

      order.quantity -= filled;
      restingIt->second.quantity -= filled;
      level.quantity -= filled;

      if (restingIt->second.quantity == 0) {
        queue.pop_front();
        orders_.erase(restingIt);
      }
    }

    if (level.quantity == 0) {
      oppBook.erase(bestIt);
    }
  }
}

void MapOrderBook::insertResting(const Order& order) {
  auto& book{(order.side == Side::Bid) ? bids_ : asks_};
  auto& level{book[order.price]};

  level.quantity += order.quantity;

  level.orderIds.push_back(order.id);

  orders_[order.id] = order;
}

bool MapOrderBook::canFillFully(const Order& order) {
  auto& oppBook{(order.side == Side::Bid) ? asks_ : bids_};

  if (oppBook.empty()) {
    return false;
  }

  Quantity remaining{order.quantity};

  if (order.side == Side::Bid) {
    for (auto it{oppBook.begin()}; it != oppBook.end() && remaining != 0;
         ++it) {
      if (!canCross(order, it->first)) {
        break;
      }
      remaining -= std::min(remaining, it->second.quantity);
    }
  } else {
    for (auto it{oppBook.rbegin()}; it != oppBook.rend() && remaining != 0;
         ++it) {
      if (!canCross(order, it->first)) {
        break;
      }
      remaining -= std::min(remaining, it->second.quantity);
    }
  }

  return remaining == 0;
}

bool MapOrderBook::canCross(const Order& order, Price bestResting) {
  if (order.type == OrderType::Market) {
    return true;
  }

  if (order.side == Side::Bid) {
    return order.price >= bestResting;
  }

  return order.price <= bestResting;
}
