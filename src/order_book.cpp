#include "lob/order_book.h"

#include "lob/types.h"

#include <optional>

Trades OrderBook::addOrder(Order order) {
  // TODO: handle Time-in-force
  Trades trades{};

  matchOrder(order, trades);

  return trades;
}

// bool OrderBook::cancelOrder(OrderId orderId) {}
// bool OrderBook::modifyOrder(OrderId orderId, Quantity quantity) {}
std::optional<Price> OrderBook::bestBid() {
  if (bids_.empty()) { return std::nullopt; }

  return bids_.rbegin()->first;
}

std::optional<Price> OrderBook::bestAsk() {
  if (asks_.empty()) { return std::nullopt; }

  return asks_.begin()->first;
}

std::size_t OrderBook::getBookDepth(Side side) {
  auto& book{(side == Side::Bid) ? bids_ : asks_};

  return book.size();
}

Quantity OrderBook::getQuantityAt(Side side, Price price) {
  auto& book{(side == Side::Bid) ? bids_ : asks_};
  auto it = book.find(price);

  if (it == book.end()) { return 0; }

  return it->second.quantity;
}

std::size_t OrderBook::getOrderBookSize() {
  return orders_.size();
}

void OrderBook::matchOrder(Order& order, Trades& trades) {
  auto& book{(order.side == Side::Bid) ? bids_ : asks_};
  auto& level{book[order.price]};

  level.quantity += order.quantity;

  level.orderIds.push_back(order.id);

  orders_[order.id] = order;
}
// bool OrderBook::canFillFully(const Order& order) {}
