#include "solver.hpp"

#include <await/executors/static_thread_pool.hpp>

////////////////////////////////////////////////////////////////////////////////

Solver::Solver(std::size_t thread_count, std::size_t batch_size)
    : thread_count_(thread_count), batch_size_(batch_size) {
}

auto Solver::Solve(const std::string& filename) -> int {
  knapsack_ = ReadFrom(filename);
  if (knapsack_.TooHeavyItems()) {
    return 0;
  }
  if (knapsack_.AllItemsFit()) {
    return knapsack_.GetTotalPrice();
  }

  knapsack_.SortItems();

  tp_ = await::executors::MakeStaticThreadPool(thread_count_);
  tp_->Execute([this] { Branch(/*states=*/{}, /*root=*/true); });
  tp_->Join();

  return max_price_.Get();
}

////////////////////////////////////////////////////////////////////////////////

auto Solver::Branch(States states, bool root) -> void {
  if (root && states.empty()) {
    states.push({});
  }

  while (!states.empty()) {
    auto state = states.top();
    states.pop();

    root ? root = false : state.cursor += 1;
    SingleBranch(state, &states);

    if (states.size() >= thread_count_ * batch_size_) {
      break;
    }
  }

  BatchPost(std::move(states));
}

auto Solver::SingleBranch(State state, States* states) -> void {
  auto [price, weight] = knapsack_.items[state.cursor];
  if (state.current_weight + weight <= knapsack_.capacity) {
    max_price_.Update(state.current_price + price);
  }

  // branch without item under cursor
  if (state.ComputeBound(knapsack_) > max_price_.Get()) {
    states->push(state);
  }

  // branch including item under cursor
  state.current_price += price;
  state.current_weight += weight;
  if (state.ComputeBound(knapsack_) > max_price_.Get()) {
    states->push(state);
  }
}

auto Solver::BatchPost(States states) -> void {
  for (const auto& s : Split(std::move(states), batch_size_)) {
    tp_->Execute([this, s] { Branch(s); });
  }
}
