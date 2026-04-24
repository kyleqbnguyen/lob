#pragma once

#include "lob/order_book_concept.h"
#include "lob/workload.h"

#include <vector>

template <OrderBookLike Book>
void runWorkload(Book& book, const std::vector<OrderEvent>& events) {
  for (const auto& event : events) {
    switch (event.type) {
    case EventType::Add:
      book.addOrder(event.order);
      break;
    case EventType::Cancel:
      book.cancelOrder(event.id);
      break;
    case EventType::Modify:
      book.modifyOrder(event.id, event.newQuantity);
      break;
    }
  }
}
