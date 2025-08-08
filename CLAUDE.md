# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- `./build.sh [handheld|drone|base|all]` - Build specific environment or all
- `./upload.sh [handheld|drone|base]` - Upload to specific board


## Design Principles & Architecture

### Core Architectural Patterns

**Layered Architecture**: Maintain strict separation between:
- Hardware abstraction layer (HAL) - isolates hardware dependencies
- Core Framework layer - application lifecycle, logging, state management
- Business Logic layer - reusable components (monitoring, display, input, formatting)
- Application layer - application-specific implementations with minimal main.cpp
- Communication layer - inter-module and external messaging
- Presentation layer - UI/UX and user feedback

**Application Framework Pattern**: All applications extend AppFramework base class with:
- Lifecycle hooks: onInitialize(), onStart(), onUpdate(), onShutdown(), onError()
- Consistent state management via StateMachine<T> template
- Centralized logging via Logger class with severity levels
- System monitoring via SystemMonitor with health checks

**Message-Passing Architecture**: Use asynchronous, event-driven communication between modules. Prefer publish-subscribe patterns over direct coupling. This enables:
- Module independence and testability
- Dynamic system reconfiguration
- Fault isolation and recovery

**Reactive System Design**: All functionality should be:
- Divided into exchangeable, reusable components
- Communicating via asynchronous message passing
- Non-blocking and interrupt-driven where possible

### Memory & Resource Management

**Static Allocation First**: Prefer compile-time memory allocation over dynamic allocation. Use memory pools for predictable runtime behavior.

**Resource Budgeting**: Every module must define and respect:
- Maximum memory footprint (stack + heap)
- CPU time budget per cycle
- Peripheral access patterns

**Defensive Programming**: Always validate inputs, check return values, and handle edge cases. Assume hardware can fail.

### Real-Time Constraints

**Deterministic Execution**: Critical paths must have:
- Bounded execution time
- No unbounded loops or recursion
- Predictable interrupt latency

**Priority Inversion Prevention**: Use priority ceiling protocol or priority inheritance when sharing resources between tasks.

**Watchdog Strategy**: Implement multi-level watchdogs:
- Hardware watchdog for system crashes
- Software watchdogs for task monitoring
- Communication timeouts for external interfaces

## Development Workflow

### Feature Implementation Process

1. **Design First**: Before coding, document:
   - Module interfaces and contracts
   - State machines and sequence diagrams
   - Resource requirements and constraints

2. **Test-Driven Development**: 
   - Write unit tests before implementation
   - Use mocks/stubs for hardware dependencies
   - Aim for >80% code coverage on business logic

3. **Incremental Integration**:
   - Integrate one module at a time
   - Verify interfaces at each step
   - Use hardware-in-the-loop (HIL) testing when available

### Code Quality Standards

**MISRA-C Compliance**: Follow MISRA-C guidelines for safety-critical code:
- No dynamic memory after initialization
- Restricted use of pointers
- Explicit type conversions
- Single exit point for functions

**Defensive Boundaries**: Every module interface must:
- Validate all inputs
- Never trust external data
- Fail safely with meaningful error codes
- Log anomalies for debugging

**Const Correctness**: Use `const` extensively:
- Mark all read-only parameters as const
- Use const for lookup tables and configuration
- Prefer immutable data structures

### Testing Strategy

**Unit Testing** (Off-Target):
- Test business logic in isolation
- Mock hardware interfaces
- Run on every commit via CI/CD
- Target: microsecond execution, thousands of tests

**Integration Testing** (Target/Simulator):
- Test module interactions
- Verify timing constraints
- Use Software-in-the-Loop (SIL) first
- Progress to Hardware-in-the-Loop (HIL)

**System Testing** (On-Target):
- End-to-end scenarios
- Performance profiling
- Power consumption analysis
- Environmental stress testing

**Regression Testing**:
- Automated test suite running nightly
- Capture and replay real-world scenarios
- Performance benchmarks with thresholds
- Memory leak detection

## Optimization Guidelines

### Performance Optimization

**Profile First**: Never optimize without measurements. Use:
- Cycle counters for timing analysis
- Logic analyzer for protocol debugging
- Power profiler for energy optimization

**Optimization Hierarchy**:
1. Algorithm optimization (O(n) improvements)
2. Data structure optimization (cache locality)
3. Compiler optimization flags
4. Assembly optimization (last resort)

**Trade-off Documentation**: Every optimization must document:
- Performance gain measured
- Code complexity increase
- Maintenance implications
- Alternative approaches considered

### Memory Optimization

**ROM/Flash Optimization**:
- Const data in program memory
- Compress large lookup tables
- Remove dead code aggressively
- Link-time optimization (LTO)

**RAM Optimization**:
- Stack usage analysis per function
- Union overlapping data structures
- Bit-packing for flags
- Circular buffers for streaming data

## Error Handling Philosophy

**Fail-Safe Defaults**: System must enter safe state on any error.

**Error Propagation**: Errors bubble up through return codes, never exceptions or setjmp/longjmp.

**Recovery Strategies**:
- Retry with exponential backoff
- Degraded operation modes
- Graceful degradation
- Safe shutdown sequences

**Logging & Diagnostics**:
- Structured logging with severity levels
- Black box recorder for post-mortem
- Assertion checks in debug builds
- Runtime diagnostics via telemetry

## Module Design Principles

**Single Responsibility**: Each module does one thing well.

**Interface Segregation**: Clients shouldn't depend on interfaces they don't use.

**Dependency Inversion**: Depend on abstractions, not concretions.

**Open/Closed Principle**: Open for extension, closed for modification.

**Liskov Substitution**: Derived modules must be substitutable for their base modules.

## Communication Patterns

**Command-Query Separation**: Functions either change state (command) or return data (query), never both.

**Publisher-Subscriber**: For one-to-many communication without coupling.

**Request-Response**: For synchronous operations with acknowledgment.

**State Machines**: For complex behavioral logic with clear transitions.

## Continuous Improvement

**Code Reviews**: Every change requires review focusing on:
- Adherence to patterns and principles
- Resource usage analysis
- Test coverage and quality
- Documentation completeness

**Metrics Tracking**:
- Cyclomatic complexity per function (<10)
- Module coupling and cohesion
- Build time and binary size
- Static analysis warnings

**Technical Debt Management**:
- Document shortcuts with TODO/FIXME
- Regular refactoring sprints
- Deprecation strategy for old interfaces
- Migration paths for breaking changes