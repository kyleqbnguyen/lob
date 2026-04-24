#pragma once
#include "lob/types.h"

#include <string_view>
#include <vector>

enum class EventType : std::uint8_t { Add, Cancel, Modify };

struct OrderEvent {
  EventType type;
  OrderId id;
  Order order;
  Quantity newQuantity;
};

std::vector<OrderEvent> loadEvents(std::string_view path);
void generateSyntheticWorkload(std::vector<OrderEvent>& out, std::size_t n,
                               std::uint64_t seed);
