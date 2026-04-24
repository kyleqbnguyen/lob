#pragma once

#include "lob/types.h"

#include <concepts>
#include <cstddef>
#include <optional>

template <typename T>
concept OrderBookLike = requires(T book, const T cbook, Order order, OrderId id,
                                 Quantity quantity, Side side, Price price) {
  { book.addOrder(order) } -> std::same_as<Trades>;
  { book.cancelOrder(id) } -> std::same_as<bool>;
  { book.modifyOrder(id, quantity) } -> std::same_as<bool>;

  { cbook.bestBid() } -> std::same_as<std::optional<Price>>;
  { cbook.bestAsk() } -> std::same_as<std::optional<Price>>;
  { cbook.depth(side) } -> std::same_as<std::size_t>;
  { cbook.quantityAt(side, price) } -> std::same_as<Quantity>;
  { cbook.orderCount() } -> std::same_as<std::size_t>;
};
