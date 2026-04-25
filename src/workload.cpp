#include "lob/workload.h"

#include "lob/types.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

constexpr Price midPrice{10000};
constexpr Price priceHalfRange{50};
constexpr Quantity minQuantity{1};
constexpr Quantity maxQuantity{100};

constexpr int addWeight{80};
constexpr int cancelWeight{15};
constexpr int modifyWeight{5};

} // namespace

// TODO:
//   - crossing
//   - TIF: IOC, FOK
void generateSyntheticWorkload(std::vector<OrderEvent>& out, std::size_t n,
                               std::uint64_t seed) {
  out.clear();
  out.reserve(n);

  std::mt19937_64 rng{seed};

  std::bernoulli_distribution sideDist{0.5};
  std::uniform_int_distribution<Price> offsetDist{1, priceHalfRange};
  std::uniform_int_distribution<Quantity> quantityDist{minQuantity,
                                                       maxQuantity};

  std::vector<OrderId> liveIds;
  liveIds.reserve(n);

  OrderId nextId{1};

  auto emitAdd = [&]() {
    Side side{sideDist(rng) ? Side::Bid : Side::Ask};
    Price offset{offsetDist(rng)};
    Price price{(side == Side::Bid) ? midPrice - offset : midPrice + offset};

    Order order{.id = nextId,
                .side = side,
                .type = OrderType::Limit,
                .timeInForce = TimeInForce::GTC,
                .price = price,
                .quantity = quantityDist(rng)};

    out.push_back(OrderEvent{.type = EventType::Add,
                             .id = nextId,
                             .order = order,
                             .newQuantity = 0});

    liveIds.push_back(nextId);
    ++nextId;
  };

  auto pickLiveIndex = [&]() {
    std::uniform_int_distribution<std::size_t> idxDist{0, liveIds.size() - 1};
    return idxDist(rng);
  };

  std::discrete_distribution<int> eventDist{
      {addWeight, cancelWeight, modifyWeight}};

  for (std::size_t i{0}; i < n; ++i) {
    int choice{eventDist(rng)};

    if (choice != 0 && liveIds.empty()) {
      choice = 0;
    }

    switch (choice) {
    case 0:
      emitAdd();
      break;

    case 1: {
      std::size_t idx{pickLiveIndex()};
      OrderId id{liveIds[idx]};

      out.push_back(OrderEvent{
          .type = EventType::Cancel, .id = id, .order = {}, .newQuantity = 0});

      liveIds[idx] = liveIds.back();
      liveIds.pop_back();
      break;
    }

    case 2: {
      std::size_t idx{pickLiveIndex()};
      OrderId id{liveIds[idx]};

      out.push_back(OrderEvent{.type = EventType::Modify,
                               .id = id,
                               .order = {},
                               .newQuantity = quantityDist(rng)});
      break;
    }
    }
  }
}

// TODO: load tests from file
std::vector<OrderEvent> loadEvents(std::string_view /*path*/) {
  throw std::runtime_error{"loadEvents: not implemented"};
}
