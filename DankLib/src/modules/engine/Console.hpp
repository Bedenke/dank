#pragma once

namespace dank {
namespace console {
void init();
void release();
void log(const char *format, ...);
void warn(const char *format, ...);
}; // namespace console

} // namespace dank
