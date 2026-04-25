#include "harness.h"
#include "lob/map_order_book.h"
#include "lob/order_book_concept.h"
#include "lob/workload.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static_assert(OrderBookLike<MapOrderBook>);

int main(int argc, char** argv) {
  std::size_t n{argc > 1 ? std::stoull(argv[1]) : 1'000'000};
  std::uint64_t seed{argc > 2 ? std::stoull(argv[2]) : 42};

  std::vector<OrderEvent> events;
  generateSyntheticWorkload(events, n, seed);

  MapOrderBook book;
  runWorkload(book, events);

  std::cout << "events=" << events.size() << " orders=" << book.orderCount()
            << " bestBid=" << (book.bestBid() ? *book.bestBid() : -1)
            << " bestAsk=" << (book.bestAsk() ? *book.bestAsk() : -1) << "\n";

  return 0;
}
