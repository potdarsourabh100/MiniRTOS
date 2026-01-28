
---

## ✅ CHANGELOG.md (First Release)

```md
# Changelog

All notable changes to this project will be documented in this file.

---

## [v1.0.0] – Initial Release - 26/1/2026

### 🎉 Added
- Time-based round-robin task scheduler
- Support for:
  - Periodic tasks
  - One-shot tasks
  - Immediate execution tasks
- Circular linked-list task management
- Task control APIs:
  - Add task
  - Remove task
  - Pause task
  - Resume task
  - Modify task
  - Get task status
- Lightweight interrupt-safe queue implementation
- CMSIS-based critical section handling using PRIMASK
- Configurable limits:
  - Maximum number of tasks
  - Maximum task interval
  - Queue size and elements
- MIT license

### 🧠 Design Decisions
- Cooperative scheduling for simplicity
- No dynamic memory allocation
- Minimal RAM and CPU overhead
- Readability prioritized over complexity

### ⚠ Known Limitations
- No task priorities
- No preemption or context switching
- No software timers or synchronization primitives
- Requires external system tick increment

---

*This is the first public release of MiniRTOS and serves as a stable foundation for future enhancements.*
