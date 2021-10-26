#pragma once

#include <cstdint>

class Bus;
struct CartCoreState;
struct CartData;

void UpdatePpuRamMap(Bus& bus, CartCoreState& state);

void UpdateChrMap8k(CartCoreState& state, CartData& data, uint32_t bank);