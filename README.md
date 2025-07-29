# fecs — Fast ECS for C++

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![WIP](https://img.shields.io/badge/status-work_in_progress-orange)

**fecs** is a minimal and efficient Entity Component System (ECS) library for C++, designed to be simple, fast, and easy to integrate into any project.

> 🚧 **Work in Progress**: This library is still in early development and not yet feature complete.

> ☄️ In this small documentation I tried to explain, among other things, 
> how and why some parts of the system work the way they do.

---

## ✨ Features

### ✅ Core Functionality
- [x] 🧱 **Entity creation** — lightweight `uint32_t`-based entities
- [x] 🧩 **Component management** — add/remove with optional constructor args
- [x] 🧹 **Component cleanup** — auto-removal on entity destruction
- [x] 🏗 **Entity builder** — simplifies adding multiple components

### 🔁 Iteration Queues
- [x] 🔍 **Simple queues** — `view`, `runner`, `direct_for_each` for lightweight iteration
- [x] ⚡ **Fast owning queues** — `group`, `group_slice` for cache-friendly iteration
- [x] 👀 **View support in groups** — combine owned + viewed components
- [ ] 🚫 **Excluder** — filter out specific component types from iteration

---

# 🚀 How to Add to Project

## 🔧 Step 1: Clone the Repository

Clone this GitHub repository to a desired folder:

```bash
git clone https://github.com/foured/fecs.git
```

## ⚙️ Step 2: Include in CMake

Add the repository to your CMake project:

```cmake
add_subdirectory("somefolder/fecs")
```

## 🔗 Step 3: Link the Library

Link `fecs` to your target:

```cmake
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE fecs)
```

---

# 🛠 Getting Started

To begin using **fecs**, you need to create the main object — the **registry**:

```cpp
#include <fecs/core/registry.h>

int main() {
    fecs::registry registry;
    return 0;
}
```

This `registry` will manage all your entities and components.

---

# 🧱 Entity Management

### ➕ Creating Entities

```cpp
fecs::entity_t e = registry.create_entity();
```

Entities in **fecs** are just `uint32_t`, making them lightweight and efficient.

> 💡 It's recommended to pass entities **by value**, since they are only 4 bytes, whereas pointers or references typically take 8 bytes.

### ❌ Destroying Entities

```cpp
registry.destroy_entity(e);
```

This will automatically remove all components attached to the entity.

---

# 🧩 Component Management

### 🧬 Defining a Component

```cpp
struct component_1 {
    int bullets;
    float lifetime;
};
```

### ➕ Adding a Component

```cpp
registry.add_component<component_1>(e);
```

If your component has a constructor, you can pass arguments to `add_component`:

```cpp
struct component_2 {
    component_2(const std::string& s)
        : val(std::atoi(s.c_str())) {}

    int val;
};

registry.add_component<component_2>(e, "123456789");
```

If you need to add a lot of components and you don't want to pass
entity every time

```cpp
fecs::entity_builder builder(registry);
builder.add_component<component_1>(1, 0.1f);
fecs::entity_t e = builder.get();
```
Or simpler

```cpp
fecs::entity_t e = fecs::entity_builder(registry).with<component_1>(1, 0.1f).get();
```

### ➕ Bulk Component Addition

```cpp
std::vector<fecs::entity_t> entities;
registry.add_component<component_1>(entities);
```

### ❌ Removing a Component

```cpp
registry.remove_component<component_1>(e);
registry.remove_component<component_2>(entities);
```

### 🔍 Checking for Component Presence

```cpp
if (registry.has_component<component_1>(e)) {
    // ...
}
```

---

# ⚙️ Component Processing

## 💡 Concept

**fecs** does not enforce a strict "systems" abstraction, as optimal processing patterns vary by project.  
Instead, it provides a flexible set of **queues** to iterate over and process components.

> ❓ **Queues** are helper classes that simplify iteration and logic application on entities and their components.

---

## 📚 Types of Queues

There are two main categories of queues:

### 1. **Single Component Iteration**
- `runner`
- `direct_for_each`

### 2. **Multi-Component Iteration**
- `group`
- `group_slice`
- `view`

---

## 🧠 Why Two Types?

Because **fecs** stores each component type in a separate **sparse set** — a cache-friendly data structure that enables fast iteration and lookup.

> ℹ️ Sparse sets are similar to maps, but store data contiguously in memory for better performance.

When iterating over one component type, it's straightforward and fast.  
But when working with multiple components, there are **two strategies**:

### ⚡ Strategy 1: Fast Grouped Access (Preferred)

Align all sparse sets so that component data for entity `X` is stored at the same index `Y` across sets.  
This allows fast access via shared index.

- Implemented via: `group`, `group_slice`
- Requires **owned components**

> ⚠️ Only one group can **own** a given component type.

### 🧩 Strategy 2: Dynamic Lookup (Fallback)

Look up each component individually before applying logic.

- Implemented via: `view`
- Slower, but works in all cases

---

## 🆚 Queue Differences

### 🧮 `runner` vs `direct_for_each`

| Feature            | `runner`                   | `direct_for_each`         |
|--------------------|----------------------------|----------------------------|
| Type               | Class                      | Registry method           |
| Caching            | ✅ Yes                     | ❌ No                     |
| Suitable for       | Repeated logic             | One-time initialization   |
| Under the hood     | Uses `direct_for_each`     | Simple loop               |

**Guideline**:

- Use `direct_for_each` for simple initialization logic.
- Use `runner` when executing the same logic repeatedly — it caches iteration data for better performance.

---

### 🔬 `group` vs `group_slice`

Both allow multi-component iteration with fast access to **owned** and **viewed** components, but differ in flexibility:

| Feature                | `group`                                     | `group_slice`                                         |
|------------------------|----------------------------------------------|--------------------------------------------------------|
| Iteration scope        | All owned components                        | Only a selected subset of owned components             |
| Filtering capability   | ❌ No filtering — uses all owned             | ✅ Select only specific owned components               |
| Viewed components      | ✅ Supported                                 | ✅ Supported                                           |
| Performance tuning     | ❌ Less flexible                             | ✅ More granular and optimized                         |
| Use case               | Full group logic                            | Targeted or partial logic within the same group setup  |

> 💡 `group_slice` can only be used after a `group` that owns **all** of the slice’s owned components has been created.

**Guideline**:

- Use `group` for full access to all components in the group.
- Use `group_slice` when you only need to operate on a few components from the group — it avoids unnecessary unpacking.

---

# ⚡ Usage Examples

> 💡 All `for_each` functions can optionally take `fecs::entity_t` as the first parameter.

### `direct_for_each`

```cpp
registry.direct_for_each<component_1>([](component_1& c) {
    // ...
});

registry.direct_for_each<component_1>([](fecs::entity_t e, component_1& c) {
    // ...
});
```

### `runner`

```cpp
auto r = registry.runner<component_1>();
r.for_each([](fecs::entity_t e, component_1& c) {
    // ...
});
```

### `group`

Only owning group:

```cpp
registry.create_group<component_1, component_2>();
auto g = registry.group<component_1, component_2>();
g->for_each([](component_1& c1, component_2& c2) {
    // ...
});
```

Owning + viewed components:

```cpp
registry.create_group<component_1, component_2>(fecs::view_part<component_3>{});
auto g = registry.group<component_1, component_2>(fecs::view_part<component_3>{});
g->for_each([](component_1& c1, component_2& c2, component_3& c3) {
    // ...
});
```

Shorter with descriptor:

```cpp
using group_desc = fecs::queue_args_descriptor<
    fecs::pack_part<component_1, component_2>,
    fecs::view_part<component_3>
>;

registry.create_group(group_desc{});
auto g = registry.group(group_desc{});
g->for_each([](component_1& c1, component_2& c2, component_3& c3) {
    // ...
});
```

### `group_slice`

`group_slice` is used when you want to iterate over only **some** of the owned components from a `group`:

```cpp
registry.create_group<component_1, component_2, component_3>(fecs::view_part<component_4>{});

auto gs = registry.group_slice<component_1, component_3>(fecs::view_part<component_5>{});
gs.for_each([](fecs::entity_t e, component_1& c1, component_3& c3, component_5& c5) {
    // ...
});
```

You can also construct it via `queue_args_descriptor`:

```cpp
using slice_desc = fecs::queue_args_descriptor<
    fecs::pack_part<component_1, component_3>,
    fecs::view_part<component_5>
>;

auto gs = registry.group_slice(slice_desc{});
gs.for_each([](fecs::entity_t e, component_1& c1, component_3& c3, component_5& c5) {
    // ...
});
```

---
