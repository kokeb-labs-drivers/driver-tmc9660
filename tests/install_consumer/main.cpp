/**
 * @file main.cpp
 * @brief Builds the driver from the installed package only: headers, version header and .ipp.
 */
#include <cstddef>
#include <cstdint>

#include "tmc9660.hpp"

namespace {

// A port that answers every read with 0: enough to instantiate the whole driver template.
class NullPort : public tmc9660::SpiInterface<NullPort> {
public:
  bool EnsureInitialized() noexcept {
    return true;
  }
  bool Transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) noexcept {
    static_cast<void>(tx);
    for (std::size_t i = 0; (rx != nullptr) && (i < len); ++i) {
      rx[i] = 0;
    }
    return true;
  }
  void DelayUs(uint32_t us) noexcept {
    static_cast<void>(us);
  }
};

} // namespace

int main() {
  NullPort port;
  tmc9660::TMC9660<NullPort> chip(port);
  // The chip answers 0 to everything, so EnsureInitialized() must fail: proof that the template
  // compiled and ran.
  const auto r = chip.EnsureInitialized();
  return ((r.error != tmc9660::Error::Ok) &&
          (static_cast<unsigned char>(tmc9660::GetDriverVersion()[0]) != 0U))
             ? 0
             : 1;
}
